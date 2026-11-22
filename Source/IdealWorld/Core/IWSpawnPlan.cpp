#include "IWSpawnPlan.h"

#include "IWGeometry.h"
#include "IWMaterials.h"

#include <algorithm>

namespace IW
{
	namespace SpawnDetail
	{
		MachinePieceSpec P(const char* Name, Vec3 Middle, Vec3 Size, Vec3 Trim = Vec3(1, 1, 1))
		{
			MachinePieceSpec S;
			S.Name = Name; S.Middle = Middle; S.Size = Size; S.Trim = Trim;
			return S;
		}

		MachineJointSpec J(int A, int B, const char* Kind, int Axis, Vec3 Pivot)
		{
			MachineJointSpec S;
			S.A = A; S.B = B; S.Kind = Kind; S.Axis = Axis; S.Pivot = Pivot;
			return S;
		}

		std::map<std::string, MachineSpec> BuildMachines()
		{
			std::map<std::string, MachineSpec> M;
			const Vec3 O(0, 0, 0);
			const Vec3 One(1, 1, 1);
			// Two leaves on a pin.
			M["hinge"] = {"hinge", {P("base", Vec3(0, -0.25, 0), Vec3(1, 0.5, 1)), P("top", Vec3(0, 0.25, 0), Vec3(1, 0.5, 1))},
				{J(0, 1, "hinge", 0, O)}};
			// A housing, with the output on the top face turning about the shaft.
			const MachineSpec Housing = {"motor",
				{P("base", Vec3(0, -0.125, 0), Vec3(1, 0.75, 1)), P("top", Vec3(0, 0.375, 0), Vec3(1, 0.25, 1), Vec3(0.7, 1, 0.7))},
				{J(0, 1, "hinge", 1, O)}};
			M["motor"] = Housing;
			MachineSpec Servo = Housing;
			Servo.Role = "servo";
			M["servo"] = Servo;
			// A plate on the bottom and a leaf standing at the back (-Z), hinged along the edge they share.
			M["door_hinge_servo"] = {"servo",
				{P("base", Vec3(0, -0.4375, 0), Vec3(1, 0.125, 1)), P("top", Vec3(0, 0.0625, -0.4375), Vec3(1, 0.875, 0.125))},
				{J(0, 1, "hinge", 0, Vec3(0, -0.3125, -0.3125))}};
			M["ball_joint"] = {"ball", {P("base", Vec3(0, -0.25, 0), Vec3(1, 0.5, 1)), P("top", Vec3(0, 0.25, 0), Vec3(1, 0.5, 1))},
				{J(0, 1, "ball", 1, O)}};
			// A housing that welds by its four sides, and a shaft through it that
			// welds by its two ends and turns freely.
			M["bearing"] = {"bearing", {P("base", O, Vec3(1, 0.75, 1)), P("top", O, Vec3(0.3, 1, 0.3))}, {J(0, 1, "hinge", 1, O)}};
			const std::vector<MachinePieceSpec> Barrel = {P("base", Vec3(0, -0.125, 0), Vec3(1, 0.75, 1)), P("top", Vec3(0, 0.375, 0), Vec3(1, 0.25, 1))};
			M["piston"] = {"piston", Barrel, {J(0, 1, "slide", 1, O)}};
			M["fluid_cylinder"] = {"cylinder", Barrel, {J(0, 1, "slide", 1, O)}};
			M["combustion_cylinder"] = {"cylinder", Barrel, {J(0, 1, "slide", 1, O)}};
			M["solenoid"] = {"solenoid", Barrel, {J(0, 1, "slide", 1, O)}};
			M["spring"] = {"spring", {P("base", Vec3(0, -0.25, 0), Vec3(1, 0.5, 1)), P("top", Vec3(0, 0.25, 0), Vec3(1, 0.5, 1))},
				{J(0, 1, "slide", 1, O)}};
			// Journals that weld by the two ends (along X), and the webs and crank pin between them.
			M["crankshaft"] = {"crank", {P("base", O, Vec3(1, 0.3, 0.3)), P("top", O, Vec3(0.5, 1, 1), Vec3(1, 0.7, 0.7))},
				{J(0, 1, "hinge", 0, O)}};
			// A hub that welds by the two ends of the axle (along X), and the tyre, which turns on it.
			M["wheel"] = {"wheel", {P("base", O, Vec3(1, 0.3, 0.3)), P("top", O, Vec3(0.8, 1, 1))}, {J(0, 1, "hinge", 0, O)}};
			// A bar with a joint at each end. Each end welds to what touches it.
			M["linkage_bar"] = {"link",
				{P("bar", O, Vec3(0.84, 1, 1)), P("end_a", Vec3(-0.46, 0, 0), Vec3(0.08, 1, 1)), P("end_b", Vec3(0.46, 0, 0), Vec3(0.08, 1, 1))},
				{J(0, 1, "end", 1, Vec3(-0.46, 0, 0)), J(0, 2, "end", 1, Vec3(0.46, 0, 0))}};
			// Two plates that turn against each other, or grip.
			M["clutch"] = {"clutch",
				{P("base", Vec3(0, -0.25, 0), Vec3(1, 0.5, 1), Vec3(0.7, 1, 0.7)), P("top", Vec3(0, 0.25, 0), Vec3(1, 0.5, 1), Vec3(0.7, 1, 0.7))},
				{J(0, 1, "hinge", 1, O)}};
			// A caliper that welds by the +X face, and a disc on a hub that welds by its two ends (along Y).
			M["brake"] = {"brake", {P("base", Vec3(0.3, 0, 0), Vec3(0.4, 1, 0.45)), P("top", O, Vec3(0.6, 1, 0.6))}, {J(0, 1, "hinge", 1, O)}};
			(void)One;
			return M;
		}

