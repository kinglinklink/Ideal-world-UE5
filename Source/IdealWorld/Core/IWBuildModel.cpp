#include "IWBuildModel.h"

#include "IWAero.h"
#include "IWMaterials.h"

#include <algorithm>
#include <set>

namespace IW
{
	namespace Records
	{
		PartRecord Make(const std::string& Type, const IVec3& Pos, const Basis& Rot, const IVec3& Size,
			const std::string& Material, const IW::Color& C)
		{
			PartRecord R;
			R.Type = Type;
			R.Pos = Pos;
			R.Rot = Rot;
			R.Size = Size;
			R.Material = Material;
			R.PartColor = C;
			R.Props = Settings::Defaults(Type);
			return R;
		}

		IVec3 DefaultSizeUnits(const std::string& Type)
		{
			return IVec3::FromRounded(Parts::Get(Type).DefaultSizeCells * (double)Units::UnitsPerCell);
		}

		Vec3 HalfExtentUnits(const PartRecord& R)
		{
			const Vec3 Size = R.Size.ToVec();
			const Vec3 AlongX = (R.Rot.Col[0] * Size.X).Abs();
			const Vec3 AlongY = (R.Rot.Col[1] * Size.Y).Abs();
			const Vec3 AlongZ = (R.Rot.Col[2] * Size.Z).Abs();
			return (AlongX + AlongY + AlongZ) * 0.5;
		}

		Box BoundsOf(const PartRecord& R)
		{
			const Vec3 Half = HalfExtentUnits(R) * Units::Unit;
			const Vec3 Center = R.Pos.ToVec() * Units::Unit;
			return Box(Center - Half, Half * 2.0);
		}

		double VolumeOf(const PartRecord& R)
		{
			const Vec3 SizeM = Units::ToMetres(R.Size);
			return SizeM.X * SizeM.Y * SizeM.Z * Parts::Get(R.Type).VolumeFraction;
		}

		bool IsHiddenControl(const PartRecord& R)
		{
			return Parts::IsControlPart(R.Type) && !Settings::Flag(R.Props, "visible_after_spawn", false);
		}

		double MassOf(const PartRecord& R)
		{
			if (IsHiddenControl(R)) { return 0.0; }
			return VolumeOf(R) * Materials::Info(R.Material).Density;
		}

		double HealthOf(const PartRecord& R)
		{
			const MaterialInfo& M = Materials::Info(R.Material);
			if (M.Immovable) { return 1e30; }
			return MassOf(R) * M.Strength;
		}

		Vec3 CentroidOf(const PartRecord& R)
		{
			const Vec3 SizeM = Units::ToMetres(R.Size);
			const Vec3 Offset = Parts::Get(R.Type).UnitCentroid.Mul(SizeM);
			return R.Pos.ToVec() * Units::Unit + R.Rot * Offset;
		}

		IVec3 CellCenterUnits(const IVec3& Cell)
		{
			return Cell * Units::UnitsPerCell + IVec3(Units::HalfCellUnits, Units::HalfCellUnits, Units::HalfCellUnits);
		}

		IVec3 PointToCell(const Vec3& LocalPoint)
		{
			return IVec3::FromFloored(LocalPoint / Units::Cell);
		}
	}

	namespace BuildModelDetail
	{
		bool Overlaps(const Box& A, const Box& B)
		{
			return A.Grow(-GridSize::OverlapEpsilon).Intersects(B);
		}
	}

	std::vector<int> BuildModel::AllUids() const
	{
		std::vector<int> Uids;
		Uids.reserve(Parts.size());
		for (const auto& Pair : Parts) { Uids.push_back(Pair.first); }
		return Uids;
	}

	const PartRecord* BuildModel::Find(int Uid) const
	{
		const auto Found = Parts.find(Uid);
		return Found == Parts.end() ? nullptr : &Found->second;
	}

	PartRecord BuildModel::Get(int Uid) const
	{
		const PartRecord* R = Find(Uid);
		return R ? *R : PartRecord();
	}

