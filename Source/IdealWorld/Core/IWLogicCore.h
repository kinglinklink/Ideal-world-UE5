// Ideal World - engine-free core.
//
// The arithmetic of the logic parts (logic.gd) and the drives (spawner.gd):
// boolean gates, number logic, PID, PWM, and the eased setpoint every servo
// and piston follows. The game layer feeds them signals each physics tick.

#pragma once

#include "IWMath.h"

#include <string>

namespace IW
{
	namespace Logic
	{
		constexpr int GatePasses = 4;   // chains of up to this many gates settle in one tick

		struct GateState
		{
			bool Latched = false;
			bool WasOn = false;
			double Timer = 0.0;
			double Integral = 0.0;
			double Last = 0.0;
			bool Primed = false;
		};

		/** A boolean gate's result. Latch and Timer only change state on the last pass. */
		bool BooleanGate(const std::string& Operation, bool A, bool B, double Seconds, double Delta, bool LastPass, GateState& State);

		/** Number logic: result and whether its comparison is true. */
		void NumberGate(const std::string& Operation, double A, double B, double C, double Value, double& Result, bool& IsTrue);

		/** A PID controller, once a tick. Rate is taken from the measurement, so a setpoint jump does not kick. */
		double Pid(double Setpoint, double Measured, double Kp, double Ki, double Kd, double Low, double High, double Delta,
			GateState& State);

		/** A PWM generator: advances its phase on pass 0. Returns the output; Phase out. */
		bool Pwm(bool Enabled, double Frequency, double Duty, double Delta, int PassIndex, GateState& State, double& Phase);

		/**
		 * Moves Setpoint toward Target no faster than TopSpeed, easing away from
		 * rest and easing in to stop on the target. Rate is the speed it moves at.
		 */
		void EaseToward(double& Setpoint, double& Rate, double Target, double TopSpeed, double Easing, double Delta);
	}
}