		const std::map<std::string, MachineSpec>& Table()
		{
			static const std::map<std::string, MachineSpec> M = BuildMachines();
			return M;
		}

		const MachinePieceSpec& PieceSpec(const PartRecord& R, int Index)
		{
			return Machines::Of(R)->Pieces[(size_t)Index];
		}

		Vec3 PieceMiddle(const PartRecord& R, int Index)
		{
			const Vec3 SizeM = Units::ToMetres(R.Size);
			return R.Pos.ToVec() * Units::Unit + R.Rot * PieceSpec(R, Index).Middle.Mul(SizeM);
		}

		Vec3 PieceSizeOf(const PartRecord& R, int Index)
		{
			return PieceSpec(R, Index).Size.Mul(Units::ToMetres(R.Size));
		}

		Box PieceBox(const PartRecord& R, int Index)
		{
			if (!Machines::Of(R)) { return Records::BoundsOf(R); }
			const Vec3 Half = PieceSizeOf(R, Index) * 0.5;
			const Vec3 Reach = (R.Rot.Col[0] * Half.X).Abs() + (R.Rot.Col[1] * Half.Y).Abs() + (R.Rot.Col[2] * Half.Z).Abs();
			const Vec3 Centre = PieceMiddle(R, Index);
			return Box(Centre - Reach, Reach * 2.0);
		}

		double PieceMass(const PartRecord& R, int Index)
		{
			const MachineSpec* Machine = Machines::Of(R);
			const double Whole = Records::MassOf(R) * R.SolidShare;
			if (!Machine) { return Whole; }
			double Total = 0.0, Mine = 0.0;
			for (size_t At = 0; At < Machine->Pieces.size(); ++At)
			{
				const Vec3& S = Machine->Pieces[At].Size;
				const double Volume = S.X * S.Y * S.Z;
				Total += Volume;
				if ((int)At == Index) { Mine = Volume; }
			}
			return Total > 0.0 ? Whole * Mine / Total : 0.0;
		}

		Vec3 PieceCentre(const PartRecord& R, int Index)
		{
			if (!Machines::Of(R)) { return Records::CentroidOf(R); }
			return PieceMiddle(R, Index);
		}

		IVec3 BucketOf(const Vec3& P)
		{
			return IVec3::FromFloored(P / Welding::Bucket);
		}

		int RootOf(std::map<int, int>& Parent, int Piece)
		{
			int Root = Piece;
			while (Parent[Root] != Root) { Root = Parent[Root]; }
			int Current = Piece;
			while (Current != Root)
			{
				const int Next = Parent[Current];
				Parent[Current] = Root;
				Current = Next;
			}
			return Root;
		}

		void Join(std::map<int, int>& Parent, int A, int B)
		{
			const int RootA = RootOf(Parent, A);
			const int RootB = RootOf(Parent, B);
			if (RootA != RootB) { Parent[RootA] = RootB; }
		}

		void AddToCube(std::map<IVec3, std::pair<double, Vec3>>& Cubes, const Vec3& Point, double Volume, double CubeSize)
		{
			const IVec3 Key = IVec3::FromFloored(Point / CubeSize);
			auto& Cube = Cubes[Key];
			Cube.first += Volume;
			Cube.second += Point * Volume;
		}
	}

	namespace Machines
	{
		const MachineSpec* Of(const PartRecord& R)
		{
			const auto& T = SpawnDetail::Table();
			const auto Found = T.find(R.Type);
			if (Found == T.end()) { return nullptr; }
			if (R.Type == "wheel" && !Settings::Flag(R.Props, "free_spinning", true)) { return nullptr; }   // a solid wheel, for bolting to a motor
			return &Found->second;
		}

		int PieceCount(const PartRecord& R)
		{
			const MachineSpec* M = Of(R);
			return M ? (int)M->Pieces.size() : 1;
		}
	}

