// Ideal World - engine-free core.
//
// Spawn's planning, from spawner.gd: which parts come apart into pieces
// (MACHINES), which pieces weld into one rigid body, each body's mass, centre
// of mass, inertia, drag outline and sealed air, and the joints between
// pieces. The game layer turns the plan into physics bodies and constraints.
//
// Everything here is in the grid's space (Godot axes, metres). A body's own
// frame keeps the grid's axes, with its origin at its centre of mass.

#pragma once

#include "IWBuildModel.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace IW
{
	constexpr int PiecesPerPart = 8;   // a part's pieces are numbered uid * 8 + 0, 1, 2 ...
	inline int UidOfPiece(int Piece) { return Piece / PiecesPerPart; }

	struct MachinePieceSpec
	{
		std::string Name;   // "base", "top", "bar", "end_a", "end_b"
		Vec3 Middle;        // share of the part's size, along its own axes
		Vec3 Size;          // share of the part's size
		Vec3 Trim;          // scales the collision box so a square piece has room to turn
	};

	struct MachineJointSpec
	{
		int A = 0, B = 1;
		std::string Kind;   // "hinge", "slide", "ball", "end"
		int Axis = 1;       // 0 X, 1 Y, 2 Z of the part
		Vec3 Pivot;         // share of the part's size
	};

	struct MachineSpec
	{
		std::string Role;   // what runs it: hinge, motor, servo, ball, bearing, piston, cylinder, solenoid, spring, crank, wheel, link, clutch, brake
		std::vector<MachinePieceSpec> Pieces;
		std::vector<MachineJointSpec> Joints;
	};

	namespace Machines
	{
		/** The machine a part comes apart into, or null for a part that stays whole. */
		const MachineSpec* Of(const PartRecord& R);
		int PieceCount(const PartRecord& R);
	}

	struct PieceInfo
	{
		int Piece = 0;
		int Uid = 0;
		int Index = 0;
		std::string Name;       // machine piece name, or "" for a whole part
		Box Bounds;             // in the grid's space
		Vec3 Middle;            // the piece's box centre, grid space
		Vec3 PieceSize;         // along the part's own axes, metres
		Vec3 CollisionSize;     // the piece's collision box size (machine pieces)
		double Mass = 0.0;
		Vec3 Centre;            // centre of mass, grid space
		double Friction = 0.5;
		bool Immovable = false;
		bool Solid = true;      // false for a dynamic offsetter, which welds but is not there once spawned
	};

	struct BodyPlan
	{
		std::vector<int> Pieces;
		bool Fixed = false;
		double Mass = 0.0;
		Vec3 Centre;            // grid space; the body's origin
		double Friction = 0.5;  // mass-weighted average coefficient
		Vec3 Inertia;           // principal moments about the centre, along the grid's axes, kg m^2
	};

	struct JointPlan
	{
		int Uid = 0;
		std::string Role;
		std::string Kind;       // hinge, slide, ball
		int PieceA = 0, PieceB = 0;
		int BodyA = -1, BodyB = -1;
		Vec3 Axis, Across, Third;   // grid directions
		Vec3 Pivot;                 // grid space
	};

	struct DragAxis
	{
		double Area = 0.0;      // m^2 of outline seen along this axis
		Vec3 Point;             // middle of that outline, in the body (the axis's own coordinate set to 0)
	};

	struct FloatCube
	{
		Vec3 Point;             // in the body, from its origin
		double Volume = 0.0;
	};

	namespace Welding
	{
		constexpr double Touch = 0.0004;   // metres; faces this close count as touching
		constexpr double Bucket = 1.0;     // metres; sorting boxes for finding neighbours

		/** True if two boxes overlap, or meet over an area rather than a line or point. */
		bool Touching(const Box& A, const Box& B);
		/** The area of the face two touching boxes meet over. */
		double Contact(const Box& A, const Box& B);
	}

	/** The whole plan for one spawn. */
	class SpawnPlan
	{
	public:
		/** Records as spawned: offsetters applied, overlap counted once. */
		std::map<int, PartRecord> Records;
		std::map<int, PieceInfo> Pieces;
		std::vector<BodyPlan> Bodies;
		std::map<int, int> BodyOfPiece;     // piece -> index into Bodies
		std::vector<JointPlan> Joints;      // joints whose two sides are on different bodies
		int Locked = 0;                     // joints whose two sides ended up on one body
		std::vector<int> HiddenControls;    // control parts with no body

		static SpawnPlan Make(const BuildModel& Model);

		/** Weld groups among a set of pieces (all pieces' boxes must be in Pieces). */
		std::vector<std::vector<int>> WeldGroups(const std::vector<int>& PieceList) const;
		bool Welds(int A, int B) const;
		/** True if a piece welds to an immovable piece. */
		bool TouchesFixed(int Piece) const;

		/** Fills mass, centre, friction and inertia of a body from its pieces. */
		void RefreshBody(BodyPlan& Body) const;
		/** The outline a body shows the air along each of its axes. */
		std::vector<DragAxis> DragAreas(const BodyPlan& Body) const;
		/**
		 * The floats of a body: each piece's solid volume plus the air sealed
		 * inside the body, merged into 1 m cubes. Points are in the body.
		 */
		std::vector<FloatCube> Floats(const BodyPlan& Body, double& Reach) const;

	private:
		void ApplyOffsetters(std::vector<std::pair<int, std::vector<int>>>& Forced);
		void CountOverlapsOnce();
		void NotePieces();
		bool ShapesMeet(int A, int B, const Box& BoxA, const Box& BoxB) const;
		bool InShape(int Piece, const Vec3& Point) const;
		bool Chooses(int Other, int Piece) const;
		std::vector<FloatCube> SealedAir(const BodyPlan& Body) const;

		std::map<int, Box> Boxes;           // piece -> bounding box, grid space
		std::set<int> Shaped;               // pieces whose shape is less than their box
		std::map<IVec3, std::vector<int>> FixedBuckets;
	};
}
