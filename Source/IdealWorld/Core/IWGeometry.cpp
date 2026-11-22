#include "IWGeometry.h"

#include "IWParts.h"

#include <map>
#include <set>

namespace IW
{
	namespace GeometryDetail
	{
		using Outline2 = std::vector<std::pair<double, double>>;

		struct Recipe
		{
			int Kind = 0;          // 0 none, 1 extrude, 2 lathe
			Outline2 Outline;
			int Segments = 0;
		};

		std::map<std::string, Recipe>& Recipes()
		{
			static std::map<std::string, Recipe> R;
			return R;
		}

		Vec3 AxisVector(int Axis)
		{
			Vec3 V;
			V[Axis] = 1.0;
			return V;
		}

		/** Adds one triangle, wound so that (B - A) x (C - A) points along Outward. */
		void AddTriangle(MeshData& Mesh, uint8_t Surface, const Vec3& A, Vec3 B, Vec3 C, const Vec3& Outward)
		{
			const Vec3 Normal = (B - A).Cross(C - A);
			if (Normal.Length() < 1e-7) { return; }   // a sliver with no area
			if (Normal.Dot(Outward) < 0.0) { std::swap(B, C); }
			Tri T;
			T.A = A; T.B = B; T.C = C; T.Surface = Surface;
			Mesh.Tris.push_back(T);
		}

		void AddBox(MeshData& Mesh, uint8_t Surface, const Vec3& Center, const Vec3& Size)
		{
			const Vec3 Half = Size * 0.5;
			for (int Axis = 0; Axis < 3; ++Axis)
			{
				const Vec3 Normal = AxisVector(Axis);
				const Vec3 U = AxisVector((Axis + 1) % 3) * Half[(Axis + 1) % 3];
				const Vec3 V = AxisVector((Axis + 2) % 3) * Half[(Axis + 2) % 3];
				for (int Facing = -1; Facing <= 1; Facing += 2)
				{
					const Vec3 Out = Normal * (double)Facing;
					const Vec3 Face = Center + Out * Half[Axis];
					AddTriangle(Mesh, Surface, Face - U - V, Face + U - V, Face + U + V, Out);
					AddTriangle(Mesh, Surface, Face - U - V, Face + U + V, Face - U + V, Out);
				}
			}
		}

		void AddBoxPlaced(MeshData& Mesh, uint8_t Surface, const Xform& Placement, const Vec3& Size)
		{
			const Vec3 Half = Size * 0.5;
			for (int Axis = 0; Axis < 3; ++Axis)
			{
				const Vec3 Normal = AxisVector(Axis);
				const Vec3 U = AxisVector((Axis + 1) % 3) * Half[(Axis + 1) % 3];
				const Vec3 V = AxisVector((Axis + 2) % 3) * Half[(Axis + 2) % 3];
				for (int Facing = -1; Facing <= 1; Facing += 2)
				{
					const Vec3 Face = Normal * ((double)Facing * Half[Axis]);
					const Vec3 Outward = Placement.B * (Normal * (double)Facing);
					AddTriangle(Mesh, Surface, Placement.Apply(Face - U - V), Placement.Apply(Face + U - V),
						Placement.Apply(Face + U + V), Outward);
					AddTriangle(Mesh, Surface, Placement.Apply(Face - U - V), Placement.Apply(Face + U + V),
						Placement.Apply(Face - U + V), Outward);
				}
			}
		}

		void AddCylinder(MeshData& Mesh, uint8_t Surface, const Vec3& Center, int Axis, double Radius, double Length, int Segments = 16)
		{
			const Vec3 Along = AxisVector(Axis) * (Length * 0.5);
			const Vec3 U = AxisVector((Axis + 1) % 3);
			const Vec3 V = AxisVector((Axis + 2) % 3);
			for (int Segment = 0; Segment < Segments; ++Segment)
			{
				const double AngleA = (double)Segment / Segments * kTau;
				const double AngleB = (double)(Segment + 1) / Segments * kTau;
				const Vec3 RimA = (U * std::cos(AngleA) + V * std::sin(AngleA)) * Radius;
				const Vec3 RimB = (U * std::cos(AngleB) + V * std::sin(AngleB)) * Radius;
				const Vec3 Outward = RimA + RimB;
				AddTriangle(Mesh, Surface, Center + RimA - Along, Center + RimA + Along, Center + RimB + Along, Outward);
				AddTriangle(Mesh, Surface, Center + RimA - Along, Center + RimB + Along, Center + RimB - Along, Outward);
				AddTriangle(Mesh, Surface, Center + Along, Center + RimA + Along, Center + RimB + Along, Along);
				AddTriangle(Mesh, Surface, Center - Along, Center + RimA - Along, Center + RimB - Along, -Along);
			}
		}

		void AddFrustum(MeshData& Mesh, uint8_t Surface, const Vec3& Center, int Axis, double RadiusLow, double RadiusHigh,
			double Length, int Segments = 16)
		{
			const Vec3 Along = AxisVector(Axis) * (Length * 0.5);
			const Vec3 U = AxisVector((Axis + 1) % 3);
			const Vec3 V = AxisVector((Axis + 2) % 3);
			for (int Segment = 0; Segment < Segments; ++Segment)
			{
				const double AngleA = (double)Segment / Segments * kTau;
				const double AngleB = (double)(Segment + 1) / Segments * kTau;
				const Vec3 SpokeA = U * std::cos(AngleA) + V * std::sin(AngleA);
				const Vec3 SpokeB = U * std::cos(AngleB) + V * std::sin(AngleB);
				const Vec3 LowA = Center - Along + SpokeA * RadiusLow;
				const Vec3 LowB = Center - Along + SpokeB * RadiusLow;
				const Vec3 HighA = Center + Along + SpokeA * RadiusHigh;
				const Vec3 HighB = Center + Along + SpokeB * RadiusHigh;
				const Vec3 Outward = SpokeA + SpokeB;
				AddTriangle(Mesh, Surface, LowA, HighA, HighB, Outward);
				AddTriangle(Mesh, Surface, LowA, HighB, LowB, Outward);
				AddTriangle(Mesh, Surface, Center + Along, HighA, HighB, Along);
				AddTriangle(Mesh, Surface, Center - Along, LowA, LowB, -Along);
			}
		}

		void AddExtrudedPolygon(MeshData& Mesh, uint8_t Surface, const std::vector<Vec3>& Points, const Vec3& Offset)
		{
			const size_t Count = Points.size();
			Vec3 Middle;
			for (const Vec3& Corner : Points) { Middle += Corner; }
			Middle = Middle / (double)Count;
			for (size_t Index = 1; Index + 1 < Count; ++Index)
			{
				const Vec3& A = Points[0];
				const Vec3& B = Points[Index];
				const Vec3& C = Points[Index + 1];
				AddTriangle(Mesh, Surface, A, B, C, -Offset);
				AddTriangle(Mesh, Surface, A + Offset, B + Offset, C + Offset, Offset);
			}
			for (size_t Index = 0; Index < Count; ++Index)
			{
				const Vec3& P = Points[Index];
				const Vec3& Q = Points[(Index + 1) % Count];
				const Vec3 Outward = (P + Q) * 0.5 - Middle;
				AddTriangle(Mesh, Surface, P, P + Offset, Q + Offset, Outward);
				AddTriangle(Mesh, Surface, P, Q + Offset, Q, Outward);
			}
		}

		Vec3 SpherePoint(const Vec3& Center, double Radius, double Polar, double Azimuth)
		{
			return Center + Vec3(std::sin(Polar) * std::cos(Azimuth), std::cos(Polar), std::sin(Polar) * std::sin(Azimuth)) * Radius;
		}

		void AddSphere(MeshData& Mesh, uint8_t Surface, const Vec3& Center, double Radius, int Segments = 12, int Rings = 6)
		{
			for (int Ring = 0; Ring < Rings; ++Ring)
			{
				const double PolarA = (double)Ring / Rings * kPi;
				const double PolarB = (double)(Ring + 1) / Rings * kPi;
				for (int Segment = 0; Segment < Segments; ++Segment)
				{
					const double AzA = (double)Segment / Segments * kTau;
					const double AzB = (double)(Segment + 1) / Segments * kTau;
					const Vec3 PAA = SpherePoint(Center, Radius, PolarA, AzA);
					const Vec3 PAB = SpherePoint(Center, Radius, PolarA, AzB);
					const Vec3 PBA = SpherePoint(Center, Radius, PolarB, AzA);
					const Vec3 PBB = SpherePoint(Center, Radius, PolarB, AzB);
					const Vec3 Outward = (PAA + PAB + PBA + PBB) * 0.25 - Center;
					AddTriangle(Mesh, Surface, PAA, PAB, PBB, Outward);
					AddTriangle(Mesh, Surface, PAA, PBB, PBA, Outward);
				}
			}
		}