	namespace Welding
	{
		bool Touching(const Box& A, const Box& B)
		{
			const Vec3 AHigh = A.End();
			const Vec3 BHigh = B.End();
			int SharedAxes = 0;
			for (int Axis = 0; Axis < 3; ++Axis)
			{
				const double Overlap = std::min(AHigh[Axis], BHigh[Axis]) - std::max(A.Position[Axis], B.Position[Axis]);
				if (Overlap < -Touch) { return false; }   // a gap between them
				if (Overlap > Touch) { ++SharedAxes; }
			}
			return SharedAxes >= 2;
		}

		double Contact(const Box& A, const Box& B)
		{
			const Vec3 AHigh = A.End();
			const Vec3 BHigh = B.End();
			double Spans[3];
			for (int Axis = 0; Axis < 3; ++Axis)
			{
				Spans[Axis] = std::max(std::min(AHigh[Axis], BHigh[Axis]) - std::max(A.Position[Axis], B.Position[Axis]), 0.0);
			}
			std::sort(Spans, Spans + 3);
			return Spans[1] * Spans[2];
		}
	}

	// --- Making the plan ----------------------------------------------------------

	SpawnPlan SpawnPlan::Make(const BuildModel& Model)
	{
		SpawnPlan Plan;
		Plan.Records = Model.All();
		std::vector<std::pair<int, std::vector<int>>> Forced;
		Plan.ApplyOffsetters(Forced);
		Plan.CountOverlapsOnce();
		Plan.NotePieces();

		std::vector<int> AllPieces;
		for (const auto& Pair : Plan.Pieces) { AllPieces.push_back(Pair.first); }
		std::vector<std::vector<int>> Groups = Plan.WeldGroups(AllPieces);

		// Join the weld groups that an offsetter holds together.
		if (!Forced.empty())
		{
			std::map<int, size_t> GroupOf;
			for (size_t Index = 0; Index < Groups.size(); ++Index)
			{
				for (int Piece : Groups[Index]) { GroupOf[Piece] = Index; }
			}
			for (const auto& Item : Forced)
			{
				const int Holder = Item.first * PiecesPerPart;
				for (int HeldUid : Item.second)
				{
					const int Held = HeldUid * PiecesPerPart;
					if (!GroupOf.count(Holder) || !GroupOf.count(Held)) { continue; }
					const size_t Keep = GroupOf[Holder];
					const size_t Gone = GroupOf[Held];
					if (Keep == Gone) { continue; }
					for (int Piece : Groups[Gone])
					{
						Groups[Keep].push_back(Piece);
						GroupOf[Piece] = Keep;
					}
					Groups[Gone].clear();
				}
			}
			std::vector<std::vector<int>> Kept;
			for (auto& Group : Groups)
			{
				if (!Group.empty()) { Kept.push_back(Group); }
			}
			Groups = Kept;
		}

		for (const std::vector<int>& Group : Groups)
		{
			// The engine gives one friction value to a whole body, so frictionless
			// parts get a body of their own. They and everything welded to them
			// are fixed, so nothing needs to hold the two bodies together.
			std::vector<int> Slick, Rough;
			bool HasAnchor = false;
			for (int Piece : Group)
			{
				const PieceInfo& Info = Plan.Pieces[Piece];
				(Info.Immovable ? Slick : Rough).push_back(Piece);
				if (!Info.Immovable && Plan.Records[Info.Uid].Type == "anchor_block") { HasAnchor = true; }
			}
			for (int Pass = 0; Pass < 2; ++Pass)
			{
				const std::vector<int>& Members = Pass == 0 ? Slick : Rough;
				if (Members.empty()) { continue; }
				BodyPlan Body;
				Body.Pieces = Members;
				Body.Fixed = Pass == 0 || !Slick.empty() || HasAnchor;
				Plan.RefreshBody(Body);
				bool Solid = false;
				for (int Piece : Members)
				{
					if (Plan.Pieces[Piece].Solid) { Solid = true; }
				}
				if (Body.Mass <= 0.0 || !Solid) { continue; }   // nothing to see or touch
				const int Index = (int)Plan.Bodies.size();
				for (int Piece : Members) { Plan.BodyOfPiece[Piece] = Index; }
				Plan.Bodies.push_back(Body);
			}
		}

		// Join the pieces of each machine.
		for (const auto& Pair : Plan.Records)
		{
			const PartRecord& R = Pair.second;
			const MachineSpec* Machine = Machines::Of(R);
			if (!Machine || Records::IsHiddenControl(R)) { continue; }
			const Vec3 SizeM = Units::ToMetres(R.Size);
			for (const MachineJointSpec& Spec : Machine->Joints)
			{
				const int A = Pair.first * PiecesPerPart + Spec.A;
				const int B = Pair.first * PiecesPerPart + Spec.B;
				const auto BodyA = Plan.BodyOfPiece.find(A);
				const auto BodyB = Plan.BodyOfPiece.find(B);
				if (BodyA == Plan.BodyOfPiece.end() || BodyB == Plan.BodyOfPiece.end()) { continue; }
				if (BodyA->second == BodyB->second)
				{
					++Plan.Locked;
					continue;
				}
				JointPlan Joint;
				Joint.Uid = Pair.first;
				Joint.Role = Machine->Role;
				Joint.Kind = Spec.Kind;
				int AxisIndex = Spec.Axis;
				if (Joint.Kind == "end")
				{
					// A linkage bar's end is a ball joint or a hinge, as its setting says.
					const std::string Ends = Settings::Text(R.Props, "ends", "Ball joints");
					Joint.Kind = "ball";
					if (Ends == "Hinges about Y") { Joint.Kind = "hinge"; AxisIndex = 1; }
					else if (Ends == "Hinges about Z") { Joint.Kind = "hinge"; AxisIndex = 2; }
				}
				Joint.PieceA = A;
				Joint.PieceB = B;
				Joint.BodyA = BodyA->second;
				Joint.BodyB = BodyB->second;
				Joint.Axis = R.Rot.Col[AxisIndex];
				Joint.Across = R.Rot.Col[(AxisIndex + 1) % 3];
				Joint.Third = R.Rot.Col[(AxisIndex + 2) % 3];
				Joint.Pivot = R.Pos.ToVec() * Units::Unit + R.Rot * Spec.Pivot.Mul(SizeM);
				Plan.Joints.push_back(Joint);
			}
		}
		return Plan;
	}

