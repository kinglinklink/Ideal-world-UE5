// Ideal World - engine-free core.
//
// Small vector types used by the core. Everything in the core is in the
// Godot version's conventions, so that save files, part layouts and every
// formula carry over unchanged:
//   metres, kilograms, seconds; Y up; right-handed; a part's "forward" is -Z.
// The game layer (Game/IWConv.h) turns these into Unreal's centimetres and
// Z-up axes in exactly one place.
//
// Names here avoid Unreal's macros (check, PI, DELTA, IN, OUT ...) because
// these files are also compiled inside the Unreal module.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace IW
{
	constexpr double kPi = 3.14159265358979323846;
	constexpr double kTau = 2.0 * kPi;

	inline double Clamp(double V, double Lo, double Hi) { return V < Lo ? Lo : (V > Hi ? Hi : V); }
	inline int ClampInt(int V, int Lo, int Hi) { return V < Lo ? Lo : (V > Hi ? Hi : V); }
	inline double Lerp(double A, double B, double T) { return A + (B - A) * T; }
	inline double DegToRad(double D) { return D * kPi / 180.0; }
	inline double RadToDeg(double R) { return R * 180.0 / kPi; }
	inline double Sign(double V) { return V > 0.0 ? 1.0 : (V < 0.0 ? -1.0 : 0.0); }
	inline double MoveToward(double From, double To, double Step)
	{
		if (std::fabs(To - From) <= Step) { return To; }
		return From + Sign(To - From) * Step;
	}
	inline double SmoothStep(double From, double To, double X)
	{
		if (From == To) { return X < From ? 0.0 : 1.0; }
		const double T = Clamp((X - From) / (To - From), 0.0, 1.0);
		return T * T * (3.0 - 2.0 * T);
	}
	/** Godot's fposmod: always in [0, B). */
	inline double PosMod(double A, double B)
	{
		double R = std::fmod(A, B);
		if ((R < 0.0 && B > 0.0) || (R > 0.0 && B < 0.0)) { R += B; }
		return R;
	}
	inline int PosModInt(int A, int B)
	{
		int R = A % B;
		if (R < 0) { R += B; }
		return R;
	}
	/** Wraps into [Lo, Hi). */
	inline double Wrap(double V, double Lo, double Hi)
	{
		const double Range = Hi - Lo;
		if (Range == 0.0) { return Lo; }
		return Lo + PosMod(V - Lo, Range);
	}

	struct Vec3
	{
		double X = 0.0, Y = 0.0, Z = 0.0;

		Vec3() = default;
		constexpr Vec3(double InX, double InY, double InZ) : X(InX), Y(InY), Z(InZ) {}

		double& operator[](int I) { return I == 0 ? X : (I == 1 ? Y : Z); }
		double operator[](int I) const { return I == 0 ? X : (I == 1 ? Y : Z); }

		Vec3 operator+(const Vec3& O) const { return Vec3(X + O.X, Y + O.Y, Z + O.Z); }
		Vec3 operator-(const Vec3& O) const { return Vec3(X - O.X, Y - O.Y, Z - O.Z); }
		Vec3 operator-() const { return Vec3(-X, -Y, -Z); }
		Vec3 operator*(double S) const { return Vec3(X * S, Y * S, Z * S); }
		Vec3 operator/(double S) const { return Vec3(X / S, Y / S, Z / S); }
		Vec3& operator+=(const Vec3& O) { X += O.X; Y += O.Y; Z += O.Z; return *this; }
		Vec3& operator-=(const Vec3& O) { X -= O.X; Y -= O.Y; Z -= O.Z; return *this; }
		Vec3& operator*=(double S) { X *= S; Y *= S; Z *= S; return *this; }
		/** Component-wise product (Godot's Vector3 * Vector3). */
		Vec3 Mul(const Vec3& O) const { return Vec3(X * O.X, Y * O.Y, Z * O.Z); }

		double Dot(const Vec3& O) const { return X * O.X + Y * O.Y + Z * O.Z; }
		Vec3 Cross(const Vec3& O) const { return Vec3(Y * O.Z - Z * O.Y, Z * O.X - X * O.Z, X * O.Y - Y * O.X); }
		double LengthSquared() const { return X * X + Y * Y + Z * Z; }
		double Length() const { return std::sqrt(LengthSquared()); }
		Vec3 Normalized() const
		{
			const double L = Length();
			return L > 1e-12 ? *this / L : Vec3();
		}
		Vec3 Abs() const { return Vec3(std::fabs(X), std::fabs(Y), std::fabs(Z)); }
		Vec3 Floor() const { return Vec3(std::floor(X), std::floor(Y), std::floor(Z)); }
		Vec3 Ceil() const { return Vec3(std::ceil(X), std::ceil(Y), std::ceil(Z)); }
		/** Godot rounds halves away from zero. */
		Vec3 Round() const { return Vec3(std::round(X), std::round(Y), std::round(Z)); }
		Vec3 Min(const Vec3& O) const { return Vec3(std::min(X, O.X), std::min(Y, O.Y), std::min(Z, O.Z)); }
		Vec3 Max(const Vec3& O) const { return Vec3(std::max(X, O.X), std::max(Y, O.Y), std::max(Z, O.Z)); }
		int MaxAxisIndex() const
		{
			if (X >= Y && X >= Z) { return 0; }
			return Y >= Z ? 1 : 2;
		}
		Vec3 LimitLength(double MaxLength) const
		{
			const double L = Length();
			return (L > MaxLength && L > 0.0) ? *this * (MaxLength / L) : *this;
		}
		bool IsNearlyEqual(const Vec3& O, double Tol = 1e-6) const
		{
			return std::fabs(X - O.X) <= Tol && std::fabs(Y - O.Y) <= Tol && std::fabs(Z - O.Z) <= Tol;
		}
	};
	inline Vec3 operator*(double S, const Vec3& V) { return V * S; }

	struct IVec3
	{
		int X = 0, Y = 0, Z = 0;

		IVec3() = default;
		constexpr IVec3(int InX, int InY, int InZ) : X(InX), Y(InY), Z(InZ) {}
		static IVec3 FromRounded(const Vec3& V)
		{
			return IVec3((int)std::lround(V.X), (int)std::lround(V.Y), (int)std::lround(V.Z));
		}
		static IVec3 FromFloored(const Vec3& V)
		{
			return IVec3((int)std::floor(V.X), (int)std::floor(V.Y), (int)std::floor(V.Z));
		}

		int& operator[](int I) { return I == 0 ? X : (I == 1 ? Y : Z); }
		int operator[](int I) const { return I == 0 ? X : (I == 1 ? Y : Z); }
		IVec3 operator+(const IVec3& O) const { return IVec3(X + O.X, Y + O.Y, Z + O.Z); }
		IVec3 operator-(const IVec3& O) const { return IVec3(X - O.X, Y - O.Y, Z - O.Z); }
		IVec3 operator*(int S) const { return IVec3(X * S, Y * S, Z * S); }
		bool operator==(const IVec3& O) const { return X == O.X && Y == O.Y && Z == O.Z; }
		bool operator!=(const IVec3& O) const { return !(*this == O); }
		bool operator<(const IVec3& O) const
		{
			if (X != O.X) { return X < O.X; }
			if (Y != O.Y) { return Y < O.Y; }
			return Z < O.Z;
		}
		Vec3 ToVec() const { return Vec3((double)X, (double)Y, (double)Z); }
	};

	/**
	 * A 3x3 rotation, stored as its three columns: where the part's own X, Y
	 * and Z axes point. Same layout as Godot's Basis (Basis(x, y, z) takes
	 * columns), so `B * v = B.Col[0] * v.x + B.Col[1] * v.y + B.Col[2] * v.z`.
	 */
	struct Basis
	{
		Vec3 Col[3] = { Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1) };

		Basis() = default;
		Basis(const Vec3& InX, const Vec3& InY, const Vec3& InZ) { Col[0] = InX; Col[1] = InY; Col[2] = InZ; }

		static Basis Identity() { return Basis(); }

		/** Rotation by Angle radians about a unit Axis (right-hand rule). */
		static Basis FromAxisAngle(const Vec3& Axis, double Angle)
		{
			const Vec3 A = Axis.Normalized();
			const double C = std::cos(Angle), S = std::sin(Angle), T = 1.0 - C;
			// Rows of the rotation matrix.
			const Vec3 R0(T * A.X * A.X + C, T * A.X * A.Y - S * A.Z, T * A.X * A.Z + S * A.Y);
			const Vec3 R1(T * A.X * A.Y + S * A.Z, T * A.Y * A.Y + C, T * A.Y * A.Z - S * A.X);
			const Vec3 R2(T * A.X * A.Z - S * A.Y, T * A.Y * A.Z + S * A.X, T * A.Z * A.Z + C);
			return Basis(Vec3(R0.X, R1.X, R2.X), Vec3(R0.Y, R1.Y, R2.Y), Vec3(R0.Z, R1.Z, R2.Z));
		}

		/** Godot's Basis.from_euler with the default YXZ order: Ry * Rx * Rz. */
		static Basis FromEulerYXZ(const Vec3& Radians)
		{
			return FromAxisAngle(Vec3(0, 1, 0), Radians.Y) * FromAxisAngle(Vec3(1, 0, 0), Radians.X)
				* FromAxisAngle(Vec3(0, 0, 1), Radians.Z);
		}

		Vec3 operator*(const Vec3& V) const { return Col[0] * V.X + Col[1] * V.Y + Col[2] * V.Z; }
		Basis operator*(const Basis& O) const { return Basis(*this * O.Col[0], *this * O.Col[1], *this * O.Col[2]); }
		/** Row I of the matrix. */
		Vec3 Row(int I) const { return Vec3(Col[0][I], Col[1][I], Col[2][I]); }
		Basis Transposed() const { return Basis(Row(0), Row(1), Row(2)); }
		/** For a rotation the inverse is the transpose. */
		Basis Inverse() const { return Transposed(); }
		double Determinant() const { return Col[0].Dot(Col[1].Cross(Col[2])); }
		Basis Rounded() const { return Basis(Col[0].Round(), Col[1].Round(), Col[2].Round()); }
		Basis Orthonormalized() const
		{
			Vec3 X = Col[0].Normalized();
			Vec3 Y = (Col[1] - X * X.Dot(Col[1])).Normalized();
			Vec3 Z = (Col[2] - X * X.Dot(Col[2]) - Y * Y.Dot(Col[2])).Normalized();
			return Basis(X, Y, Z);
		}
		bool IsNearlyEqual(const Basis& O, double Tol = 1e-5) const
		{
			return Col[0].IsNearlyEqual(O.Col[0], Tol) && Col[1].IsNearlyEqual(O.Col[1], Tol) && Col[2].IsNearlyEqual(O.Col[2], Tol);
		}
		bool operator==(const Basis& O) const { return IsNearlyEqual(O, 1e-9); }
		bool operator!=(const Basis& O) const { return !(*this == O); }
	};

	/** A quarter turn about a world axis (0 X, 1 Y, 2 Z), with exact 0 / +1 / -1 entries. */
	inline Basis QuarterTurn(int AxisIndex, int Quarters = 1)
	{
		Vec3 Axis;
		Axis[ClampInt(AxisIndex, 0, 2)] = 1.0;
		return Basis::FromAxisAngle(Axis, (double)Quarters * kPi * 0.5).Rounded();
	}

	/** An axis-aligned box: position (low corner) and size, like Godot's AABB. */
	struct Box
	{
		Vec3 Position;
		Vec3 Size;

		Box() = default;
		Box(const Vec3& InPosition, const Vec3& InSize) : Position(InPosition), Size(InSize) {}

		Vec3 End() const { return Position + Size; }
		Vec3 Center() const { return Position + Size * 0.5; }
		Box Grow(double By) const { return Box(Position - Vec3(By, By, By), Size + Vec3(By, By, By) * 2.0); }
		Box Merge(const Box& O) const
		{
			const Vec3 Lo = Position.Min(O.Position);
			const Vec3 Hi = End().Max(O.End());
			return Box(Lo, Hi - Lo);
		}
		/** Godot's AABB.intersects: overlapping with positive volume. */
		bool Intersects(const Box& O) const
		{
			const Vec3 AEnd = End();
			const Vec3 BEnd = O.End();
			for (int I = 0; I < 3; ++I)
			{
				if (Position[I] >= BEnd[I] || AEnd[I] <= O.Position[I]) { return false; }
			}
			return true;
		}
		Box Intersection(const Box& O) const
		{
			const Vec3 Lo = Position.Max(O.Position);
			const Vec3 Hi = End().Min(O.End());
			const Vec3 S = Hi - Lo;
			if (S.X < 0.0 || S.Y < 0.0 || S.Z < 0.0) { return Box(); }
			return Box(Lo, S);
		}
		double Volume() const { return Size.X * Size.Y * Size.Z; }
	};

	struct Color
	{
		float R = 1.0f, G = 1.0f, B = 1.0f, A = 1.0f;

		Color() = default;
		constexpr Color(float InR, float InG, float InB, float InA = 1.0f) : R(InR), G(InG), B(InB), A(InA) {}
		bool operator==(const Color& O) const
		{
			return std::fabs(R - O.R) < 0.0025f && std::fabs(G - O.G) < 0.0025f && std::fabs(B - O.B) < 0.0025f && std::fabs(A - O.A) < 0.0025f;
		}
		bool operator!=(const Color& O) const { return !(*this == O); }
	};
}