		/** A toothed disc. Teeth are for looks only; meshing comes from tooth counts. */
		void AddGear(MeshData& Mesh, uint8_t Surface, const Vec3& Center, int Axis, double RootRadius, double TipRadius,
			double Thickness, int Teeth)
		{
			static const double ToothSteps[4] = {0.0, 0.25, 0.5, 0.75};
			static const bool ToothAtTip[4] = {false, true, true, false};
			const Vec3 AxisUnit = AxisVector(Axis);
			const Vec3 Along = AxisUnit * (Thickness * 0.5);
			const Vec3 U = AxisVector((Axis + 1) % 3);
			const Vec3 V = AxisVector((Axis + 2) % 3);
			const double Pitch = kTau / (double)Teeth;
			std::vector<Vec3> Rim;
			for (int Tooth = 0; Tooth < Teeth; ++Tooth)
			{
				for (int Corner = 0; Corner < 4; ++Corner)
				{
					const double Angle = ((double)Tooth + ToothSteps[Corner]) * Pitch;
					const double Radius = ToothAtTip[Corner] ? TipRadius : RootRadius;
					Rim.push_back((U * std::cos(Angle) + V * std::sin(Angle)) * Radius);
				}
			}
			const size_t Count = Rim.size();
			for (size_t Index = 0; Index < Count; ++Index)
			{
				const Vec3& P = Rim[Index];
				const Vec3& Q = Rim[(Index + 1) % Count];
				const Vec3 Outward = (Q - P).Cross(AxisUnit);
				AddTriangle(Mesh, Surface, Center + P - Along, Center + P + Along, Center + Q + Along, Outward);
				AddTriangle(Mesh, Surface, Center + P - Along, Center + Q + Along, Center + Q - Along, Outward);
				AddTriangle(Mesh, Surface, Center + Along, Center + P + Along, Center + Q + Along, Along);
				AddTriangle(Mesh, Surface, Center - Along, Center + P - Along, Center + Q - Along, -Along);
			}
		}

		/** +1 when a flat outline runs anticlockwise, -1 when clockwise. */
		double Orientation(const Outline2& Outline)
		{
			double Doubled = 0.0;
			const size_t Count = Outline.size();
			for (size_t Index = 0; Index < Count; ++Index)
			{
				const auto& P = Outline[Index];
				const auto& Q = Outline[(Index + 1) % Count];
				Doubled += P.first * Q.second - Q.first * P.second;
			}
			return Doubled >= 0.0 ? 1.0 : -1.0;
		}

		/** A closed convex solid from a flat list of triangle corners. */
		MeshData FromTriangles(const std::vector<Vec3>& Points)
		{
			Vec3 Middle;
			for (const Vec3& Point : Points) { Middle += Point; }
			Middle = Middle / (double)Points.size();
			MeshData Mesh;
			for (size_t Index = 0; Index + 2 < Points.size(); Index += 3)
			{
				const Vec3& A = Points[Index];
				const Vec3& B = Points[Index + 1];
				const Vec3& C = Points[Index + 2];
				AddTriangle(Mesh, 0, A, B, C, (A + B + C) / 3.0 - Middle);
			}
			return Mesh;
		}

		/** Pushes a flat outline in the XY plane through the box along Z. */
		MeshData Extrude(const std::string& Type, const Outline2& Outline)
		{
			Recipe R;
			R.Kind = 1;
			R.Outline = Outline;
			Recipes()[Type] = R;
			MeshData Mesh;
			const double Turn = Orientation(Outline);
			const size_t Count = Outline.size();
			for (size_t Index = 0; Index < Count; ++Index)
			{
				const auto& P = Outline[Index];
				const auto& Q = Outline[(Index + 1) % Count];
				const double EdgeX = Q.first - P.first;
				const double EdgeY = Q.second - P.second;
				const Vec3 SideOut = Vec3(EdgeY, -EdgeX, 0.0) * Turn;
				const Vec3 PBack(P.first, P.second, -0.5), PFront(P.first, P.second, 0.5);
				const Vec3 QBack(Q.first, Q.second, -0.5), QFront(Q.first, Q.second, 0.5);
				AddTriangle(Mesh, 0, PBack, PFront, QFront, SideOut);
				AddTriangle(Mesh, 0, PBack, QFront, QBack, SideOut);
			}
			const std::vector<int> Triangles = Geometry::TriangulatePolygon(Outline);
			for (size_t Index = 0; Index + 2 < Triangles.size(); Index += 3)
			{
				const auto& A = Outline[(size_t)Triangles[Index]];
				const auto& B = Outline[(size_t)Triangles[Index + 1]];
				const auto& C = Outline[(size_t)Triangles[Index + 2]];
				AddTriangle(Mesh, 0, Vec3(A.first, A.second, 0.5), Vec3(B.first, B.second, 0.5), Vec3(C.first, C.second, 0.5), Vec3(0, 0, 1));
				AddTriangle(Mesh, 0, Vec3(A.first, A.second, -0.5), Vec3(B.first, B.second, -0.5), Vec3(C.first, C.second, -0.5), Vec3(0, 0, -1));
			}
			return Mesh;
		}

		/** Spins a closed outline of (radius, height) points around the Y axis. */
		MeshData Lathe(const std::string& Type, const Outline2& Outline, int Segments)
		{
			Recipe R;
			R.Kind = 2;
			R.Outline = Outline;
			R.Segments = Segments;
			Recipes()[Type] = R;
			MeshData Mesh;
			const double Turn = Orientation(Outline);
			const size_t Count = Outline.size();
			for (size_t Index = 0; Index < Count; ++Index)
			{
				const auto& P = Outline[Index];
				const auto& Q = Outline[(Index + 1) % Count];
				if (P.first < 0.00001 && Q.first < 0.00001) { continue; }   // this edge lies on the axis
				const double EdgeX = Q.first - P.first;
				const double EdgeY = Q.second - P.second;
				const double OutRadial = EdgeY * Turn;
				const double OutHeight = -EdgeX * Turn;
				for (int Segment = 0; Segment < Segments; ++Segment)
				{
					const double AngleA = (double)Segment / Segments * kTau;
					const double AngleB = (double)(Segment + 1) / Segments * kTau;
					const double AngleMid = (AngleA + AngleB) * 0.5;
					const Vec3 Outward(OutRadial * std::cos(AngleMid), OutHeight, OutRadial * std::sin(AngleMid));
					const Vec3 PA(P.first * std::cos(AngleA), P.second, P.first * std::sin(AngleA));
					const Vec3 PB(P.first * std::cos(AngleB), P.second, P.first * std::sin(AngleB));
					const Vec3 QA(Q.first * std::cos(AngleA), Q.second, Q.first * std::sin(AngleA));
					const Vec3 QB(Q.first * std::cos(AngleB), Q.second, Q.first * std::sin(AngleB));
					AddTriangle(Mesh, 0, PA, QA, QB, Outward);
					AddTriangle(Mesh, 0, PA, QB, PB, Outward);
				}
			}
			return Mesh;
		}

