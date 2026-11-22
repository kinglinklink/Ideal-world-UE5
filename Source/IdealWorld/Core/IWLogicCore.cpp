#include "IWLogicCore.h"

namespace IW
{
	namespace Logic
	{
		bool BooleanGate(const std::string& Operation, bool A, bool B, double Seconds, double Delta, bool LastPass, GateState& State)
		{
			if (Operation == "AND") { return A && B; }
			if (Operation == "OR") { return A || B; }
			if (Operation == "NOT") { return !A; }
			if (Operation == "XOR") { return A != B; }
			if (Operation == "NAND") { return !(A && B); }
			if (Operation == "NOR") { return !(A || B); }
			if (Operation == "Latch")
			{
				// A switches it on, B switches it off.
				if (LastPass)
				{
					if (B) { State.Latched = false; }
					else if (A) { State.Latched = true; }
				}
				return State.Latched;
			}
			if (Operation == "Timer")
			{
				// When A switches on, the result stays on for the timer length.
				const bool Rising = A && !State.WasOn;
				if (LastPass)
				{
					if (Rising) { State.Timer = std::max(Seconds, 0.0); }
					else if (State.Timer > 0.0) { State.Timer = std::max(State.Timer - Delta, 0.0); }
					State.WasOn = A;
				}
				return State.Timer > 0.0 || (A && !State.WasOn);
			}
			return A && B;
		}

		void NumberGate(const std::string& Operation, double A, double B, double C, double Value, double& Result, bool& IsTrue)
		{
			Result = 0.0;
			IsTrue = false;
			bool Comparison = false;
			if (Operation == "Constant") { Result = Value; }
			else if (Operation == "Add") { Result = A + B; }
			else if (Operation == "Subtract") { Result = A - B; }
			else if (Operation == "Multiply") { Result = A * B; }
			else if (Operation == "Divide") { Result = std::fabs(B) > 1e-9 ? A / B : 0.0; }
			else if (Operation == "Absolute value") { Result = std::fabs(A); }
			else if (Operation == "Min") { Result = std::min(A, B); }
			else if (Operation == "Max") { Result = std::max(A, B); }
			else if (Operation == "Clamp") { Result = Clamp(A, std::min(B, C), std::max(B, C)); }
			else if (Operation == "Compare")
			{
				Comparison = true;
				IsTrue = A > B;
				Result = IsTrue ? 1.0 : 0.0;
			}
			else if (Operation == "Select") { Result = A != 0.0 ? B : C; }
			else if (Operation == "Map range") { Result = std::fabs(C - B) > 1e-9 ? (A - B) / (C - B) : 0.0; }
			else if (Operation == "Modulo") { Result = std::fabs(B) > 1e-9 ? PosMod(A, B) : 0.0; }
			else if (Operation == "Between")
			{
				Comparison = true;
				IsTrue = A >= std::min(B, C) && A <= std::max(B, C);
				Result = IsTrue ? 1.0 : 0.0;
			}
			if (!std::isfinite(Result)) { Result = 0.0; }
			if (!Comparison) { IsTrue = Result != 0.0; }
		}

		double Pid(double Setpoint, double Measured, double Kp, double Ki, double Kd, double Low, double High, double Delta,
			GateState& State)
		{
			if (High < Low) { std::swap(Low, High); }
			const double Error = Setpoint - Measured;
			double Rate = 0.0;
			if (State.Primed) { Rate = -(Measured - State.Last) / std::max(Delta, 1e-6); }
			State.Last = Measured;
			State.Primed = true;
			double Integral = State.Integral + Error * Delta;
			double Output = Kp * Error + Ki * Integral + Kd * Rate;
			// Stop winding up the integral while the output is pinned at a limit.
			if ((Output > High && Error > 0.0) || (Output < Low && Error < 0.0))
			{
				Integral = State.Integral;
				Output = Kp * Error + Ki * Integral + Kd * Rate;
			}
			State.Integral = Integral;
			return std::isfinite(Output) ? Clamp(Output, Low, High) : 0.0;
		}

		bool Pwm(bool Enabled, double Frequency, double Duty, double Delta, int PassIndex, GateState& State, double& Phase)
		{
			if (!State.Primed)
			{
				State.Primed = true;
				State.Last = 0.0;
			}
			if (PassIndex == 0)
			{
				State.Last = Enabled ? PosMod(State.Last + Frequency * Delta, 1.0) : 0.0;
			}
			Phase = State.Last;
			return Enabled && Duty > 0.0 && (Duty >= 1.0 || Phase < Duty);
		}

		void EaseToward(double& Setpoint, double& Rate, double Target, double TopSpeed, double Easing, double Delta)
		{
			const double Error = Target - Setpoint;
			double Approach = TopSpeed;
			if (Easing > 0.0)
			{
				Approach = std::min(TopSpeed, std::sqrt(2.0 * Easing * std::fabs(Error)));
				Rate = MoveToward(Rate, Sign(Error) * Approach, Easing * Delta);
			}
			else
			{
				Rate = Sign(Error) * Approach;
			}
			if (std::fabs(Error) <= std::fabs(Rate) * Delta)
			{
				Setpoint = Target;
				Rate = 0.0;
				return;
			}
			Setpoint += Rate * Delta;
		}
	}
}