	void SpawnPlan::NotePieces()
	{
		for (const auto& Pair : Records)
		{
			const int Uid = Pair.first;
			const PartRecord& R = Pair.second;
			if (Records::IsHiddenControl(R))
			{
				HiddenControls.push_back(Uid);
				continue;
			}
			const MaterialInfo& M = Materials::Info(R.Material);
			const MachineSpec* Machine = Machines::Of(R);
			const int Count = Machines::PieceCount(R);
			for (int Index = 0; Index < Count; ++Index)
			{
				PieceInfo Info;
				Info.Piece = Uid * PiecesPerPart + Index;
				Info.Uid = Uid;
				Info.Index = Index;
				Info.Name = Machine ? Machine->Pieces[(size_t)Index].Name : "";
				Info.Bounds = SpawnDetail::PieceBox(R, Index);
				Info.Middle = Machine ? SpawnDetail::PieceMiddle(R, Index) : R.Pos.ToVec() * Units::Unit;
				Info.PieceSize = Machine ? SpawnDetail::PieceSizeOf(R, Index) : Units::ToMetres(R.Size);
				Info.CollisionSize = Machine ? Info.PieceSize.Mul(Machine->Pieces[(size_t)Index].Trim) : Info.PieceSize;
				Info.Mass = SpawnDetail::PieceMass(R, Index);
				Info.Centre = SpawnDetail::PieceCentre(R, Index);
				Info.Friction = M.Friction;
				Info.Immovable = M.Immovable;
				Info.Solid = R.Type != "dynamic_offsetter";
				Pieces[Info.Piece] = Info;
				Boxes[Info.Piece] = Info.Bounds;
				// Pieces whose shape is not their whole box weld only where their shapes meet.
				if (!Machine && Parts::Get(R.Type).Collision != CollisionKind::Box) { Shaped.insert(Info.Piece); }
				if (M.Immovable)
				{
					const Vec3 Margin(Welding::Touch, Welding::Touch, Welding::Touch);
					const IVec3 Low = SpawnDetail::BucketOf(Info.Bounds.Position - Margin);
					const IVec3 High = SpawnDetail::BucketOf(Info.Bounds.End() + Margin);
					for (int X = Low.X; X <= High.X; ++X)
						for (int Y = Low.Y; Y <= High.Y; ++Y)
							for (int Z = Low.Z; Z <= High.Z; ++Z) { FixedBuckets[IVec3(X, Y, Z)].push_back(Info.Piece); }
				}
			}
		}
	}

	// --- Welding --------------------------------------------------------------

	std::vector<std::vector<int>> SpawnPlan::WeldGroups(const std::vector<int>& PieceList) const
	{
		std::map<int, int> Parent;
		std::map<IVec3, std::vector<int>> Buckets;
		const Vec3 Margin(Welding::Touch, Welding::Touch, Welding::Touch);
		for (int Piece : PieceList)
		{
			Parent[Piece] = Piece;
			const Box& B = Boxes.at(Piece);
			const IVec3 Low = SpawnDetail::BucketOf(B.Position - Margin);
			const IVec3 High = SpawnDetail::BucketOf(B.End() + Margin);
			for (int X = Low.X; X <= High.X; ++X)
			{
				for (int Y = Low.Y; Y <= High.Y; ++Y)
				{
					for (int Z = Low.Z; Z <= High.Z; ++Z)
					{
						std::vector<int>& Neighbours = Buckets[IVec3(X, Y, Z)];
						for (int Other : Neighbours)
						{
							if (Welds(Piece, Other)) { SpawnDetail::Join(Parent, Piece, Other); }
						}
						Neighbours.push_back(Piece);
					}
				}
			}
		}
		std::map<int, std::vector<int>> Groups;
		for (const auto& Pair : Parent)
		{
			Groups[SpawnDetail::RootOf(Parent, Pair.first)].push_back(Pair.first);
		}
		std::vector<std::vector<int>> Out;
		for (auto& Pair : Groups) { Out.push_back(Pair.second); }
		return Out;
	}