		MeshData BuildShape(const std::string& Type)
		{
			const int Steps = Parts::CurveSteps;
			if (Type == "wedge")
			{
				return Extrude(Type, {{-0.5, -0.5}, {0.5, -0.5}, {-0.5, 0.5}});
			}
			if (Type == "triangle_wedge")
			{
				return Extrude(Type, {{-0.5, -0.5}, {0.5, -0.5}, {0.0, 0.5}});
			}
			if (Type == "cylinder" || Type == "rod")
			{
				return Lathe(Type, {{0.0, -0.5}, {0.5, -0.5}, {0.5, 0.5}, {0.0, 0.5}}, 32);
			}
			if (Type == "cone")
			{
				return Lathe(Type, {{0.0, -0.5}, {0.5, -0.5}, {0.0, 0.5}}, 32);
			}
			if (Type == "frustum")
			{
				return Lathe(Type, {{0.0, -0.5}, {0.5, -0.5}, {0.25, 0.5}, {0.0, 0.5}}, 32);
			}
			if (Type == "sphere")
			{
				Outline2 Arc;
				for (int Step = 0; Step <= 12; ++Step)
				{
					const double Angle = (double)Step / 12.0 * kPi;
					Arc.push_back({0.5 * std::sin(Angle), -0.5 * std::cos(Angle)});
				}
				return Lathe(Type, Arc, 24);
			}
			if (Type == "hemisphere")
			{
				Outline2 Dome = {{0.0, -0.5}};
				for (int Step = 0; Step <= Steps; ++Step)
				{
					const double Angle = (double)Step / Steps * kPi * 0.5;
					Dome.push_back({0.5 * std::cos(Angle), -0.5 + std::sin(Angle)});
				}
				return Lathe(Type, Dome, 24);
			}
			if (Type == "ring")
			{
				const double In = Parts::RingInnerRadius;
				return Lathe(Type, {{In, -0.5}, {0.5, -0.5}, {0.5, 0.5}, {In, 0.5}}, 32);
			}
			if (Type == "pipe")
			{
				const double In = Parts::PipeInnerRadius;
				return Lathe(Type, {{In, -0.5}, {0.5, -0.5}, {0.5, 0.5}, {In, 0.5}}, 32);
			}
			if (Type == "torus")
			{
				// An oval outline that becomes a true circle at the default 1 x 0.25 x 1 size.
				Outline2 Tube;
				for (int Step = 0; Step < 16; ++Step)
				{
					const double Angle = (double)Step / 16.0 * kTau;
					Tube.push_back({0.375 + 0.125 * std::cos(Angle), 0.5 * std::sin(Angle)});
				}
				return Lathe(Type, Tube, 32);
			}
			if (Type == "convex_curve")
			{
				// A quarter disc: the round is centred on the (-X, -Y) corner.
				Outline2 Quarter = {{-0.5, -0.5}};
				for (int Step = 0; Step <= Steps; ++Step)
				{
					const double Angle = (double)Step / Steps * kPi * 0.5;
					Quarter.push_back({-0.5 + std::cos(Angle), -0.5 + std::sin(Angle)});
				}
				return Extrude(Type, Quarter);
			}
			if (Type == "concave_curve")
			{
				// A block with a quarter disc scooped out of its (+X, +Y) corner.
				Outline2 Fillet = {{-0.5, -0.5}};
				for (int Step = 0; Step <= Steps; ++Step)
				{
					const double Angle = (double)Step / Steps * kPi * 0.5;
					Fillet.push_back({0.5 - std::sin(Angle), 0.5 - std::cos(Angle)});
				}
				return Extrude(Type, Fillet);
			}
			if (Type == "half_cylinder")
			{
				Outline2 HalfDisc;
				for (int Step = 0; Step <= Steps * 2; ++Step)
				{
					const double Angle = (double)Step / (Steps * 2) * kPi;
					HalfDisc.push_back({0.5 * std::cos(Angle), -0.5 + std::sin(Angle)});
				}
				return Extrude(Type, HalfDisc);
			}
			if (Type == "arch")
			{
				const double R = Parts::ArchRadius;
				Outline2 Arch = {{-0.5, -0.5}, {-R, -0.5}};
				for (int Step = 1; Step < Steps * 2; ++Step)
				{
					const double Angle = kPi - (double)Step / (Steps * 2) * kPi;
					Arch.push_back({R * std::cos(Angle), -0.5 + R * std::sin(Angle)});
				}
				Arch.push_back({R, -0.5});
				Arch.push_back({0.5, -0.5});
				Arch.push_back({0.5, 0.5});
				Arch.push_back({-0.5, 0.5});
				return Extrude(Type, Arch);
			}
			if (Type == "inverse_corner")
			{
				const Vec3 C000(-0.5, -0.5, -0.5), C100(0.5, -0.5, -0.5), C010(-0.5, 0.5, -0.5), C001(-0.5, -0.5, 0.5);
				const Vec3 C110(0.5, 0.5, -0.5), C101(0.5, -0.5, 0.5), C011(-0.5, 0.5, 0.5);
				return FromTriangles({
					C000, C010, C011, C000, C011, C001,
					C000, C100, C101, C000, C101, C001,
					C000, C100, C110, C000, C110, C010,
					C100, C110, C101, C010, C110, C011, C001, C101, C011,
					C110, C101, C011,
				});
			}
			if (Type == "corner_wedge")
			{
				const Vec3 A(-0.5, -0.5, -0.5), B(0.5, -0.5, -0.5), C(-0.5, 0.5, -0.5), D(-0.5, -0.5, 0.5);
				return FromTriangles({A, C, B, A, B, D, A, D, C, B, C, D});
			}
			if (Type == "pyramid")
			{
				const Vec3 P0(-0.5, -0.5, -0.5), P1(0.5, -0.5, -0.5), P2(0.5, -0.5, 0.5), P3(-0.5, -0.5, 0.5);
				const Vec3 Apex(0.0, 0.5, 0.0);
				return FromTriangles({P0, P1, P2, P0, P2, P3, P0, Apex, P1, P1, Apex, P2, P2, Apex, P3, P3, Apex, P0});
			}
			MeshData Box;
			AddBox(Box, 0, Vec3(), Vec3(1, 1, 1));
			return Box;
		}

		const std::set<std::string>& MechIds()
		{
			static const std::set<std::string> Ids = {
				"motor", "servo", "door_hinge_servo", "hinge", "ball_joint", "bearing",
				"piston", "solenoid", "spring", "gear", "bevel_gear", "rack", "pulley",
				"crankshaft", "wheel", "linkage_bar", "clutch", "brake",
				"light_panel", "impact_detector", "explosive_block", "machine_gun", "hitscan_gun", "large_cannon", "ammo_generator",
				"fluid_cylinder", "pump", "valve", "fluid_pipe", "fuel_tank", "pressure_tank",
				"combustion_cylinder", "compressor", "combustor", "turbine", "nozzle", "thruster", "solid_rocket",
				"aero_panel", "aero_panel_triangle", "aero_panel_dragless", "aero_panel_triangle_dragless",
				"propeller", "lift_gas_tank",
				"seat", "anchor_block", "dynamic_offsetter",
				"input_block", "mouse_click", "boolean_gate", "number_gate", "pid_controller", "pwm_generator",
				"speed_sensor", "altitude_sensor", "attitude_sensor", "distance_sensor",
				"graph_display", "instrument", "magnet", "winch", "coupler",
			};
			return Ids;
		}

		void Tint(MeshData& Mesh, Color C)
		{
			Mesh.DetailColor = C;
			Mesh.DetailMetallic = 0.0f;
			Mesh.DetailRoughness = 0.5f;
		}

		void Lens(MeshData& Mesh)
		{
			Mesh.DetailColor = Color(0.02f, 0.02f, 0.03f);
			Mesh.DetailMetallic = 0.0f;
			Mesh.DetailRoughness = 0.12f;
			Mesh.DetailIsLens = true;
		}