	std::string BuildModel::CanPlace(const PartRecord& R, const std::vector<int>& Ignore) const
	{
		const Box Bounds = Records::BoundsOf(R);
		const Vec3 Pad = GridSize::SizeMetres();
		const Vec3 Low = Bounds.Position;
		const Vec3 High = Bounds.End();
		const double E = GridSize::OverlapEpsilon;
		if (Low.X < -E || Low.Y < -E || Low.Z < -E || High.X > Pad.X + E || High.Y > Pad.Y + E || High.Z > Pad.Z + E)
		{
			return "That is outside the build grid.";
		}
		for (const auto& Pair : Parts)
		{
			if (std::find(Ignore.begin(), Ignore.end(), Pair.first) != Ignore.end()) { continue; }
			const PartRecord& Other = Pair.second;
			if (!BuildModelDetail::Overlaps(Bounds, Records::BoundsOf(Other))) { continue; }
			if (Other.Material != R.Material) { return "Materials must be homogeneous to overlap."; }
			if (Other.Type == R.Type && Other.Pos == R.Pos && Other.Size == R.Size && Other.Rot.IsNearlyEqual(R.Rot))
			{
				return "An identical part is already there.";
			}
		}
		return "";
	}

	std::vector<int> BuildModel::OverlapGroup(int Uid) const
	{
		std::vector<int> Group = {Uid};
		std::vector<int> Queue = {Uid};
		std::set<int> InGroup = {Uid};
		while (!Queue.empty())
		{
			const int Current = Queue.back();
			Queue.pop_back();
			const Box Bounds = Records::BoundsOf(Parts.at(Current));
			for (const auto& Pair : Parts)
			{
				if (InGroup.count(Pair.first)) { continue; }
				if (BuildModelDetail::Overlaps(Bounds, Records::BoundsOf(Pair.second)))
				{
					InGroup.insert(Pair.first);
					Group.push_back(Pair.first);
					Queue.push_back(Pair.first);
				}
			}
		}
		return Group;
	}

	std::vector<std::tuple<int, std::string, std::string>> BuildModel::ConsumersOf(int Uid) const
	{
		std::vector<std::tuple<int, std::string, std::string>> Found;
		for (const auto& Pair : Parts)
		{
			for (const auto& LinkPair : Pair.second.Links)
			{
				if (LinkPair.second.Uid == Uid) { Found.emplace_back(Pair.first, LinkPair.first, LinkPair.second.Output); }
			}
		}
		return Found;
	}

	int BuildModel::NearestOfType(const std::string& Type, const Vec3& Point, double Reach) const
	{
		int Best = 0;
		double BestDistance = Reach;
		for (const auto& Pair : Parts)
		{
			if (Pair.second.Type != Type) { continue; }
			const double Distance = (Records::BoundsOf(Pair.second).Center() - Point).Length();
			if (Distance < BestDistance)
			{
				Best = Pair.first;
				BestDistance = Distance;
			}
		}
		return Best;
	}

	double BuildModel::TotalMass() const
	{
		double Total = 0.0;
		for (const auto& Pair : Parts) { Total += Records::MassOf(Pair.second); }
		return Total;
	}

	Vec3 BuildModel::CenterOfMass() const
	{
		double Total = 0.0;
		Vec3 Weighted;
		for (const auto& Pair : Parts)
		{
			const double Mass = Records::MassOf(Pair.second);
			Total += Mass;
			Weighted += Records::CentroidOf(Pair.second) * Mass;
		}
		return Total > 0.0 ? Weighted / Total : Vec3();
	}

	bool BuildModel::CenterOfLift(Vec3& Out) const
	{
		Vec3 Forward(0, 0, -1);
		Vec3 Up(0, 1, 0);
		for (const auto& Pair : Parts)
		{
			if (Pair.second.Type == "seat")
			{
				Forward = Pair.second.Rot * Vec3(0, 0, -1);
				Up = Pair.second.Rot * Vec3(0, 1, 0);
				break;
			}
		}
		// Flying forward and sinking a little: the air meets the underside at 5 degrees.
		const double Attack = DegToRad(5.0);
		const Vec3 Moving = (Forward * std::cos(Attack) - Up * std::sin(Attack)) * 10.0;
		double Total = 0.0;
		Vec3 Weighted;
		for (const auto& Pair : Parts)
		{
			const PartRecord& R = Pair.second;
			if (!Parts::IsAeroPanel(R.Type)) { continue; }
			const Vec3 SizeM = Units::ToMetres(R.Size);
			double Area = SizeM.X * SizeM.Z;
			if (Parts::IsTrianglePanel(R.Type)) { Area *= 0.5; }
			const double Lift = Aero::PanelForce(Moving, R.Rot.Col[1], Area, true, Aero::SeaLevelAir).Dot(Up);
			Total += Lift;
			Weighted += Records::CentroidOf(R) * Lift;
		}
		if (std::fabs(Total) < 0.000001) { return false; }
		Out = Weighted / Total;
		return true;
	}

