// Ideal World - engine-free core.
//
// Every part's shape: the flat-shaded model drawn for it (part_library.gd,
// mech_models.gd and mesh_kit.gd), the convex pieces it collides as, and an
// exact test of whether a point lies inside it. All in a 1 x 1 x 1 box
// centred on the origin; a part's real shape is that box stretched to its
// size.

#pragma once

#include "IWMath.h"

#include <string>
#include <vector>

namespace IW
{
	/** One triangle. (B - A) x (C - A) points out of the solid. */
	struct Tri
	{
		Vec3 A, B, C;
		uint8_t Surface = 0;   // 0 the paintable body, 1 the detail
	};

	struct MeshData
	{
		std::vector<Tri> Tris;
		bool HasDetail = false;
		/** The look of surface 1: dark metal unless the model gives its own. */
		Color DetailColor = Color(0.24f, 0.25f, 0.28f);
		float DetailMetallic = 0.7f;
		float DetailRoughness = 0.45f;
		bool DetailIsLens = false;   // a light panel's or screen's glass
	};

	/** Settings that change a model's shape. */
	struct MeshOptions
	{
		std::string Half;     // "" whole part; "base", "top", "coils", "bar", "end_a", "end_b" for one piece
		double Reach = 0.0;   // pistons and solenoids: extra rod, as a share of the part's height
		int Teeth = 12;
		int Blades = 4;
		int PitchDeg = 20;
		std::string Key() const;
	};

	/** A transform with a basis that may stretch (Godot's Transform3D). */
	struct Xform
	{
		Basis B;
		Vec3 O;
		Xform() = default;
		Xform(const Basis& InB, const Vec3& InO) : B(InB), O(InO) {}
		Vec3 Apply(const Vec3& V) const { return B * V + O; }
		Xform operator*(const Xform& Other) const { return Xform(B * Other.B, B * Other.O + O); }
	};

	namespace Geometry
	{
		/** The unit model for a part (cached). */
		const MeshData& UnitMesh(const std::string& Type, const MeshOptions& Options = MeshOptions());
		/** True if the part has a model of its own (otherwise a plain shape). */
		bool HasMechModel(const std::string& Type);

		/** The distinct corner points of a part's unit shape (its convex hull's points). */
		const std::vector<Vec3>& UnitHullPoints(const std::string& Type);
		/**
		 * The convex pieces a part collides as, at unit size. One piece for most
		 * parts; a hollow part (concave curve, arch, ring, pipe, torus) is split
		 * into solid convex pieces that follow its outline.
		 */
		const std::vector<std::vector<Vec3>>& UnitConvexPieces(const std::string& Type);
		/** Whether a point in unit space is inside the part's real shape, give or take Slack. */
		bool ContainsUnitPoint(const std::string& Type, const Vec3& Point, double Slack = 0.02);

		/** Ray against triangle. Returns the distance along Dir (unit) or a negative number. */
		double RayTriangle(const Vec3& Origin, const Vec3& Dir, const Vec3& A, const Vec3& B, const Vec3& C);
		/** Ray against box. Returns the entry distance or a negative number. */
		double RayBox(const Vec3& Origin, const Vec3& Dir, const Box& B);

		/** Triangulates a simple polygon (either winding). Indices into Outline, three per triangle. */
		std::vector<int> TriangulatePolygon(const std::vector<std::pair<double, double>>& Outline);
		/** Splits a simple polygon into convex polygons. */
		std::vector<std::vector<std::pair<double, double>>> ConvexDecompose(const std::vector<std::pair<double, double>>& Outline);
	}
}