		/** The models of every part that is more than a plain shape (mech_models.gd). */
		MeshData BuildMech(const std::string& Type, const MeshOptions& Options)
		{
			const int X = 0, Y = 1, Z = 2;
			const uint8_t BodyS = 0, DetailS = 1;
			const std::string& Half = Options.Half;
			const int Teeth = ClampInt(Options.Teeth, 4, 120);
			MeshData M;
			bool HasDetail = true;
			const Vec3 Zero;

			if (Type == "motor")
			{
				if (Half != "top")
				{
					AddBox(M, BodyS, Vec3(0, -0.1875, 0), Vec3(0.875, 0.625, 0.875));
					AddCylinder(M, DetailS, Vec3(0, 0.1875, 0), Y, 0.28, 0.125);
				}
				const uint8_t TopS = Half == "top" ? BodyS : DetailS;
				if (Half != "base")
				{
					AddCylinder(M, TopS, Vec3(0, 0.34375, 0), Y, 0.1, 0.1875);
					AddBox(M, TopS, Vec3(0, 0.46875, 0), Vec3(0.65, 0.0625, 0.4));
				}
				if (Half == "top") { HasDetail = false; }
			}
			else if (Type == "servo")
			{
				if (Half != "top") { AddBox(M, BodyS, Vec3(0, -0.1875, 0), Vec3(0.875, 0.625, 0.625)); }
				const uint8_t TopS = Half == "top" ? BodyS : DetailS;
				if (Half != "base")
				{
					AddCylinder(M, TopS, Vec3(0, 0.28125, 0), Y, 0.2, 0.3125);
					AddBox(M, TopS, Vec3(0, 0.46875, 0), Vec3(0.15, 0.0625, 0.65));
				}
				if (!Half.empty()) { HasDetail = false; }
			}
			else if (Type == "door_hinge_servo")
			{
				if (Half != "top")
				{
					AddBox(M, BodyS, Vec3(0, -0.4375, 0), Vec3(1.0, 0.125, 1.0));
					AddCylinder(M, DetailS, Vec3(0, -0.3125, -0.3125), X, 0.125, 0.9375);
					AddBox(M, DetailS, Vec3(0.3125, -0.25, 0), Vec3(0.25, 0.25, 0.375));
				}
				if (Half != "base") { AddBox(M, BodyS, Vec3(0, 0.0625, -0.4375), Vec3(1.0, 0.875, 0.125)); }
				if (Half == "top") { HasDetail = false; }
			}
			else if (Type == "hinge")
			{
				if (Half != "top")
				{
					AddBox(M, BodyS, Vec3(0, -0.3125, 0), Vec3(1.0, 0.375, 1.0));
					AddCylinder(M, DetailS, Zero, X, 0.125, 0.9375);
				}
				if (Half != "base") { AddBox(M, BodyS, Vec3(0, 0.3125, 0), Vec3(1.0, 0.375, 1.0)); }
				if (Half == "top") { HasDetail = false; }
			}
			else if (Type == "ball_joint")
			{
				if (Half != "top")
				{
					AddBox(M, BodyS, Vec3(0, -0.3125, 0), Vec3(1.0, 0.375, 1.0));
					AddSphere(M, DetailS, Zero, 0.27, 16, 8);
				}
				if (Half != "base") { AddBox(M, BodyS, Vec3(0, 0.3125, 0), Vec3(1.0, 0.375, 1.0)); }
				if (Half == "top") { HasDetail = false; }
			}
			else if (Type == "bearing")
			{
				if (Half != "top")
				{
					AddBox(M, BodyS, Zero, Vec3(1.0, 0.875, 1.0));
					AddCylinder(M, DetailS, Zero, Y, 0.32, 0.9375);
				}
				const uint8_t ShaftS = Half == "top" ? BodyS : DetailS;
				if (Half != "base")
				{
					AddCylinder(M, ShaftS, Zero, Y, 0.14, 0.96);
					AddBox(M, ShaftS, Vec3(0, 0.485, 0), Vec3(0.5, 0.03, 0.16));
					AddBox(M, ShaftS, Vec3(0, -0.485, 0), Vec3(0.5, 0.03, 0.16));
				}
				if (Half == "top") { HasDetail = false; }
			}
			else if (Type == "piston")
			{
				const double Reach = Options.Reach;
				if (Half != "top")
				{
					AddBox(M, BodyS, Vec3(0, -0.4375, 0), Vec3(0.875, 0.125, 0.875));
					AddCylinder(M, BodyS, Vec3(0, -0.0625, 0), Y, 0.34, 0.625);
				}
				const uint8_t RodS = Half == "top" ? BodyS : DetailS;
				if (Half != "base")
				{
					AddCylinder(M, RodS, Vec3(0, 0.34375 - Reach * 0.5, 0), Y, 0.11, 0.1875 + Reach);
					AddBox(M, RodS, Vec3(0, 0.46875, 0), Vec3(0.4, 0.0625, 0.4));
				}
				if (!Half.empty()) { HasDetail = false; }
			}
			else if (Type == "solenoid")
			{
				const double Reach = Options.Reach;
				if (Half != "top")
				{
					AddBox(M, BodyS, Vec3(0, -0.1875, 0), Vec3(0.625, 0.625, 0.625));
					AddCylinder(M, DetailS, Vec3(0, -0.1875, 0), Y, 0.38, 0.4375);
				}
				const uint8_t PlungerS = Half == "top" ? BodyS : DetailS;
				if (Half != "base") { AddCylinder(M, PlungerS, Vec3(0, 0.3125 - Reach * 0.5, 0), Y, 0.1, 0.375 + Reach); }
				if (Half == "top")
				{
					AddBox(M, PlungerS, Vec3(0, 0.47, 0), Vec3(0.35, 0.06, 0.35));
					HasDetail = false;
				}
			}
			else if (Type == "spring")
			{
				if (Half.empty() || Half == "base") { AddBox(M, BodyS, Vec3(0, -0.46875, 0), Vec3(0.75, 0.0625, 0.75)); }
				if (Half.empty() || Half == "top") { AddBox(M, BodyS, Vec3(0, 0.46875, 0), Vec3(0.75, 0.0625, 0.75)); }
				if (Half.empty() || Half == "coils")
				{
					const uint8_t CoilS = Half == "coils" ? BodyS : DetailS;
					for (int Coil = 0; Coil < 6; ++Coil)
					{
						AddCylinder(M, CoilS, Vec3(0, -0.3125 + 0.125 * Coil, 0), Y, 0.3, 0.0625);
					}
					AddCylinder(M, CoilS, Zero, Y, 0.07, 0.875);
				}
				if (!Half.empty()) { HasDetail = false; }
			}
			else if (Type == "gear")
			{
				AddGear(M, BodyS, Zero, Y, 0.4, 0.5, 0.8, Teeth);
				AddCylinder(M, DetailS, Zero, Y, 0.15, 1.0);
			}
			else if (Type == "bevel_gear")
			{
				AddGear(M, BodyS, Vec3(0, -0.3, 0), Y, 0.4, 0.5, 0.3, Teeth);
				AddCylinder(M, BodyS, Vec3(0, -0.05, 0), Y, 0.33, 0.2);
				AddCylinder(M, BodyS, Vec3(0, 0.15, 0), Y, 0.22, 0.2);
				AddCylinder(M, DetailS, Zero, Y, 0.12, 1.0);
			}
			else if (Type == "rack")
			{
				AddBox(M, BodyS, Vec3(0, -0.2, 0), Vec3(1.0, 0.6, 1.0));
				for (int Tooth = 0; Tooth < 5; ++Tooth)
				{
					AddBox(M, DetailS, Vec3(-0.4 + 0.2 * Tooth, 0.3, 0), Vec3(0.1, 0.4, 1.0));
				}
			}
			else if (Type == "pulley")
			{
				AddCylinder(M, BodyS, Vec3(0, -0.375, 0), Y, 0.5, 0.2, 24);
				AddCylinder(M, BodyS, Vec3(0, 0.375, 0), Y, 0.5, 0.2, 24);
				AddCylinder(M, BodyS, Zero, Y, 0.36, 0.55, 24);
				AddCylinder(M, DetailS, Zero, Y, 0.14, 1.0);
			}
			else if (Type == "crankshaft")
			{
				if (Half != "top")
				{
					const uint8_t JournalS = Half == "base" ? BodyS : DetailS;
					AddCylinder(M, JournalS, Vec3(-0.375, 0, 0), X, 0.12, 0.25);
					AddCylinder(M, JournalS, Vec3(0.375, 0, 0), X, 0.12, 0.25);
				}
				if (Half != "base")
				{
					AddBox(M, BodyS, Vec3(-0.1875, 0.0625, 0), Vec3(0.125, 0.75, 0.35));
					AddBox(M, BodyS, Vec3(0.1875, 0.0625, 0), Vec3(0.125, 0.75, 0.35));
					AddCylinder(M, DetailS, Vec3(0, 0.3, 0), X, 0.1, 0.25);
				}
				if (Half == "base") { HasDetail = false; }
			}
			else if (Type == "wheel")
			{
				if (Half == "base")
				{
					AddCylinder(M, BodyS, Zero, X, 0.12, 1.0);
					HasDetail = false;
				}
				else
				{
					AddCylinder(M, BodyS, Zero, X, 0.5, 0.9, 24);
					AddCylinder(M, DetailS, Zero, X, 0.3, 1.0);
					AddBox(M, DetailS, Zero, Vec3(1.02, 0.08, 0.56));
					AddBox(M, DetailS, Zero, Vec3(1.02, 0.56, 0.08));
				}
			}
			else if (Type == "linkage_bar")
			{
				if (Half.empty() || Half == "bar") { AddBox(M, BodyS, Zero, Vec3(0.84, 0.7, 0.7)); }
				if (Half.empty())
				{
					AddBox(M, DetailS, Vec3(-0.46, 0, 0), Vec3(0.08, 1.0, 1.0));
					AddBox(M, DetailS, Vec3(0.46, 0, 0), Vec3(0.08, 1.0, 1.0));
				}
				else if (Half == "end_a") { AddBox(M, BodyS, Vec3(-0.46, 0, 0), Vec3(0.08, 1.0, 1.0)); }
				else if (Half == "end_b") { AddBox(M, BodyS, Vec3(0.46, 0, 0), Vec3(0.08, 1.0, 1.0)); }
				if (!Half.empty()) { HasDetail = false; }
			}
			else if (Type == "clutch")
			{
				if (Half != "top")
				{
					AddCylinder(M, BodyS, Vec3(0, -0.3, 0), Y, 0.45, 0.35, 24);
					AddBox(M, BodyS, Vec3(0.42, -0.3, 0), Vec3(0.14, 0.3, 0.14));
					AddCylinder(M, DetailS, Zero, Y, 0.38, 0.25, 24);
				}
				if (Half != "base")
				{
					AddCylinder(M, BodyS, Vec3(0, 0.3, 0), Y, 0.45, 0.35, 24);
					AddBox(M, BodyS, Vec3(0.42, 0.3, 0), Vec3(0.14, 0.3, 0.14));
				}
				if (Half.empty()) { AddCylinder(M, DetailS, Zero, Y, 0.12, 1.0); }
				if (Half == "top") { HasDetail = false; }
			}
			else if (Type == "brake")
			{
				if (Half != "top") { AddBox(M, BodyS, Vec3(0.3, 0, 0), Vec3(0.4, 0.9, 0.45)); }
				if (Half != "base")
				{
					const uint8_t DiscS = Half == "top" ? BodyS : DetailS;
					AddCylinder(M, DiscS, Zero, Y, 0.5, 0.3, 24);
					AddCylinder(M, DiscS, Zero, Y, 0.14, 1.0);
					AddBox(M, DiscS, Vec3(0, 0.47, 0), Vec3(0.5, 0.06, 0.16));
				}
				if (!Half.empty()) { HasDetail = false; }
			}
			else if (Type == "light_panel")
			{
				AddBox(M, BodyS, Vec3(0, -0.2, 0), Vec3(1.0, 0.6, 1.0));
				AddBox(M, DetailS, Vec3(0, 0.3, 0), Vec3(0.875, 0.4, 0.875));
				Lens(M);
			}
			else if (Type == "impact_detector")
			{
				AddBox(M, BodyS, Vec3(0, -0.25, 0), Vec3(0.75, 0.5, 0.75));
				AddCylinder(M, DetailS, Vec3(0, 0.03125, 0), Y, 0.3, 0.0625);
				AddSphere(M, DetailS, Vec3(0, 0.15, 0), 0.22);
			}
			else if (Type == "explosive_block")
			{
				AddBox(M, BodyS, Zero, Vec3(0.9375, 0.9375, 0.9375));
				AddBox(M, DetailS, Vec3(0, 0.25, 0), Vec3(1.0, 0.125, 1.0));
				AddBox(M, DetailS, Vec3(0, -0.25, 0), Vec3(1.0, 0.125, 1.0));
				AddCylinder(M, DetailS, Vec3(0, 0.484375, 0), Y, 0.15, 0.03125);
			}
			else if (Type == "machine_gun" || Type == "hitscan_gun")
			{
				AddBox(M, BodyS, Vec3(0, 0, 0.2), Vec3(0.45, 0.45, 0.6));
				AddCylinder(M, DetailS, Vec3(0, 0.05, -0.275), Z, 0.07, 0.35);
				AddCylinder(M, DetailS, Vec3(0, 0.05, -0.475), Z, 0.1, 0.05);
				AddBox(M, DetailS, Vec3(0, -0.3625, 0.2), Vec3(0.3, 0.275, 0.3));
				AddBox(M, DetailS, Vec3(0.3125, 0, 0.3), Vec3(0.175, 0.35, 0.25));
			}
			else if (Type == "large_cannon")
			{
				AddBox(M, BodyS, Vec3(0, 0, 0.35), Vec3(0.8, 0.8, 0.3));
				AddCylinder(M, DetailS, Vec3(0, 0, 0.17), Z, 0.24, 0.06, 20);
				AddCylinder(M, DetailS, Vec3(0, 0, -0.145), Z, 0.15, 0.69, 20);
				AddCylinder(M, DetailS, Vec3(0, 0, -0.1), Z, 0.21, 0.12, 20);
				AddCylinder(M, DetailS, Vec3(0, 0, -0.48), Z, 0.18, 0.04, 20);
			}
			else if (Type == "ammo_generator")
			{
				AddBox(M, BodyS, Vec3(0, -0.09, 0), Vec3(0.9375, 0.78, 0.9375));
				AddBox(M, DetailS, Vec3(-0.25, -0.1, 0), Vec3(0.125, 0.8, 1.0));
				AddBox(M, DetailS, Vec3(0.25, -0.1, 0), Vec3(0.125, 0.8, 1.0));
				AddBox(M, DetailS, Vec3(0, 0.35, 0), Vec3(1.0, 0.1, 1.0));
				AddBox(M, DetailS, Vec3(0, 0.45, 0), Vec3(0.3, 0.1, 0.3));
			}
			else if (Type == "fluid_cylinder")
			{
				AddBox(M, BodyS, Vec3(0, -0.46875, 0), Vec3(0.7, 0.0625, 0.7));
				AddCylinder(M, BodyS, Vec3(0, -0.15, 0), Y, 0.3, 0.575);
				AddCylinder(M, DetailS, Vec3(0, 0.2875, 0), Y, 0.1, 0.3);
				AddBox(M, DetailS, Vec3(0, 0.46875, 0), Vec3(0.35, 0.0625, 0.35));
				AddCylinder(M, DetailS, Vec3(0.35, -0.3, 0), X, 0.07, 0.2);
			}
			else if (Type == "pump")
			{
				AddBox(M, BodyS, Vec3(0, -0.1875, 0), Vec3(0.75, 0.625, 0.75));
				AddCylinder(M, DetailS, Vec3(0, 0.25, 0), Y, 0.25, 0.25);
				AddCylinder(M, DetailS, Vec3(0, -0.2, 0), X, 0.1, 1.0);
			}
			else if (Type == "valve")
			{
				AddCylinder(M, DetailS, Zero, X, 0.14, 1.0);
				AddBox(M, BodyS, Zero, Vec3(0.4, 0.4, 0.4));
				AddCylinder(M, DetailS, Vec3(0, 0.3, 0), Y, 0.05, 0.2);
				AddBox(M, BodyS, Vec3(0, 0.43125, 0), Vec3(0.5, 0.0625, 0.12));
			}
			else if (Type == "fluid_pipe")
			{
				AddCylinder(M, BodyS, Zero, X, 0.15, 1.0);
				AddCylinder(M, DetailS, Vec3(-0.44, 0, 0), X, 0.22, 0.08);
				AddCylinder(M, DetailS, Vec3(0.44, 0, 0), X, 0.22, 0.08);
			}
			else if (Type == "fuel_tank")
			{
				AddCylinder(M, BodyS, Zero, X, 0.42, 0.9, 24);
				AddCylinder(M, DetailS, Vec3(-0.25, 0, 0), X, 0.45, 0.06, 24);
				AddCylinder(M, DetailS, Vec3(0.25, 0, 0), X, 0.45, 0.06, 24);
				AddCylinder(M, DetailS, Vec3(0, 0.44, 0), Y, 0.08, 0.1);
				AddBox(M, DetailS, Vec3(-0.3, -0.44, 0), Vec3(0.1, 0.12, 0.7));
				AddBox(M, DetailS, Vec3(0.3, -0.44, 0), Vec3(0.1, 0.12, 0.7));
			}
			else if (Type == "pressure_tank")
			{
				AddCylinder(M, BodyS, Zero, Y, 0.38, 0.24, 20);
				AddSphere(M, BodyS, Vec3(0, 0.12, 0), 0.38, 20, 10);
				AddSphere(M, BodyS, Vec3(0, -0.12, 0), 0.38, 20, 10);
				AddCylinder(M, DetailS, Zero, Y, 0.4, 0.05, 20);
				AddCylinder(M, DetailS, Vec3(0.42, 0, 0), X, 0.06, 0.12);
			}
			else if (Type == "combustion_cylinder")
			{
				AddBox(M, BodyS, Vec3(0, -0.1, 0), Vec3(0.7, 0.5, 0.7));
				for (int Fin = 0; Fin < 3; ++Fin) { AddBox(M, DetailS, Vec3(0, -0.25 + 0.15 * Fin, 0), Vec3(0.8, 0.04, 0.8)); }
				AddBox(M, BodyS, Vec3(0, 0.225, 0), Vec3(0.6, 0.15, 0.6));
				AddCylinder(M, DetailS, Vec3(0, 0.4, 0), Y, 0.05, 0.2);
				AddCylinder(M, DetailS, Vec3(0, -0.425, 0), Y, 0.09, 0.15);
			}
			else if (Type == "compressor")
			{
				AddCylinder(M, BodyS, Zero, Z, 0.45, 0.9, 24);
				AddCylinder(M, DetailS, Zero, Z, 0.36, 0.94, 24);
				AddCylinder(M, DetailS, Zero, Z, 0.12, 1.0);
			}
			else if (Type == "combustor")
			{
				AddCylinder(M, BodyS, Zero, Z, 0.4, 0.8, 24);
				AddCylinder(M, DetailS, Vec3(0, 0, -0.25), Z, 0.43, 0.08, 24);
				AddCylinder(M, DetailS, Vec3(0, 0, 0.25), Z, 0.43, 0.08, 24);
				AddCylinder(M, DetailS, Zero, Y, 0.05, 0.94);
				AddCylinder(M, DetailS, Zero, X, 0.05, 0.94);
			}
			else if (Type == "turbine")
			{
				AddCylinder(M, BodyS, Vec3(0, 0, -0.1), Z, 0.45, 0.6, 24);
				AddGear(M, DetailS, Vec3(0, 0, -0.1), Z, 0.3, 0.4, 0.66, 16);
				AddCylinder(M, DetailS, Zero, Z, 0.1, 1.0);
			}
			else if (Type == "nozzle")
			{
				AddFrustum(M, BodyS, Vec3(0, 0, -0.015), Z, 0.25, 0.45, 0.97, 24);
				AddCylinder(M, DetailS, Vec3(0, 0, 0.48), Z, 0.38, 0.04, 24);
				AddCylinder(M, DetailS, Vec3(0, 0, -0.44), Z, 0.3, 0.06, 24);
			}
			else if (Type == "thruster")
			{
				AddBox(M, BodyS, Vec3(0, 0, -0.35), Vec3(0.6, 0.6, 0.3));
				AddFrustum(M, BodyS, Vec3(0, 0, 0.125), Z, 0.2, 0.42, 0.65, 24);
				AddCylinder(M, DetailS, Vec3(0, 0, 0.475), Z, 0.34, 0.05, 24);
			}
			else if (Type == "solid_rocket")
			{
				AddCylinder(M, BodyS, Vec3(0, 0, -0.05), Z, 0.4, 0.7, 24);
				AddFrustum(M, BodyS, Vec3(0, 0, -0.45), Z, 0.15, 0.4, 0.1, 24);
				AddFrustum(M, DetailS, Vec3(0, 0, 0.4), Z, 0.18, 0.32, 0.2, 24);
				AddCylinder(M, DetailS, Vec3(0, 0, 0.1), Z, 0.42, 0.05, 24);
			}
			else if (Type == "aero_panel" || Type == "aero_panel_dragless")
			{
				AddBox(M, BodyS, Zero, Vec3(1, 1, 1));
				HasDetail = false;
			}
			else if (Type == "aero_panel_triangle" || Type == "aero_panel_triangle_dragless")
			{
				AddExtrudedPolygon(M, BodyS, {Vec3(-0.5, -0.5, 0.5), Vec3(0.5, -0.5, 0.5), Vec3(-0.5, -0.5, -0.5)}, Vec3(0, 1, 0));
				HasDetail = false;
			}
			else if (Type == "propeller")
			{
				const int Blades = ClampInt(Options.Blades, 2, 12);
				const double Pitch = DegToRad(Clamp((double)Options.PitchDeg, 0.0, 90.0));
				// The propeller is a quarter as tall as it is wide at its normal size,
				// so heights here are stretched four times to come out true.
				const Xform TrueHeight(Basis(Vec3(1, 0, 0), Vec3(0, 4, 0), Vec3(0, 0, 1)), Vec3());
				AddCylinder(M, BodyS, Zero, Y, 0.16, 0.6);
				for (int Blade = 0; Blade < Blades; ++Blade)
				{
					const Basis Around = Basis::FromAxisAngle(Vec3(0, 1, 0), kTau * Blade / Blades);
					const Basis Tilt = Basis::FromAxisAngle(Vec3(1, 0, 0), Pitch);
					const Xform Placement = TrueHeight * Xform(Around, Vec3()) * Xform(Tilt, Vec3(0.31, 0, 0));
					AddBoxPlaced(M, BodyS, Placement, Vec3(0.38, 0.03, 0.2));
				}
				AddCylinder(M, DetailS, Zero, Y, 0.12, 1.0);
			}
			else if (Type == "lift_gas_tank")
			{
				AddSphere(M, BodyS, Vec3(0, 0.03, 0), 0.45, 20, 10);
				AddCylinder(M, DetailS, Vec3(0, -0.45, 0), Y, 0.3, 0.1, 20);
				AddCylinder(M, DetailS, Vec3(0, 0.03, 0), Y, 0.46, 0.05, 20);
			}
			else if (Type == "seat")
			{
				AddBox(M, BodyS, Vec3(0, -0.25, 0.05), Vec3(0.8, 0.2, 0.7));
				AddBox(M, BodyS, Vec3(0, 0.175, 0.375), Vec3(0.8, 0.65, 0.15));
				AddBox(M, DetailS, Vec3(0, -0.425, 0.05), Vec3(0.4, 0.15, 0.4));
				AddBox(M, DetailS, Vec3(-0.44, -0.02, 0.1), Vec3(0.08, 0.08, 0.5));
				AddBox(M, DetailS, Vec3(0.44, -0.02, 0.1), Vec3(0.08, 0.08, 0.5));
			}
			else if (Type == "anchor_block")
			{
				AddBox(M, BodyS, Zero, Vec3(0.9375, 0.9375, 0.9375));
				AddBox(M, DetailS, Zero, Vec3(1.0, 0.15, 1.0));
				AddCylinder(M, DetailS, Zero, Y, 0.2, 1.0);
			}
			else if (Type == "dynamic_offsetter")
			{
				AddBox(M, BodyS, Vec3(0, -0.29, 0), Vec3(0.9, 0.42, 0.9));
				AddBox(M, BodyS, Vec3(0, 0.29, 0), Vec3(0.9, 0.42, 0.9));
				AddBox(M, DetailS, Zero, Vec3(0.5, 0.16, 0.5));
				for (int Corner = 0; Corner < 4; ++Corner)
				{
					const double PostX = Corner % 2 == 0 ? 0.35 : -0.35;
					const double PostZ = Corner < 2 ? 0.35 : -0.35;
					AddCylinder(M, DetailS, Vec3(PostX, 0, PostZ), Y, 0.05, 0.16);
				}
			}
			else if (Type == "input_block")
			{
				AddBox(M, BodyS, Vec3(0, -0.2, 0), Vec3(0.9375, 0.6, 0.9375));
				AddBox(M, DetailS, Vec3(0, 0.225, 0), Vec3(0.5, 0.25, 0.5));
				Tint(M, Color(0.95f, 0.55f, 0.15f));
			}
			else if (Type == "mouse_click")
			{
				AddBox(M, BodyS, Vec3(0, -0.2, 0), Vec3(0.9375, 0.6, 0.9375));
				AddBox(M, DetailS, Vec3(0, 0.16, 0.06), Vec3(0.36, 0.12, 0.5));
				AddBox(M, DetailS, Vec3(-0.09, 0.24, -0.12), Vec3(0.16, 0.06, 0.2));
				AddBox(M, DetailS, Vec3(0.09, 0.24, -0.12), Vec3(0.16, 0.06, 0.2));
				Tint(M, Color(0.95f, 0.85f, 0.2f));
			}
			else if (Type == "boolean_gate")
			{
				AddBox(M, BodyS, Vec3(0, -0.25, 0), Vec3(0.9375, 0.5, 0.9375));
				AddBox(M, DetailS, Vec3(0, 0.1, 0), Vec3(0.45, 0.2, 0.6));
				AddBox(M, DetailS, Vec3(-0.36, 0.06, 0.2), Vec3(0.12, 0.12, 0.12));
				AddBox(M, DetailS, Vec3(-0.36, 0.06, -0.2), Vec3(0.12, 0.12, 0.12));
				AddBox(M, DetailS, Vec3(0.36, 0.06, 0), Vec3(0.12, 0.12, 0.12));
				Tint(M, Color(0.25f, 0.75f, 0.35f));
			}
			else if (Type == "number_gate")
			{
				AddBox(M, BodyS, Vec3(0, -0.25, 0), Vec3(0.9375, 0.5, 0.9375));
				AddCylinder(M, DetailS, Vec3(0, 0.1, 0), Y, 0.26, 0.2);
				AddBox(M, DetailS, Vec3(-0.38, 0.06, 0.2), Vec3(0.12, 0.12, 0.12));
				AddBox(M, DetailS, Vec3(-0.38, 0.06, -0.2), Vec3(0.12, 0.12, 0.12));
				AddBox(M, DetailS, Vec3(0.38, 0.06, 0), Vec3(0.12, 0.12, 0.12));
				Tint(M, Color(0.25f, 0.5f, 0.95f));
			}
			else if (Type == "pwm_generator")
			{
				AddBox(M, BodyS, Vec3(0, -0.25, 0), Vec3(0.9375, 0.5, 0.9375));
				for (int Step = 0; Step < 4; ++Step)
				{
					const bool High = Step % 2 == 0;
					AddBox(M, DetailS, Vec3(-0.3 + 0.2 * Step, High ? 0.1 : 0.03, 0), Vec3(0.18, High ? 0.2 : 0.06, 0.5));
				}
				Tint(M, Color(0.95f, 0.35f, 0.65f));
			}
			else if (Type == "pid_controller")
			{
				AddBox(M, BodyS, Vec3(0, -0.25, 0), Vec3(0.9375, 0.5, 0.9375));
				for (int Bar = 0; Bar < 3; ++Bar) { AddBox(M, DetailS, Vec3(-0.28 + 0.28 * Bar, 0.125, 0), Vec3(0.2, 0.25, 0.6)); }
				Tint(M, Color(0.65f, 0.35f, 0.9f));
			}
			else if (Type == "speed_sensor")
			{
				AddBox(M, BodyS, Vec3(0, -0.3, 0), Vec3(0.75, 0.4, 0.75));
				AddBox(M, DetailS, Vec3(0, 0, 0.2), Vec3(0.12, 0.2, 0.12));
				AddCylinder(M, DetailS, Vec3(0, 0.05, -0.05), Z, 0.06, 0.9);
				Tint(M, Color(0.95f, 0.8f, 0.2f));
			}
			else if (Type == "altitude_sensor")
			{
				AddBox(M, BodyS, Vec3(0, -0.3, 0), Vec3(0.75, 0.4, 0.75));
				AddSphere(M, DetailS, Vec3(0, -0.05, 0), 0.25);
				AddCylinder(M, DetailS, Vec3(0, 0.3, 0), Y, 0.04, 0.35);
				Tint(M, Color(0.95f, 0.8f, 0.2f));
			}
			else if (Type == "attitude_sensor")
			{
				AddBox(M, BodyS, Vec3(0, -0.3, 0), Vec3(0.75, 0.4, 0.75));
				AddCylinder(M, DetailS, Vec3(0, 0.2, 0), X, 0.3, 0.08, 20);
				AddCylinder(M, DetailS, Vec3(0, 0.2, 0), Z, 0.3, 0.08, 20);
				Tint(M, Color(0.95f, 0.8f, 0.2f));
			}
			else if (Type == "distance_sensor")
			{
				AddBox(M, BodyS, Vec3(0, -0.25, 0.1), Vec3(0.75, 0.5, 0.6));
				AddCylinder(M, DetailS, Vec3(-0.1, -0.2, -0.3), Z, 0.16, 0.25);
				AddCylinder(M, DetailS, Vec3(0.22, -0.2, -0.28), Z, 0.08, 0.2);
				Tint(M, Color(0.95f, 0.8f, 0.2f));
			}
			else if (Type == "instrument")
			{
				AddBox(M, BodyS, Zero, Vec3(1.0, 1.0, 0.6));
				AddCylinder(M, DetailS, Zero, Z, 0.46, 1.0, 24);
				Lens(M);
			}
			else if (Type == "graph_display")
			{
				AddBox(M, BodyS, Zero, Vec3(1.0, 1.0, 0.6));
				AddBox(M, DetailS, Zero, Vec3(0.92, 0.88, 1.0));
				Lens(M);
			}
			else if (Type == "magnet")
			{
				AddBox(M, BodyS, Vec3(0, -0.15, 0), Vec3(1.0, 0.7, 1.0));
				AddBox(M, DetailS, Vec3(0, 0.35, 0), Vec3(0.94, 0.3, 0.94));
				Tint(M, Color(0.75f, 0.77f, 0.80f));
			}
			else if (Type == "winch")
			{
				AddBox(M, BodyS, Vec3(-0.44, 0, 0), Vec3(0.12, 1.0, 1.0));
				AddBox(M, BodyS, Vec3(0.44, 0, 0), Vec3(0.12, 1.0, 1.0));
				AddBox(M, BodyS, Vec3(0, 0.44, 0), Vec3(1.0, 0.12, 1.0));
				AddCylinder(M, DetailS, Vec3(0, -0.05, 0), X, 0.32, 0.76, 20);
				AddCylinder(M, DetailS, Vec3(0, -0.4, 0), Y, 0.05, 0.2);
			}
			else if (Type == "coupler")
			{
				AddBox(M, BodyS, Vec3(0, 0, 0.3), Vec3(1.0, 1.0, 0.4));
				AddFrustum(M, DetailS, Vec3(0, 0, -0.2), Z, 0.26, 0.42, 0.6, 20);
				Tint(M, Color(0.95f, 0.72f, 0.10f));
			}
			else
			{
				AddBox(M, BodyS, Zero, Vec3(1, 1, 1));
				AddCylinder(M, DetailS, Zero, Y, 0.1, 1.0);
			}
			if (!HasDetail)
			{
				for (Tri& T : M.Tris) { T.Surface = 0; }
			}
			M.HasDetail = false;
			for (const Tri& T : M.Tris)
			{
				if (T.Surface == 1) { M.HasDetail = true; break; }
			}
			return M;
		}