	bool SpawnPlan::Welds(int A, int B) const
	{
		if (UidOfPiece(A) == UidOfPiece(B)) { return false; }   // pieces of one machine: held by its joints
		const auto FoundA = Boxes.find(A);
		const auto FoundB = Boxes.find(B);
		if (FoundA == Boxes.end() || FoundB == Boxes.end()) { return false; }
		if (!Welding::Touching(FoundA->second, FoundB->second)) { return false; }
		if ((Shaped.count(A) || Shaped.count(B)) && !ShapesMeet(A, B, FoundA->second, FoundB->second)) { return false; }
		return Chooses(A, B) && Chooses(B, A);
	}

	bool SpawnPlan::ShapesMeet(int A, int B, const Box& BoxA, const Box& BoxB) const
	{
		const Vec3 Low = BoxA.Position.Max(BoxB.Position) - Vec3(1, 1, 1) * Welding::Touch;
		const Vec3 High = BoxA.End().Min(BoxB.End()) + Vec3(1, 1, 1) * Welding::Touch;
		const int Steps = 6;
		for (int I = 0; I <= Steps; ++I)
		{
			for (int J = 0; J <= Steps; ++J)
			{
				for (int K = 0; K <= Steps; ++K)
				{
					const Vec3 Point = Low + (High - Low).Mul(Vec3(I, J, K)) / (double)Steps;
					if (InShape(A, Point) && InShape(B, Point)) { return true; }
				}
			}
		}
		return false;
	}

	bool SpawnPlan::InShape(int Piece, const Vec3& Point) const
	{
		if (!Shaped.count(Piece)) { return true; }   // a box: the region is inside it already
		const PartRecord& R = Records.at(UidOfPiece(Piece));
		const Vec3 Size = Units::ToMetres(R.Size);
		const Vec3 Local = R.Rot.Inverse() * (Point - R.Pos.ToVec() * Units::Unit);
		const Vec3 Unit(Local.X / std::max(Size.X, 1e-6), Local.Y / std::max(Size.Y, 1e-6), Local.Z / std::max(Size.Z, 1e-6));
		// Slack of about a millimetre, as a share of the part's size.
		const double Slack = 0.002 / std::max(std::min(Size.X, std::min(Size.Y, Size.Z)), 0.01) + 0.01;
		return Geometry::ContainsUnitPoint(R.Type, Unit, Slack);
	}

	bool SpawnPlan::Chooses(int Other, int Piece) const
	{
		// Something that touches several pieces of one machine welds to only one
		// of them, or it would lock the machine solid: the one it shares the most
		// area with, the earliest listed if they are even.
		const int First = Piece - (Piece % PiecesPerPart);
		if (!Boxes.count(First + 1)) { return true; }   // not a machine
		const Box& OtherBox = Boxes.at(Other);
		int Best = -1;
		double BestContact = -1.0;
		for (int Index = 0; Index < PiecesPerPart; ++Index)
		{
			const int Candidate = First + Index;
			const auto Found = Boxes.find(Candidate);
			if (Found == Boxes.end()) { break; }
			if (!Welding::Touching(OtherBox, Found->second)) { continue; }
			const double ContactArea = Welding::Contact(OtherBox, Found->second);
			if (Best < 0 || ContactArea > BestContact * 1.02 + 0.000001)
			{
				Best = Candidate;
				BestContact = ContactArea;
			}
		}
		return Best == Piece;
	}

	bool SpawnPlan::TouchesFixed(int Piece) const
	{
		const auto Found = Boxes.find(Piece);
		if (Found == Boxes.end()) { return false; }
		const Vec3 Margin(Welding::Touch, Welding::Touch, Welding::Touch);
		const IVec3 Low = SpawnDetail::BucketOf(Found->second.Position - Margin);
		const IVec3 High = SpawnDetail::BucketOf(Found->second.End() + Margin);
		for (int X = Low.X; X <= High.X; ++X)
		{
			for (int Y = Low.Y; Y <= High.Y; ++Y)
			{
				for (int Z = Low.Z; Z <= High.Z; ++Z)
				{
					const auto Bucket = FixedBuckets.find(IVec3(X, Y, Z));
					if (Bucket == FixedBuckets.end()) { continue; }
					for (int Other : Bucket->second)
					{
						if (Other != Piece && Welds(Piece, Other)) { return true; }
					}
				}
			}
		}
		return false;
	}

