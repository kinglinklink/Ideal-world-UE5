// Ideal World - engine-free core.
//
// The air: how thick it is, and what it does to a flat aero panel (aero.gd).
// Used by Spawn for the real forces and by build mode for the centre of lift
// marker, so the two can never disagree. Pure functions on vectors, so they
// work the same in Godot's axes and Unreal's.

#pragma once

#include "IWMath.h"

namespace IW
{
	namespace Aero
	{
		constexpr double SeaLevelAir = 1.225;     // kg/m^3
		constexpr double AirScaleHeight = 8500.0; // metres; air density falls by e over this height
		constexpr double StallAngle = 15.0;       // degrees
		constexpr double StallBlend = 2.0;        // degrees either side of stall
		/** Metres above sea level of the build pad (Y = 0). */
		constexpr double BaseElevation = 901.0;

		/** Air density at a height in game metres above the pad. */
		inline double AirDensity(double Height)
		{
			return SeaLevelAir * std::exp(-std::max(Height + BaseElevation, 0.0) / AirScaleHeight);
		}

		/** Lift coefficient of a flat panel: 4 per radian, blending to a flat plate past stall. */
		inline double LiftCoefficient(double Angle)
		{
			const double Attached = 4.0 * Angle;
			const double Stalled = std::sin(2.0 * Angle);
			const double Blend = SmoothStep(DegToRad(StallAngle - StallBlend), DegToRad(StallAngle + StallBlend), Angle);
			return Lerp(Attached, Stalled, Blend);
		}

		/**
		 * The force of the air on one flat panel, in newtons. Moving is the
		 * panel's velocity through still air, Normal the direction its faces
		 * look along. Lift acts at right angles to the airflow, pushing the
		 * panel away from the face the air strikes; drag acts along the airflow.
		 * A dragless panel makes the lift and no drag at all.
		 */
		inline Vec3 PanelForce(const Vec3& Moving, const Vec3& Normal, double Area, bool Dragless, double Density)
		{
			const double Speed = Moving.Length();
			if (Speed < 0.05) { return Vec3(); }
			const Vec3 Heading = Moving / Speed;
			const double Facing = Heading.Dot(Normal);
			const double Sine = std::min(std::fabs(Facing), 1.0);
			const double Angle = std::asin(Sine);
			const double Pressure = 0.5 * Density * Speed * Speed * Area;
			Vec3 Force;
			if (!Dragless)
			{
				Force = -Heading * (Pressure * (0.01 + 2.0 * Sine * Sine));
			}
			const Vec3 Across = Normal - Heading * Facing;
			if (Across.Length() > 0.0001)
			{
				Force += Across.Normalized() * (-Sign(Facing) * Pressure * LiftCoefficient(Angle));
			}
			return Force;
		}
	}
}