		double Cross2(const std::pair<double, double>& O, const std::pair<double, double>& A, const std::pair<double, double>& B)
		{
			return (A.first - O.first) * (B.second - O.second) - (A.second - O.second) * (B.first - O.first);
		}

		bool PointInTriangle2(const std::pair<double, double>& P, const std::pair<double, double>& A,
			const std::pair<double, double>& B, const std::pair<double, double>& C)
		{
			const double D1 = Cross2(A, B, P);
			const double D2 = Cross2(B, C, P);
			const double D3 = Cross2(C, A, P);
			const bool HasNeg = D1 < -1e-12 || D2 < -1e-12 || D3 < -1e-12;
			const bool HasPos = D1 > 1e-12 || D2 > 1e-12 || D3 > 1e-12;
			return !(HasNeg && HasPos);
		}

		bool IsConvexPolygon(const Outline2& Outline, const std::vector<int>& Poly, double Turn)
		{
			const size_t Count = Poly.size();
			for (size_t Index = 0; Index < Count; ++Index)
			{
				const auto& A = Outline[(size_t)Poly[Index]];
				const auto& B = Outline[(size_t)Poly[(Index + 1) % Count]];
				const auto& C = Outline[(size_t)Poly[(Index + 2) % Count]];
				if (Cross2(A, B, C) * Turn < -1e-9) { return false; }
			}
			return true;
		}
	}