	// --- Offsetters and overlaps -----------------------------------------------------

	void SpawnPlan::ApplyOffsetters(std::vector<std::pair<int, std::vector<int>>>& Forced)
	{
		std::vector<int> Uids;
		for (const auto& Pair : Records) { Uids.push_back(Pair.first); }
		for (int Uid : Uids)
		{
			const PartRecord Rec = Records[Uid];
			if (Rec.Type != "dynamic_offsetter") { continue; }
			const Vec3 ShiftLocal = Vec3(Settings::Num(Rec.Props, "shift_x", 0), Settings::Num(Rec.Props, "shift_y", 0),
				Settings::Num(Rec.Props, "shift_z", 0)) * Units::Cell;
			const Vec3 TurnLocal(Settings::Num(Rec.Props, "turn_x", 0), Settings::Num(Rec.Props, "turn_y", 0),
				Settings::Num(Rec.Props, "turn_z", 0));
			if (ShiftLocal.Length() < 0.0001 && TurnLocal.Length() < 0.01) { continue; }
			const Box OwnBox = Records::BoundsOf(Rec);
			const Vec3 Up = Rec.Rot.Col[1].Normalized();
			const Vec3 SizeM = Units::ToMetres(Rec.Size);
			const Vec3 TopMiddle = Rec.Pos.ToVec() * Units::Unit + Up * (SizeM.Y * 0.5);
			std::map<int, Box> BoxesOf;
			for (const auto& Pair : Records)
			{
				if (Pair.first != Uid) { BoxesOf[Pair.first] = Records::BoundsOf(Pair.second); }
			}
			// What sits on the top face, then everything joined on beyond it.
			std::vector<int> Seeds;
			for (const auto& Pair : BoxesOf)
			{
				if (Welding::Touching(Pair.second, OwnBox) && (Pair.second.Center() - TopMiddle).Dot(Up) > -0.001)
				{
					Seeds.push_back(Pair.first);
				}
			}
			if (Seeds.empty()) { continue; }
			std::set<int> Moving(Seeds.begin(), Seeds.end());
			std::vector<int> Queue = Seeds;
			bool Tangled = false;
			while (!Queue.empty() && !Tangled)
			{
				const int Current = Queue.back();
				Queue.pop_back();
				const Box CurrentBox = BoxesOf[Current];
				for (const auto& Pair : BoxesOf)
				{
					if (Moving.count(Pair.first) || !Welding::Touching(Pair.second, CurrentBox)) { continue; }
					if (Welding::Touching(Pair.second, OwnBox) && (Pair.second.Center() - TopMiddle).Dot(Up) <= -0.001)
					{
						Tangled = true;   // it wraps round to the offsetter's other side
						break;
					}
					Moving.insert(Pair.first);
					Queue.push_back(Pair.first);
				}
			}
			if (Tangled) { continue; }
			const Basis Turn = Rec.Rot * Basis::FromEulerYXZ(TurnLocal * (kPi / 180.0)) * Rec.Rot.Inverse();
			const Vec3 Shift = Rec.Rot * ShiftLocal;
			for (int MovedUid : Moving)
			{
				PartRecord& Moved = Records[MovedUid];
				const Vec3 At = Moved.Pos.ToVec() * Units::Unit;
				const Vec3 NewAt = TopMiddle + Turn * (At - TopMiddle) + Shift;
				Moved.Pos = IVec3::FromRounded(NewAt / Units::Unit);
				Moved.Rot = (Turn * Moved.Rot).Orthonormalized();
			}
			Forced.push_back({Uid, Seeds});
		}
	}

