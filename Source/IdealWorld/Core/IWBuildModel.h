// Ideal World - engine-free core.
//
// The build pad's contents: every part, with its exact size and position
// (build_grid.gd without the drawing). The grid's minimum corner is the
// origin and Y is up. A part's position is the centre of its bounding box.
// Positions and sizes are whole numbers of units, so nothing drifts.

#pragma once

#include "IWMath.h"
#include "IWParts.h"
#include "IWUnits.h"

#include <deque>
#include <functional>
#include <tuple>
#include <map>
#include <string>
#include <vector>

namespace IW
{
	struct Link
	{
		int Uid = 0;              // the part the signal comes from
		std::string Output;       // which of its outputs
		bool operator==(const Link& O) const { return Uid == O.Uid && Output == O.Output; }
		bool operator!=(const Link& O) const { return !(*this == O); }
	};

	using Links = std::map<std::string, Link>;   // input id -> source

	struct PartRecord
	{
		std::string Type = "block";
		IVec3 Pos;                // units, centre of the part, from the grid corner
		Basis Rot;                // quarter-turn rotation
		IVec3 Size;               // units, along the part's own axes
		std::string Material = "plastic";
		IW::Color PartColor;
		IW::Props Props;          // settings by id
		IW::Links Links;          // input id -> source
		double SolidShare = 1.0;  // set at spawn: share of its volume not already counted in an overlapping part
		int CopiedFrom = 0;       // clipboards: the uid it was copied from

		bool operator==(const PartRecord& O) const
		{
			return Type == O.Type && Pos == O.Pos && Rot == O.Rot && Size == O.Size && Material == O.Material
				&& PartColor == O.PartColor && Props == O.Props && Links == O.Links;
		}
		bool operator!=(const PartRecord& O) const { return !(*this == O); }
	};

	namespace Records
	{
		PartRecord Make(const std::string& Type, const IVec3& Pos, const Basis& Rot, const IVec3& Size,
			const std::string& Material, const IW::Color& C);
		IVec3 DefaultSizeUnits(const std::string& Type);
		/** Half the size of a part's world-aligned bounding box, in units (may end in .5). */
		Vec3 HalfExtentUnits(const PartRecord& R);
		/** A part's bounding box in the grid's space, in metres. */
		Box BoundsOf(const PartRecord& R);
		/** Volume in cubic metres. */
		double VolumeOf(const PartRecord& R);
		/** Control parts that only think: weightless, shapeless and hidden once spawned. */
		bool IsHiddenControl(const PartRecord& R);
		double MassOf(const PartRecord& R);
		double HealthOf(const PartRecord& R);
		/** A part's centre of mass in the grid's space, in metres. */
		Vec3 CentroidOf(const PartRecord& R);
		IVec3 CellCenterUnits(const IVec3& Cell);
		IVec3 PointToCell(const Vec3& LocalPoint);
	}

	/**
	 * Every part on the pad, with undo and redo. Uids start at 1 and are never
	 * reused within a session. Changes go through here so that Undo sees them.
	 */
	class BuildModel
	{
	public:
		using Snapshot = std::map<int, PartRecord>;

		/** Called after every change (or once at the end of a batch). */
		std::function<void()> OnChanged;

		int PartCount() const { return (int)Parts.size(); }
		bool HasPart(int Uid) const { return Parts.count(Uid) > 0; }
		std::vector<int> AllUids() const;
		const PartRecord* Find(int Uid) const;
		/** A copy of a part's record (empty block record if missing). */
		PartRecord Get(int Uid) const;
		const std::map<int, PartRecord>& All() const { return Parts; }

		/** "" if a part may sit where the record says, or the reason it may not. */
		std::string CanPlace(const PartRecord& R, const std::vector<int>& Ignore = {}) const;
		/** Every part overlapping this one, directly or through a chain. Includes itself. */
		std::vector<int> OverlapGroup(int Uid) const;
		/** Every link that reads from a part: [consumer uid, input id, output id]. */
		std::vector<std::tuple<int, std::string, std::string>> ConsumersOf(int Uid) const;
		/** The nearest part of a kind whose centre is within Reach metres. 0 for none. */
		int NearestOfType(const std::string& Type, const Vec3& Point, double Reach) const;

		double TotalMass() const;
		Vec3 CenterOfMass() const;
		/** Centre of lift for level flight at 5 degrees; false if nothing makes lift. */
		bool CenterOfLift(Vec3& Out) const;

		// Changes
		void BeginBatch();
		void EndBatch();
		void Clear();
		/** Adds a part. Returns its uid, or 0 with nothing changed if it does not fit. */
		int AddPart(const PartRecord& R);
		bool RemovePart(int Uid);
		void RemoveParts(const std::vector<int>& Uids);
		/** Replaces several records at once; "" or the reason nothing changed. */
		std::string UpdateParts(const std::map<int, PartRecord>& Updates);
		void SetLinks(int Uid, const IW::Links& NewLinks);
		/** Paints a part and everything overlapping it, so overlaps stay one material. */
		void Paint(int Uid, const std::string& Material, const IW::Color& C);

		bool CanUndo() const { return !UndoStack.empty(); }
		bool CanRedo() const { return !RedoStack.empty(); }
		bool Undo();
		bool Redo();
		/** Uids whose records changed in the last Undo or Redo (to rebuild their drawing). */
		int ChangeCounter() const { return Counter; }

	private:
		Snapshot TakeSnapshot() const { return Parts; }
		void PushUndo(const Snapshot& State);
		void Remember();
		void Changed();
		void PruneLinks();
		void Restore(const Snapshot& State);

		std::map<int, PartRecord> Parts;
		int NextUid = 1;
		bool Batching = false;
		bool BatchDirty = false;
		Snapshot BatchBefore;
		std::deque<Snapshot> UndoStack;
		std::vector<Snapshot> RedoStack;
		int Counter = 0;
	};
}