	std::string MeshOptions::Key() const
	{
		return Half + "|" + std::to_string((int)std::lround(Reach * 100.0)) + "|" + std::to_string(Teeth) + "|"
			+ std::to_string(Blades) + "|" + std::to_string(PitchDeg);
	}

	namespace Geometry
	{
		bool HasMechModel(const std::string& Type)
		{
			return GeometryDetail::MechIds().count(Type) > 0;
		}

		const MeshData& UnitMesh(const std::string& Type, const MeshOptions& Options)
		{
			static std::map<std::string, MeshData> Cache;
			const std::string Key = Type + "#" + Options.Key();
			auto Found = Cache.find(Key);
			if (Found != Cache.end()) { return Found->second; }
			MeshData Mesh = HasMechModel(Type) ? GeometryDetail::BuildMech(Type, Options) : GeometryDetail::BuildShape(Type);
			return Cache.emplace(Key, Mesh).first->second;
		}

		const std::vector<Vec3>& UnitHullPoints(const std::string& Type)
		{
			static std::map<std::string, std::vector<Vec3>> Cache;
			auto Found = Cache.find(Type);
			if (Found != Cache.end()) { return Found->second; }
			std::vector<Vec3> Points;
			std::set<IVec3> Seen;
			const PartType& T = Parts::Get(Type);
			if (T.Collision == CollisionKind::Box)
			{
				for (int Corner = 0; Corner < 8; ++Corner)
				{
					Points.push_back(Vec3((Corner & 1) ? 0.5 : -0.5, (Corner & 2) ? 0.5 : -0.5, (Corner & 4) ? 0.5 : -0.5));
				}
			}
			else
			{
				for (const Tri& Triangle : UnitMesh(Type).Tris)
				{
					for (const Vec3* Corner : {&Triangle.A, &Triangle.B, &Triangle.C})
					{
						const IVec3 Key = IVec3::FromRounded(*Corner * 10000.0);
						if (Seen.insert(Key).second) { Points.push_back(*Corner); }
					}
				}
			}
			return Cache.emplace(Type, Points).first->second;
		}