	void SpawnPlan::CountOverlapsOnce()
	{
		// Parts of one material may overlap. The volume they share is counted once:
		// the later part gives up its share of the overlap.
		std::map<IVec3, std::vector<int>> Buckets;
		std::map<int, Box> BoxesOf;
		for (const auto& Pair : Records)
		{
			if (Records::IsHiddenControl(Pair.second)) { continue; }
			const Box B = Records::BoundsOf(Pair.second);
			BoxesOf[Pair.first] = B;
			const IVec3 Low = IVec3::FromFloored(B.Position);
			const IVec3 High = IVec3::FromFloored(B.End());
			for (int X = Low.X; X <= High.X; ++X)
				for (int Y = Low.Y; Y <= High.Y; ++Y)
					for (int Z = Low.Z; Z <= High.Z; ++Z) { Buckets[IVec3(X, Y, Z)].push_back(Pair.first); }
		}
		std::map<int, double> Given;
		std::set<std::pair<int, int>> Checked;
		for (const auto& Bucket : Buckets)
		{
			const std::vector<int>& Members = Bucket.second;
			for (size_t I = 0; I < Members.size(); ++I)
			{
				for (size_t J = I + 1; J < Members.size(); ++J)
				{
					const int A = std::min(Members[I], Members[J]);
					const int B = std::max(Members[I], Members[J]);
					if (!Checked.insert({A, B}).second) { continue; }
					const PartRecord& RA = Records[A];
					const PartRecord& RB = Records[B];
					if (RA.Material != RB.Material) { continue; }
					const Box Shared = BoxesOf[A].Intersection(BoxesOf[B]);
					const double Volume = Shared.Volume();
					if (Volume <= 0.000001) { continue; }
					const double FractionA = Parts::Get(RA.Type).VolumeFraction;
					const double FractionB = Parts::Get(RB.Type).VolumeFraction;
					Given[B] += Volume * std::min(FractionA, FractionB);
				}
			}
		}
		for (const auto& Pair : Given)
		{
			PartRecord& R = Records[Pair.first];
			const double Own = Records::VolumeOf(R);
			if (Own > 0.0) { R.SolidShare = Clamp(1.0 - Pair.second / Own, 0.05, 1.0); }
		}
	}

	// --- Bodies -----------------------------------------------------------------

	void SpawnPlan::RefreshBody(BodyPlan& Body) const
	{
		double Mass = 0.0, Grip = 0.0;
		Vec3 Moment;
		for (int Piece : Body.Pieces)
		{
			const PieceInfo& Info = Pieces.at(Piece);
			Mass += Info.Mass;
			Moment += Info.Centre * Info.Mass;
			Grip += Info.Friction * Info.Mass;
		}
		Body.Mass = Mass;
		if (Mass <= 0.0) { return; }
		Body.Centre = Moment / Mass;
		Body.Friction = Grip / Mass;
		// Moments of inertia about the centre, from each piece's box (parallel-axis rule).
		Vec3 Total;
		for (int Piece : Body.Pieces)
		{
			const PieceInfo& Info = Pieces.at(Piece);
			const Vec3 S = Info.Bounds.Size;
			const Vec3 Away = Info.Centre - Body.Centre;
			Total += Vec3(S.Y * S.Y + S.Z * S.Z, S.X * S.X + S.Z * S.Z, S.X * S.X + S.Y * S.Y) * (Info.Mass / 12.0);
			Total += Vec3(Away.Y * Away.Y + Away.Z * Away.Z, Away.X * Away.X + Away.Z * Away.Z, Away.X * Away.X + Away.Y * Away.Y) * Info.Mass;
		}
		const double Least = std::max(Mass, 0.001) * 0.0001;
		Body.Inertia = Vec3(std::max(Total.X, Least), std::max(Total.Y, Least), std::max(Total.Z, Least));
	}

	std::vector<DragAxis> SpawnPlan::DragAreas(const BodyPlan& Body) const
	{
		std::vector<Box> InBody;
		for (int Piece : Body.Pieces)
		{
			const PieceInfo& Info = Pieces.at(Piece);
			const std::string& Type = Records.at(Info.Uid).Type;
			if (Parts::IsAeroPanel(Type) || Type == "propeller" || !Info.Solid) { continue; }
			InBody.push_back(Box(Info.Bounds.Position - Body.Centre, Info.Bounds.Size));
		}
		std::vector<DragAxis> Axes;
		const double Cell = Units::Cell;
		for (int Axis = 0; Axis < 3; ++Axis)
		{
			const int UAxis = (Axis + 1) % 3;
			const int VAxis = (Axis + 2) % 3;
			std::set<std::pair<int, int>> Cells;
			for (const Box& B : InBody)
			{
				const int U0 = (int)std::ceil(B.Position[UAxis] / Cell - 0.5);
				const int U1 = (int)std::floor(B.End()[UAxis] / Cell - 0.5);
				const int V0 = (int)std::ceil(B.Position[VAxis] / Cell - 0.5);
				const int V1 = (int)std::floor(B.End()[VAxis] / Cell - 0.5);
				for (int U = U0; U <= U1; ++U)
					for (int V = V0; V <= V1; ++V) { Cells.insert({U, V}); }
			}
			DragAxis Out;
			Out.Area = (double)Cells.size() * Cell * Cell;
			if (!Cells.empty())
			{
				double SumU = 0.0, SumV = 0.0;
				for (const auto& C : Cells)
				{
					SumU += ((double)C.first + 0.5) * Cell;
					SumV += ((double)C.second + 0.5) * Cell;
				}
				Out.Point[UAxis] = SumU / (double)Cells.size();
				Out.Point[VAxis] = SumV / (double)Cells.size();
			}
			Axes.push_back(Out);
		}
		return Axes;
	}