	void BuildModel::Changed()
	{
		++Counter;
		if (Batching)
		{
			BatchDirty = true;
		}
		else if (OnChanged)
		{
			OnChanged();
		}
	}

	void BuildModel::BeginBatch()
	{
		BatchBefore = TakeSnapshot();
		Batching = true;
		BatchDirty = false;
	}

	void BuildModel::EndBatch()
	{
		Batching = false;
		if (BatchDirty)
		{
			BatchDirty = false;
			PushUndo(BatchBefore);
			if (OnChanged) { OnChanged(); }
		}
		BatchBefore.clear();
	}

	void BuildModel::PushUndo(const Snapshot& State)
	{
		UndoStack.push_back(State);
		while ((int)UndoStack.size() > GridSize::HistoryLimit) { UndoStack.pop_front(); }
		RedoStack.clear();
	}

	void BuildModel::Remember()
	{
		if (!Batching) { PushUndo(TakeSnapshot()); }
	}

	bool BuildModel::Undo()
	{
		if (UndoStack.empty()) { return false; }
		RedoStack.push_back(TakeSnapshot());
		Snapshot State = UndoStack.back();
		UndoStack.pop_back();
		Restore(State);
		return true;
	}

	bool BuildModel::Redo()
	{
		if (RedoStack.empty()) { return false; }
		UndoStack.push_back(TakeSnapshot());
		Snapshot State = RedoStack.back();
		RedoStack.pop_back();
		Restore(State);
		return true;
	}

	void BuildModel::Restore(const Snapshot& State)
	{
		Parts = State;
		for (const auto& Pair : Parts) { NextUid = std::max(NextUid, Pair.first + 1); }
		++Counter;
		if (OnChanged) { OnChanged(); }
	}

	void BuildModel::SetLinks(int Uid, const IW::Links& NewLinks)
	{
		auto Found = Parts.find(Uid);
		if (Found == Parts.end()) { return; }
		Remember();
		Found->second.Links = NewLinks;
		Changed();
	}

	void BuildModel::PruneLinks()
	{
		for (auto& Pair : Parts)
		{
			IW::Links& L = Pair.second.Links;
			for (auto It = L.begin(); It != L.end();)
			{
				if (!Parts.count(It->second.Uid)) { It = L.erase(It); }
				else { ++It; }
			}
		}
	}

	void BuildModel::Clear()
	{
		Remember();
		Parts.clear();
		Changed();
	}

	int BuildModel::AddPart(const PartRecord& R)
	{
		if (!CanPlace(R).empty()) { return 0; }
		Remember();
		const int Uid = NextUid++;
		PartRecord Stored = R;
		Stored.CopiedFrom = 0;
		Parts[Uid] = Stored;
		Changed();
		return Uid;
	}

	bool BuildModel::RemovePart(int Uid)
	{
		if (!Parts.count(Uid)) { return false; }
		Remember();
		Parts.erase(Uid);
		PruneLinks();
		Changed();
		return true;
	}

	void BuildModel::RemoveParts(const std::vector<int>& Uids)
	{
		Remember();
		for (int Uid : Uids) { Parts.erase(Uid); }
		PruneLinks();
		Changed();
	}

	std::string BuildModel::UpdateParts(const std::map<int, PartRecord>& Updates)
	{
		std::vector<int> Ignore;
		for (const auto& Pair : Updates) { Ignore.push_back(Pair.first); }
		for (const auto& Pair : Updates)
		{
			if (!Parts.count(Pair.first)) { return "That part no longer exists."; }
			const std::string Problem = CanPlace(Pair.second, Ignore);
			if (!Problem.empty()) { return Problem; }
		}
		Remember();
		for (const auto& Pair : Updates) { Parts[Pair.first] = Pair.second; }
		Changed();
		return "";
	}

	void BuildModel::Paint(int Uid, const std::string& Material, const IW::Color& C)
	{
		if (!Parts.count(Uid)) { return; }
		Remember();
		for (int Member : OverlapGroup(Uid))
		{
			PartRecord& R = Parts[Member];
			R.Material = Material;
			R.PartColor = C;
		}
		Changed();
	}
}