		const std::vector<std::vector<Vec3>>& UnitConvexPieces(const std::string& Type)
		{
			static std::map<std::string, std::vector<std::vector<Vec3>>> Cache;
			auto Found = Cache.find(Type);
			if (Found != Cache.end()) { return Found->second; }
			std::vector<std::vector<Vec3>> Pieces;
			const PartType& T = Parts::Get(Type);
			if (T.Collision == CollisionKind::Concave)
			{
				UnitMesh(Type);   // makes sure the recipe is known
				const auto RecipeFound = GeometryDetail::Recipes().find(Type);
				if (RecipeFound != GeometryDetail::Recipes().end())
				{
					const GeometryDetail::Recipe& R = RecipeFound->second;
					if (R.Kind == 1)
					{
						for (const auto& Polygon : ConvexDecompose(R.Outline))
						{
							std::vector<Vec3> Points;
							for (const auto& Corner : Polygon)
							{
								Points.push_back(Vec3(Corner.first, Corner.second, -0.5));
								Points.push_back(Vec3(Corner.first, Corner.second, 0.5));
							}
							Pieces.push_back(Points);
						}
					}
					else if (R.Kind == 2)
					{
						const int Wedges = std::min(R.Segments, 16);
						for (int Wedge = 0; Wedge < Wedges; ++Wedge)
						{
							std::vector<Vec3> Points;
							for (int Side = 0; Side < 2; ++Side)
							{
								const double Angle = (double)(Wedge + Side) / Wedges * kTau;
								for (const auto& Corner : R.Outline)
								{
									Points.push_back(Vec3(Corner.first * std::cos(Angle), Corner.second, Corner.first * std::sin(Angle)));
								}
							}
							Pieces.push_back(Points);
						}
					}
				}
			}
			if (Pieces.empty()) { Pieces.push_back(UnitHullPoints(Type)); }
			return Cache.emplace(Type, Pieces).first->second;
		}

