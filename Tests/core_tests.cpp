// Unit tests for the engine-free core (Source/IdealWorld/Core).
// Build and run with Tests/run_tests.sh; no Unreal needed.

#include "IWAero.h"
#include "IWBuildModel.h"
#include "IWGeometry.h"
#include "IWJson.h"
#include "IWLogicCore.h"
#include "IWMaterials.h"
#include "IWParts.h"
#include "IWSaveFormat.h"
#include "IWSpawnPlan.h"

#include <cstdio>
#include <fstream>
#include <sstream>

using namespace IW;

static int Failures = 0;
static int Checks = 0;

#define EXPECT(Cond, ...)                                                   \
	do                                                                      \
	{                                                                       \
		++Checks;                                                           \
		if (!(Cond))                                                        \
		{                                                                   \
			++Failures;                                                     \
			std::printf("FAIL %s:%d: %s  ", __FILE__, __LINE__, #Cond);     \
			std::printf(__VA_ARGS__);                                       \
			std::printf("\n");                                              \
		}                                                                   \
	} while (0)

static std::string ReadFile(const std::string& Path)
{
	std::ifstream In(Path, std::ios::binary);
	std::stringstream S;
	S << In.rdbuf();
	return S.str();
}

static void TestJson()
{
	JsonValue V;
	EXPECT(JsonValue::Parse("{\"a\": [1, 2.5, -3e2], \"b\": \"x\\ny\\u00e9\", \"c\": true, \"d\": null}", V), "parse");
	EXPECT(V.Get("a").Size() == 3 && V.Get("a").At(2).AsNumber() == -300.0, "array");
	EXPECT(V.Get("b").AsString() == "x\ny\xC3\xA9", "string escapes: %s", V.Get("b").AsString().c_str());
	EXPECT(V.Get("c").AsBool() && V.Get("d").IsNull(), "bool/null");
	JsonValue Back;
	EXPECT(JsonValue::Parse(V.Write(), Back) && Back.Write() == V.Write(), "roundtrip");
	EXPECT(!JsonValue::Parse("{\"a\": }", Back), "rejects bad");
	EXPECT(!JsonValue::Parse("[1, 2", Back), "rejects unterminated");
}

static void TestTables()
{
	EXPECT(Materials::All().size() == 30, "30 materials incl. frictionless: %zu", Materials::All().size());
	EXPECT(Materials::Info("steel").Density == 7850.0, "steel density");
	EXPECT(Materials::Info("nonsense").Id == "plastic", "default material");
	EXPECT(Parts::All().size() == 85, "part count %zu", Parts::All().size());
	for (const PartType& T : Parts::All())
	{
		bool Known = false;
		for (const std::string& C : Parts::Categories()) { Known = Known || C == T.Category; }
		EXPECT(Known, "category of %s", T.Id.c_str());
	}
	EXPECT(Settings::ForType("motor").size() == 4, "motor settings");
	EXPECT(Settings::Find("large_cannon", "projectile_material")->Default.Text == "Steel", "cannon projectile default");
	EXPECT(Ports::InputsOf("motor").size() == 3 && Ports::OutputsOf("motor").size() == 1, "motor ports");
	const PortDef* Ammo = Ports::Find(Ports::InputsOf("machine_gun"), "ammo");
	EXPECT(Ammo && Ports::OutputsFor(*Ammo, "ammo_generator").size() == 1, "ammo connects only to ammo");
	Color C;
	EXPECT(Materials::ColorFromHtml("#7a8088", C) && Materials::ColorToHtml(C) == "#7a8088", "html colour");
}

static void TestRotation()
{
	const Basis T = QuarterTurn(1);
	EXPECT(T.Col[0].IsNearlyEqual(Vec3(0, 0, -1)) && T.Col[2].IsNearlyEqual(Vec3(1, 0, 0)), "T is a quarter turn about Y");
	JsonValue Entry;
	JsonValue::Parse("{\"type\":\"wedge\",\"pos\":[1.5,0.5,1.5],\"rotation\":[0,180,0]}", Entry);
	PartRecord R;
	EXPECT(SaveFormat::EntryToRecord(Entry, R), "entry");
	EXPECT(R.Rot.Col[0].IsNearlyEqual(Vec3(-1, 0, 0)) && R.Rot.Col[2].IsNearlyEqual(Vec3(0, 0, -1)), "180 about Y");
	EXPECT(R.Pos == IVec3(300, 100, 300), "pos in units");
}

static void TestSaves(const std::string& SavesDir)
{
	BuildModel Model;
	SaveFormat::LoadResult Result = SaveFormat::LoadBuild(Model, ReadFile(SavesDir + "/example-gate.json"), "example-gate");
	EXPECT(Result.Error.empty(), "load error %s", Result.Error.c_str());
	EXPECT(Result.Loaded == 8 && Result.Skipped == 0, "gate parts %d skipped %d", Result.Loaded, Result.Skipped);
	const std::string Saved = SaveFormat::SaveBuild(Model, "copy");
	BuildModel Again;
	Result = SaveFormat::LoadBuild(Again, Saved, "copy");
	EXPECT(Result.Loaded == 8, "reload");
	bool Same = Model.PartCount() == Again.PartCount();
	const auto A = Model.All();
	const auto B = Again.All();
	auto ItA = A.begin();
	auto ItB = B.begin();
	for (; Same && ItA != A.end(); ++ItA, ++ItB)
	{
		Same = ItA->second == ItB->second;
	}
	EXPECT(Same, "save/load keeps every record");
	EXPECT(Model.CanUndo(), "loading is one undo step");
	Model.Undo();
	EXPECT(Model.PartCount() == 0, "undo the load");
	Model.Redo();
	EXPECT(Model.PartCount() == 8, "redo the load");

	BuildModel Ring;
	Result = SaveFormat::LoadBuild(Ring, ReadFile(SavesDir + "/ring-test.json"), "ring-test");
	EXPECT(Result.Error.empty() && Result.Loaded > 0, "ring-test loads (%d, %d)", Result.Loaded, Result.Skipped);

	// Links survive a save.
	BuildModel Linked;
	const int Input = Linked.AddPart(Records::Make("input_block", IVec3(100, 50, 100), Basis(), Records::DefaultSizeUnits("input_block"), "plastic", Color()));
	const int Motor = Linked.AddPart(Records::Make("motor", IVec3(500, 100, 100), Basis(), Records::DefaultSizeUnits("motor"), "steel", Color()));
	EXPECT(Input && Motor, "placed");
	Links L;
	L["clockwise"] = Link{Input, "pressed"};
	Linked.SetLinks(Motor, L);
	BuildModel LinkedAgain;
	SaveFormat::LoadBuild(LinkedAgain, SaveFormat::SaveBuild(Linked, "l"), "l");
	bool HasLink = false;
	for (const auto& Pair : LinkedAgain.All())
	{
		if (Pair.second.Type == "motor") { HasLink = Pair.second.Links.count("clockwise") && Pair.second.Links.at("clockwise").Output == "pressed"; }
	}
	EXPECT(HasLink, "link kept");
	// Removing the source drops the link.
	Linked.RemovePart(Input);
	EXPECT(Linked.Get(Motor).Links.empty(), "link pruned");
}

static void TestRules()
{
	BuildModel Model;
	const IVec3 One = Records::DefaultSizeUnits("block");
	const int A = Model.AddPart(Records::Make("block", IVec3(100, 100, 100), Basis(), One, "steel", Color()));
	EXPECT(A != 0, "first block");
	EXPECT(Model.AddPart(Records::Make("block", IVec3(100, 100, 100), Basis(), One, "steel", Color())) == 0, "identical refused");
	EXPECT(Model.CanPlace(Records::Make("wedge", IVec3(100, 100, 100), Basis(), One, "wood", Color())) == "Materials must be homogeneous to overlap.", "different materials");
	EXPECT(Model.AddPart(Records::Make("wedge", IVec3(100, 100, 100), Basis(), One, "steel", Color())) != 0, "same material overlaps");
	EXPECT(Model.AddPart(Records::Make("block", IVec3(-100, 100, 100), Basis(), One, "steel", Color())) == 0, "outside");
	EXPECT(Model.AddPart(Records::Make("block", IVec3(300, 100, 100), Basis(), One, "wood", Color())) != 0, "touching is fine");
	const double Mass = Records::MassOf(Model.Get(A));
	EXPECT(std::fabs(Mass - 7850.0 / 64.0) < 1e-6, "steel block mass %f", Mass);
	Model.Paint(A, "gold", Color(1, 0, 0));
	int Gold = 0;
	for (const auto& Pair : Model.All()) { Gold += Pair.second.Material == "gold"; }
	EXPECT(Gold == 2, "paint covers the overlap group (%d)", Gold);
}

static void TestGeometry()
{
	for (const PartType& T : Parts::All())
	{
		const MeshData& M = Geometry::UnitMesh(T.Id);
		EXPECT(!M.Tris.empty(), "mesh for %s", T.Id.c_str());
		bool Inside = true;
		for (const Tri& Tr : M.Tris)
		{
			for (const Vec3* P : {&Tr.A, &Tr.B, &Tr.C})
			{
				Inside = Inside && std::fabs(P->X) <= 0.52 && std::fabs(P->Y) <= 0.52 && std::fabs(P->Z) <= 0.52;
			}
		}
		EXPECT(Inside, "mesh of %s stays in its box", T.Id.c_str());
		EXPECT(!Geometry::UnitConvexPieces(T.Id).empty(), "pieces for %s", T.Id.c_str());
	}
	// Convex shapes: every face looks away from the middle.
	for (const char* Id : {"block", "wedge", "cylinder", "sphere", "cone", "pyramid", "corner_wedge", "inverse_corner", "hemisphere"})
	{
		const MeshData& M = Geometry::UnitMesh(Id);
		Vec3 Middle;
		for (const Tri& Tr : M.Tris) { Middle += (Tr.A + Tr.B + Tr.C) / 3.0; }
		Middle = Middle / (double)M.Tris.size();
		int Wrong = 0;
		for (const Tri& Tr : M.Tris)
		{
			const Vec3 N = (Tr.B - Tr.A).Cross(Tr.C - Tr.A);
			if (N.Dot((Tr.A + Tr.B + Tr.C) / 3.0 - Middle) < -1e-9) { ++Wrong; }
		}
		EXPECT(Wrong == 0, "%s has %d inward faces", Id, Wrong);
	}
	// Enclosed volume (divergence theorem) matches the catalogue's volume share.
	for (const char* Id : {"block", "wedge", "cylinder", "sphere", "cone", "frustum", "ring", "pipe", "arch", "convex_curve", "concave_curve", "half_cylinder", "torus"})
	{
		double Volume = 0.0;
		for (const Tri& Tr : Geometry::UnitMesh(Id).Tris) { Volume += Tr.A.Dot(Tr.B.Cross(Tr.C)) / 6.0; }
		const double Expected = Parts::Get(Id).VolumeFraction;
		EXPECT(std::fabs(Volume - Expected) < 0.02, "%s volume %.4f vs %.4f", Id, Volume, Expected);
	}
	EXPECT(Geometry::ContainsUnitPoint("ring", Vec3(0.4, 0, 0)) && !Geometry::ContainsUnitPoint("ring", Vec3(0.1, 0, 0)), "ring hole");
	EXPECT(Geometry::ContainsUnitPoint("wedge", Vec3(-0.4, -0.4, 0)) && !Geometry::ContainsUnitPoint("wedge", Vec3(0.4, 0.4, 0)), "wedge");
	EXPECT(!Geometry::ContainsUnitPoint("arch", Vec3(0, -0.45, 0)) && Geometry::ContainsUnitPoint("arch", Vec3(0.45, -0.45, 0)), "arch opening");
	const auto Pieces = Geometry::UnitConvexPieces("arch");
	EXPECT(Pieces.size() >= 2, "arch splits into %zu pieces", Pieces.size());
	EXPECT(Geometry::UnitConvexPieces("torus").size() == 16, "torus wedges");
	const MeshData& Gear = Geometry::UnitMesh("gear", [] { MeshOptions O; O.Teeth = 30; return O; }());
	EXPECT(Gear.HasDetail, "gear detail");
	EXPECT(Geometry::RayTriangle(Vec3(0, 0, -1), Vec3(0, 0, 1), Vec3(-1, -1, 0), Vec3(1, -1, 0), Vec3(0, 1, 0)) > 0.99, "ray hits");
	EXPECT(Geometry::RayBox(Vec3(0, 0, -5), Vec3(0, 0, 1), Box(Vec3(-1, -1, -1), Vec3(2, 2, 2))) == 4.0, "ray box");
}

static void TestSpawnPlan(const std::string& SavesDir)
{
	BuildModel Model;
	SaveFormat::LoadBuild(Model, ReadFile(SavesDir + "/example-gate.json"), "gate");
	SpawnPlan Plan = SpawnPlan::Make(Model);
	EXPECT(Plan.Bodies.size() == 1, "the gate is one body (%zu)", Plan.Bodies.size());
	EXPECT(std::fabs(Plan.Bodies[0].Mass - Model.TotalMass()) / Model.TotalMass() < 0.25, "body mass %f vs %f",
		Plan.Bodies[0].Mass, Model.TotalMass());

	// Two blocks meeting only along an edge do not weld.
	BuildModel Edge;
	const IVec3 One = Records::DefaultSizeUnits("block");
	Edge.AddPart(Records::Make("block", IVec3(100, 100, 100), Basis(), One, "steel", Color()));
	Edge.AddPart(Records::Make("block", IVec3(300, 300, 100), Basis(), One, "steel", Color()));
	EXPECT(SpawnPlan::Make(Edge).Bodies.size() == 2, "edge contact is no weld");

	// A hinge between two blocks: two bodies and one free joint.
	BuildModel Hinged;
	Hinged.AddPart(Records::Make("block", IVec3(100, 100, 100), Basis(), One, "steel", Color()));
	Hinged.AddPart(Records::Make("hinge", IVec3(100, 300, 100), Basis(), One, "steel", Color()));
	Hinged.AddPart(Records::Make("block", IVec3(100, 500, 100), Basis(), One, "steel", Color()));
	SpawnPlan HPlan = SpawnPlan::Make(Hinged);
	EXPECT(HPlan.Bodies.size() == 2 && HPlan.Joints.size() == 1 && HPlan.Locked == 0, "hinge: %zu bodies, %zu joints, %d locked",
		HPlan.Bodies.size(), HPlan.Joints.size(), HPlan.Locked);
	if (!HPlan.Joints.empty())
	{
		EXPECT(HPlan.Joints[0].Kind == "hinge" && HPlan.Joints[0].Axis.IsNearlyEqual(Vec3(1, 0, 0)), "hinge about X");
	}

	// A block that touches both leaves welds to one, and the hinge is locked
	// only if both leaves end up on one body.
	BuildModel Locked;
	Locked.AddPart(Records::Make("hinge", IVec3(100, 100, 100), Basis(), One, "steel", Color()));
	Locked.AddPart(Records::Make("block", IVec3(300, 100, 100), Basis(), One, "steel", Color()));
	SpawnPlan LPlan = SpawnPlan::Make(Locked);
	EXPECT(LPlan.Locked == 0 && LPlan.Bodies.size() == 2, "side block picks one leaf (%zu bodies, %d locked)", LPlan.Bodies.size(), LPlan.Locked);

	// Anchors fix their body.
	BuildModel Anchored;
	Anchored.AddPart(Records::Make("anchor_block", IVec3(100, 100, 100), Basis(), One, "steel", Color()));
	EXPECT(SpawnPlan::Make(Anchored).Bodies[0].Fixed, "anchored");

	// Overlap is counted once.
	BuildModel Over;
	Over.AddPart(Records::Make("block", IVec3(200, 100, 100), Basis(), IVec3(400, 200, 200), "steel", Color()));
	Over.AddPart(Records::Make("block", IVec3(300, 100, 100), Basis(), IVec3(400, 200, 200), "steel", Color()));
	const double Expected = 7850.0 * 0.25 * 0.25 * 0.625;   // 2.5 cells long once merged
	EXPECT(std::fabs(SpawnPlan::Make(Over).Bodies[0].Mass - Expected) < 0.5, "overlap counted once: %f vs %f",
		SpawnPlan::Make(Over).Bodies[0].Mass, Expected);

	// A closed box of plates traps air.
	BuildModel Boat;
	for (int X = 0; X < 4; ++X)
	{
		for (int Z = 0; Z < 4; ++Z)
		{
			for (int Y = 0; Y < 4; ++Y)
			{
				const bool Wall = X == 0 || X == 3 || Y == 0 || Y == 3 || Z == 0 || Z == 3;
				if (Wall) { Boat.AddPart(Records::Make("block", IVec3(X * 200 + 100, Y * 200 + 100, Z * 200 + 100), Basis(), One, "steel", Color())); }
			}
		}
	}
	SpawnPlan BPlan = SpawnPlan::Make(Boat);
	double Reach = 0.0;
	const std::vector<FloatCube> Floats = BPlan.Floats(BPlan.Bodies[0], Reach);
	double Volume = 0.0;
	for (const FloatCube& F : Floats) { Volume += F.Volume; }
	const double Solid = 56.0 / 64.0;
	const double Air = 8.0 / 64.0;
	EXPECT(std::fabs(Volume - (Solid + Air)) < 1e-6, "floats: %f vs %f", Volume, Solid + Air);
	const std::vector<DragAxis> Drag = BPlan.DragAreas(BPlan.Bodies[0]);
	EXPECT(Drag.size() == 3 && std::fabs(Drag[0].Area - 1.0) < 1e-9, "drag outline 1 m^2: %f", Drag[0].Area);
}

static void TestLogic()
{
	Logic::GateState S;
	EXPECT(Logic::BooleanGate("XOR", true, false, 0, 0.01, true, S), "xor");
	Logic::GateState Latch;
	Logic::BooleanGate("Latch", true, false, 0, 0.01, true, Latch);
	EXPECT(Logic::BooleanGate("Latch", false, false, 0, 0.01, true, Latch), "latch holds");
	EXPECT(!Logic::BooleanGate("Latch", false, true, 0, 0.01, true, Latch), "latch resets");
	double R = 0.0;
	bool T = false;
	Logic::NumberGate("Map range", 5, 0, 10, 0, R, T);
	EXPECT(R == 0.5 && T, "map range");
	Logic::NumberGate("Modulo", -1, 360, 0, 0, R, T);
	EXPECT(R == 359.0, "modulo");
	Logic::GateState Pid;
	const double Out = Logic::Pid(1.0, 0.0, 2.0, 0.0, 0.0, -1.0, 1.0, 0.01, Pid);
	EXPECT(Out == 1.0, "pid clamps");
	double Set = 0.0, Rate = 0.0;
	for (int Step = 0; Step < 1000; ++Step) { Logic::EaseToward(Set, Rate, 1.0, 0.5, 2.0, 0.01); }
	EXPECT(Set == 1.0 && Rate == 0.0, "ease arrives");
	EXPECT(std::fabs(Aero::AirDensity(-901.0) - 1.225) < 1e-9, "sea level air");
	const Vec3 F = Aero::PanelForce(Vec3(0, -1, -10).Normalized() * 50.0, Vec3(0, 1, 0), 1.0, false, 1.225);
	EXPECT(F.Y > 0.0, "a panel meeting air from below lifts: %f", F.Y);
}

int main(int Argc, char** Argv)
{
	const std::string SavesDir = Argc > 1 ? Argv[1] : "saves";
	TestJson();
	TestTables();
	TestRotation();
	TestSaves(SavesDir);
	TestRules();
	TestGeometry();
	TestSpawnPlan(SavesDir);
	TestLogic();
	std::printf("%d checks, %d failures\n", Checks, Failures);
	return Failures == 0 ? 0 : 1;
}