	std::vector<FloatCube> SpawnPlan::SealedAir(const BodyPlan& Body) const
	{
		std::vector<FloatCube> Out;
		if (Body.Fixed || Body.Pieces.empty()) { return Out; }
		const double Cell = Units::Cell;
		Box Bounds = Boxes.at(Body.Pieces.front());
		for (int Piece : Body.Pieces) { Bounds = Bounds.Merge(Boxes.at(Piece)); }
		// One empty cell of margin all round, where the flood starts.
		const IVec3 Low = IVec3::FromFloored(Bounds.Position / Cell) - IVec3(1, 1, 1);
		const IVec3 High = IVec3::FromRounded((Bounds.End() / Cell).Ceil()) + IVec3(1, 1, 1);
		const IVec3 Span = High - Low;
		const long long Total = (long long)Span.X * Span.Y * Span.Z;
		if (Total <= 0 || Total > 1500000) { return Out; }
		std::vector<uint8_t> Solid((size_t)Total, 0);
		const int StepX = Span.Y * Span.Z;
		const int StepY = Span.Z;
		for (int Piece : Body.Pieces)
		{
			if (!Pieces.at(Piece).Solid) { continue; }
			const Box& B = Boxes.at(Piece);
			const IVec3 From = IVec3::FromFloored(B.Position / Cell + Vec3(0.001, 0.001, 0.001)) - Low;
			const IVec3 To = IVec3::FromRounded((B.End() / Cell - Vec3(0.001, 0.001, 0.001)).Ceil()) - Low;
			for (int X = std::max(From.X, 0); X < std::min(To.X, Span.X); ++X)
				for (int Y = std::max(From.Y, 0); Y < std::min(To.Y, Span.Y); ++Y)
					for (int Z = std::max(From.Z, 0); Z < std::min(To.Z, Span.Z); ++Z) { Solid[(size_t)(X * StepX + Y * StepY + Z)] = 1; }
		}
		// Flood the outside: 2 marks a cell the outside water could reach.
		std::vector<int> Stack = {0};
		Solid[0] = 2;
		while (!Stack.empty())
		{
			const int At = Stack.back();
			Stack.pop_back();
			const int X = At / StepX;
			const int Y = (At % StepX) / StepY;
			const int Z = At % StepY;
			const int Next[6] = {X > 0 ? At - StepX : -1, X < Span.X - 1 ? At + StepX : -1, Y > 0 ? At - StepY : -1,
				Y < Span.Y - 1 ? At + StepY : -1, Z > 0 ? At - 1 : -1, Z < Span.Z - 1 ? At + 1 : -1};
			for (int N : Next)
			{
				if (N >= 0 && Solid[(size_t)N] == 0)
				{
					Solid[(size_t)N] = 2;
					Stack.push_back(N);
				}
			}
		}
		// What is left at 0 is sealed air.
		std::map<IVec3, std::pair<double, Vec3>> Cubes;
		const double CellVolume = Cell * Cell * Cell;
		for (long long Index = 0; Index < Total; ++Index)
		{
			if (Solid[(size_t)Index] != 0) { continue; }
			const int X = (int)(Index / StepX);
			const int Y = (int)((Index % StepX) / StepY);
			const int Z = (int)(Index % StepY);
			const Vec3 Middle = ((Low + IVec3(X, Y, Z)).ToVec() + Vec3(0.5, 0.5, 0.5)) * Cell;
			SpawnDetail::AddToCube(Cubes, Middle - Body.Centre, CellVolume, 1.0);
		}
		for (const auto& Pair : Cubes)
		{
			FloatCube Cube;
			Cube.Volume = Pair.second.first;
			Cube.Point = Pair.second.second / Pair.second.first;
			Out.push_back(Cube);
		}
		return Out;
	}

	std::vector<FloatCube> SpawnPlan::Floats(const BodyPlan& Body, double& Reach) const
	{
		std::map<IVec3, std::pair<double, Vec3>> Cubes;
		for (int Piece : Body.Pieces)
		{
			const PieceInfo& Info = Pieces.at(Piece);
			const double Density = Materials::Info(Records.at(Info.Uid).Material).Density;
			if (Density <= 0.0) { continue; }
			SpawnDetail::AddToCube(Cubes, Info.Centre - Body.Centre, Info.Mass / Density, 1.0);
		}
		for (const FloatCube& Air : SealedAir(Body)) { SpawnDetail::AddToCube(Cubes, Air.Point, Air.Volume, 1.0); }
		std::vector<FloatCube> Out;
		Reach = 0.0;
		for (const auto& Pair : Cubes)
		{
			if (Pair.second.first <= 0.0) { continue; }
			FloatCube Cube;
			Cube.Volume = Pair.second.first;
			Cube.Point = Pair.second.second / Pair.second.first;
			Reach = std::max(Reach, Cube.Point.Length());
			Out.push_back(Cube);
		}
		Reach += 1.0;
		return Out;
	}
}