		bool ContainsUnitPoint(const std::string& Type, const Vec3& P, double Slack)
		{
			const double S = Slack;
			const double Lim = 0.5 + S;
			const bool InBox = std::fabs(P.X) <= Lim && std::fabs(P.Y) <= Lim && std::fabs(P.Z) <= Lim;
			if (!InBox) { return false; }
			const PartType& T = Parts::Get(Type);
			if (T.Collision == CollisionKind::Box) { return true; }
			const double RXZ = std::sqrt(P.X * P.X + P.Z * P.Z);
			if (Type == "wedge") { return (P.X + P.Y) <= S * 1.4142; }
			if (Type == "triangle_wedge")
			{
				const double Root5 = std::sqrt(5.0);
				return (P.Y - 2.0 * P.X - 0.5) / Root5 <= S && (P.Y + 2.0 * P.X - 0.5) / Root5 <= S;
			}
			if (Type == "cylinder" || Type == "rod") { return RXZ <= 0.5 + S; }
			if (Type == "sphere") { return P.Length() <= 0.5 + S; }
			if (Type == "cone") { return RXZ <= 0.25 - 0.5 * P.Y + S; }
			if (Type == "frustum") { return RXZ <= 0.375 - 0.125 * P.Y + S; }
			if (Type == "hemisphere")
			{
				const double E = (P.X * P.X + P.Z * P.Z) / 0.25 + (P.Y + 0.5) * (P.Y + 0.5);
				return std::sqrt(E) <= 1.0 + 2.0 * S;
			}
			if (Type == "half_cylinder")
			{
				const double E = (P.X * P.X) / 0.25 + (P.Y + 0.5) * (P.Y + 0.5);
				return std::sqrt(E) <= 1.0 + 2.0 * S;
			}
			if (Type == "convex_curve")
			{
				return std::sqrt((P.X + 0.5) * (P.X + 0.5) + (P.Y + 0.5) * (P.Y + 0.5)) <= 1.0 + S;
			}
			if (Type == "concave_curve")
			{
				return std::sqrt((P.X - 0.5) * (P.X - 0.5) + (P.Y - 0.5) * (P.Y - 0.5)) >= 1.0 - S;
			}
			if (Type == "arch")
			{
				return std::sqrt(P.X * P.X + (P.Y + 0.5) * (P.Y + 0.5)) >= Parts::ArchRadius - S;
			}
			if (Type == "ring") { return RXZ >= Parts::RingInnerRadius - S && RXZ <= 0.5 + S; }
			if (Type == "pipe") { return RXZ >= Parts::PipeInnerRadius - S && RXZ <= 0.5 + S; }
			if (Type == "torus")
			{
				const double DR = (RXZ - 0.375) / 0.125;
				const double DY = P.Y / 0.5;
				return std::sqrt(DR * DR + DY * DY) <= 1.0 + 4.0 * S;
			}
			if (Type == "corner_wedge") { return (P.X + P.Y + P.Z) / 1.732 <= -0.5 / 1.732 + S; }
			if (Type == "inverse_corner") { return (P.X + P.Y + P.Z) / 1.732 <= 0.5 / 1.732 + S; }
			if (Type == "pyramid")
			{
				const double HalfWidth = 0.25 - 0.5 * P.Y;
				return std::fabs(P.X) <= HalfWidth + S && std::fabs(P.Z) <= HalfWidth + S;
			}
			if (Parts::IsTrianglePanel(Type)) { return (P.X - P.Z) / 1.4142 <= S; }
			if (Type == "wheel") { return std::sqrt(P.Y * P.Y + P.Z * P.Z) <= 0.5 + S; }
			return true;
		}

		double RayTriangle(const Vec3& Origin, const Vec3& Dir, const Vec3& A, const Vec3& B, const Vec3& C)
		{
			const Vec3 E1 = B - A;
			const Vec3 E2 = C - A;
			const Vec3 PVec = Dir.Cross(E2);
			const double Det = E1.Dot(PVec);
			if (std::fabs(Det) < 1e-12) { return -1.0; }
			const double InvDet = 1.0 / Det;
			const Vec3 TVec = Origin - A;
			const double U = TVec.Dot(PVec) * InvDet;
			if (U < 0.0 || U > 1.0) { return -1.0; }
			const Vec3 QVec = TVec.Cross(E1);
			const double V = Dir.Dot(QVec) * InvDet;
			if (V < 0.0 || U + V > 1.0) { return -1.0; }
			const double Distance = E2.Dot(QVec) * InvDet;
			return Distance >= 0.0 ? Distance : -1.0;
		}

		double RayBox(const Vec3& Origin, const Vec3& Dir, const Box& B)
		{
			double TMin = -1e30, TMax = 1e30;
			const Vec3 Lo = B.Position;
			const Vec3 Hi = B.End();
			for (int Axis = 0; Axis < 3; ++Axis)
			{
				if (std::fabs(Dir[Axis]) < 1e-15)
				{
					if (Origin[Axis] < Lo[Axis] || Origin[Axis] > Hi[Axis]) { return -1.0; }
					continue;
				}
				double T1 = (Lo[Axis] - Origin[Axis]) / Dir[Axis];
				double T2 = (Hi[Axis] - Origin[Axis]) / Dir[Axis];
				if (T1 > T2) { std::swap(T1, T2); }
				TMin = std::max(TMin, T1);
				TMax = std::min(TMax, T2);
				if (TMin > TMax) { return -1.0; }
			}
			if (TMax < 0.0) { return -1.0; }
			return TMin >= 0.0 ? TMin : 0.0;
		}

		std::vector<int> TriangulatePolygon(const std::vector<std::pair<double, double>>& Outline)
		{
			std::vector<int> Result;
			const int Count = (int)Outline.size();
			if (Count < 3) { return Result; }
			const double Turn = GeometryDetail::Orientation(Outline);
			std::vector<int> Left;
			for (int Index = 0; Index < Count; ++Index) { Left.push_back(Index); }
			int Guard = Count * Count + 10;
			while (Left.size() > 3 && Guard-- > 0)
			{
				bool Clipped = false;
				const int Size = (int)Left.size();
				for (int Index = 0; Index < Size; ++Index)
				{
					const int Prev = Left[(size_t)((Index + Size - 1) % Size)];
					const int Cur = Left[(size_t)Index];
					const int Next = Left[(size_t)((Index + 1) % Size)];
					const auto& A = Outline[(size_t)Prev];
					const auto& B = Outline[(size_t)Cur];
					const auto& C = Outline[(size_t)Next];
					if (GeometryDetail::Cross2(A, B, C) * Turn <= 1e-12) { continue; }   // a reflex or flat corner
					bool Empty = true;
					for (int Other : Left)
					{
						if (Other == Prev || Other == Cur || Other == Next) { continue; }
						if (GeometryDetail::PointInTriangle2(Outline[(size_t)Other], A, B, C)) { Empty = false; break; }
					}
					if (!Empty) { continue; }
					Result.push_back(Prev);
					Result.push_back(Cur);
					Result.push_back(Next);
					Left.erase(Left.begin() + Index);
					Clipped = true;
					break;
				}
				if (!Clipped)
				{
					// Only flat corners left: drop one and go on.
					Left.erase(Left.begin());
				}
			}
			if (Left.size() == 3)
			{
				Result.push_back(Left[0]);
				Result.push_back(Left[1]);
				Result.push_back(Left[2]);
			}
			return Result;
		}

		std::vector<std::vector<std::pair<double, double>>> ConvexDecompose(const std::vector<std::pair<double, double>>& Outline)
		{
			const double Turn = GeometryDetail::Orientation(Outline);
			const std::vector<int> Triangles = TriangulatePolygon(Outline);
			std::vector<std::vector<int>> Polys;
			for (size_t Index = 0; Index + 2 < Triangles.size(); Index += 3)
			{
				Polys.push_back({Triangles[Index], Triangles[Index + 1], Triangles[Index + 2]});
			}
			// Hertel-Mehlhorn: join neighbours across a shared edge while the result stays convex.
			bool Merged = true;
			while (Merged)
			{
				Merged = false;
				for (size_t First = 0; !Merged && First < Polys.size(); ++First)
				{
					for (size_t Second = First + 1; !Merged && Second < Polys.size(); ++Second)
					{
						const std::vector<int>& A = Polys[First];
						const std::vector<int>& B = Polys[Second];
						for (size_t EdgeA = 0; !Merged && EdgeA < A.size(); ++EdgeA)
						{
							const int U = A[EdgeA];
							const int V = A[(EdgeA + 1) % A.size()];
							for (size_t EdgeB = 0; EdgeB < B.size(); ++EdgeB)
							{
								if (B[EdgeB] != V || B[(EdgeB + 1) % B.size()] != U) { continue; }
								// Walk A from V round to U, then B from U round to V.
								std::vector<int> Joined;
								for (size_t Step = 0; Step < A.size(); ++Step) { Joined.push_back(A[(EdgeA + 1 + Step) % A.size()]); }
								for (size_t Step = 2; Step < B.size(); ++Step) { Joined.push_back(B[(EdgeB + Step) % B.size()]); }
								if (GeometryDetail::IsConvexPolygon(Outline, Joined, Turn))
								{
									Polys[First] = Joined;
									Polys.erase(Polys.begin() + (long)Second);
									Merged = true;
								}
								break;
							}
						}
					}
				}
			}
			std::vector<std::vector<std::pair<double, double>>> Out;
			for (const std::vector<int>& Poly : Polys)
			{
				std::vector<std::pair<double, double>> Points;
				for (int Index : Poly) { Points.push_back(Outline[(size_t)Index]); }
				Out.push_back(Points);
			}
			return Out;
		}
	}
}
