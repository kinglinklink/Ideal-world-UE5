// Ideal World - engine-free core.
//
// The one place that fixes the game's scale. Change Cell to resize the grid.
//
// Sizes and positions are stored as whole numbers of "units". One unit is the
// smallest increment the build tools allow: 0.005 of a grid cell. Whole numbers
// never pick up rounding error, however often a part is moved or resized.

#pragma once

#include "IWMath.h"

namespace IW
{
	namespace Units
	{
		constexpr double Cell = 0.25;               // metres per grid cell
		constexpr int UnitsPerCell = 200;           // so one unit is 0.005 of a cell
		constexpr int HalfCellUnits = 100;
		constexpr double Unit = Cell / 200.0;       // metres per unit
		constexpr double Gravity = 9.81;            // m/s^2
		/** The friction coefficient the world's fixed surfaces stand for. */
		constexpr double WorldFriction = 1.0;

		inline Vec3 ToMetres(const IVec3& U) { return U.ToVec() * Unit; }
		inline double ToMetres(int U) { return (double)U * Unit; }
	}

	/** The build pad: 256 x 128 x 256 cells (64 x 32 x 64 m). */
	namespace GridSize
	{
		constexpr int CellsX = 256;
		constexpr int CellsY = 128;
		constexpr int CellsZ = 256;
		constexpr int MaxSizeUnits = 64 * 200;     // largest single part: 64 cells per side
		constexpr double OverlapEpsilon = 0.0001;  // metres; keeps touching parts from counting as overlapping
		constexpr int HistoryLimit = 100;          // how many steps Undo can go back
		inline Vec3 SizeMetres() { return Vec3(CellsX, CellsY, CellsZ) * Units::Cell; }
	}
}
