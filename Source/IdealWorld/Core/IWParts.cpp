#include "IWParts.h"

#include "IWMaterials.h"

namespace IW
{
	namespace PartsDetail
	{
		PartType Row(const char* Id, const char* Name, const char* Category, Vec3 Size, double Volume, Vec3 Centroid,
			CollisionKind Kind)
		{
			PartType T;
			T.Id = Id;
			T.Name = Name;
			T.Category = Category;
			T.DefaultSizeCells = Size;
			T.VolumeFraction = Volume;
			T.UnitCentroid = Centroid;
			T.Collision = Kind;
			return T;
		}

		std::vector<PartType> BuildParts()
		{
			const CollisionKind B = CollisionKind::Box;
			const CollisionKind C = CollisionKind::Convex;
			const CollisionKind K = CollisionKind::Concave;
			const Vec3 O(0, 0, 0);
			std::vector<PartType> L;
			L.push_back(Row("block", "Block", "Basic", Vec3(1, 1, 1), 1.0, O, B));
			L.push_back(Row("wedge", "Wedge", "Basic", Vec3(1, 1, 1), 0.5, Vec3(-0.16667, -0.16667, 0), C));
			L.push_back(Row("triangle_wedge", "Triangle wedge", "Basic", Vec3(1, 1, 1), 0.5, Vec3(0, -0.16667, 0), C));
			L.push_back(Row("cylinder", "Cylinder", "Basic", Vec3(1, 1, 1), 0.785398, O, C));
			L.push_back(Row("sphere", "Sphere", "Basic", Vec3(1, 1, 1), 0.523599, O, C));
			L.push_back(Row("slab", "Half block", "Structural", Vec3(1, 0.5, 1), 1.0, O, B));
			L.push_back(Row("plate", "Plate", "Structural", Vec3(1, 0.25, 1), 1.0, O, B));
			L.push_back(Row("post", "Post", "Structural", Vec3(0.5, 1, 0.5), 1.0, O, B));
			L.push_back(Row("rod", "Rod", "Structural", Vec3(0.25, 1, 0.25), 0.785398, O, C));
			L.push_back(Row("corner_wedge", "Corner wedge", "Advanced", Vec3(1, 1, 1), 0.166667, Vec3(-0.25, -0.25, -0.25), C));
			L.push_back(Row("inverse_corner", "Inverse corner", "Advanced", Vec3(1, 1, 1), 0.833333, Vec3(-0.05, -0.05, -0.05), C));
			L.push_back(Row("pyramid", "Pyramid", "Advanced", Vec3(1, 1, 1), 0.333333, Vec3(0, -0.25, 0), C));
			L.push_back(Row("cone", "Cone", "Advanced", Vec3(1, 1, 1), 0.261799, Vec3(0, -0.25, 0), C));
			L.push_back(Row("frustum", "Tapered cylinder", "Advanced", Vec3(1, 1, 1), 0.458149, Vec3(0, -0.10714, 0), C));
			L.push_back(Row("hemisphere", "Hemisphere", "Advanced", Vec3(1, 0.5, 1), 0.523599, Vec3(0, -0.125, 0), C));
			L.push_back(Row("half_cylinder", "Half cylinder", "Advanced", Vec3(1, 0.5, 1), 0.785398, Vec3(0, -0.07559, 0), C));
			L.push_back(Row("convex_curve", "Convex curve", "Advanced", Vec3(1, 1, 1), 0.785398, Vec3(-0.07559, -0.07559, 0), C));
			L.push_back(Row("concave_curve", "Concave curve", "Advanced", Vec3(1, 1, 1), 0.214602, Vec3(-0.27665, -0.27665, 0), K));
			L.push_back(Row("arch", "Arch", "Advanced", Vec3(1, 1, 1), 0.748673, Vec3(0, 0.11086, 0), K));
			L.push_back(Row("ring", "Ring", "Advanced", Vec3(1, 1, 1), 0.502655, O, K));
			L.push_back(Row("pipe", "Pipe", "Advanced", Vec3(1, 1, 1), 0.282743, O, K));
			L.push_back(Row("torus", "Torus", "Advanced", Vec3(1, 0.25, 1), 0.462637, O, K));
			L.push_back(Row("motor", "Motor", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("servo", "Servo", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("door_hinge_servo", "Door hinge servo", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("hinge", "Hinge", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("ball_joint", "Ball joint", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("bearing", "Bearing block", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("piston", "Piston", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("solenoid", "Solenoid", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("spring", "Spring", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("gear", "Gear", "Mechanical", Vec3(1, 0.375, 1), 0.5, O, B));
			L.push_back(Row("bevel_gear", "Bevel gear", "Mechanical", Vec3(1, 0.5, 1), 0.5, O, B));
			L.push_back(Row("rack", "Rack", "Mechanical", Vec3(1, 0.5, 0.5), 0.5, O, B));
			L.push_back(Row("pulley", "Belt pulley", "Mechanical", Vec3(1, 0.375, 1), 0.5, O, B));
			L.push_back(Row("crankshaft", "Crankshaft", "Mechanical", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("wheel", "Wheel", "Mechanical", Vec3(0.5, 1, 1), 0.5, O, C));
			L.push_back(Row("linkage_bar", "Linkage bar", "Mechanical", Vec3(1, 0.25, 0.25), 0.5, O, B));
			L.push_back(Row("clutch", "Clutch", "Mechanical", Vec3(1, 0.5, 1), 0.5, O, B));
			L.push_back(Row("brake", "Brake", "Mechanical", Vec3(1, 0.375, 1), 0.5, O, B));
			L.push_back(Row("light_panel", "Light panel", "Logic", Vec3(1, 0.125, 1), 0.5, O, B));
			L.push_back(Row("vanish_block", "Vanishing block", "Logic", Vec3(1, 1, 1), 1.0, O, B));
			L.push_back(Row("impact_detector", "Impact detector", "Logic", Vec3(1, 1, 1), 0.4, O, B));
			L.push_back(Row("explosive_block", "Explosive block", "Combat", Vec3(1, 1, 1), 0.8, O, B));
			L.push_back(Row("machine_gun", "Machine gun", "Combat", Vec3(1, 1, 2), 0.3, O, B));
			L.push_back(Row("hitscan_gun", "Hit-scan gun", "Combat", Vec3(1, 1, 2), 0.3, O, B));
			L.push_back(Row("large_cannon", "Large cannon", "Combat", Vec3(1, 1, 4), 0.3, O, B));
			L.push_back(Row("ammo_generator", "Ammo generator", "Combat", Vec3(2, 2, 2), 0.7, O, B));
			L.push_back(Row("fluid_cylinder", "Fluid cylinder", "Fluid and Engine", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("pump", "Pump", "Fluid and Engine", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("valve", "Valve", "Fluid and Engine", Vec3(1, 1, 1), 0.3, O, B));
			L.push_back(Row("fluid_pipe", "Fluid pipe", "Fluid and Engine", Vec3(1, 1, 1), 0.1, O, B));
			L.push_back(Row("fuel_tank", "Fuel tank", "Fluid and Engine", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("pressure_tank", "Pressure tank", "Fluid and Engine", Vec3(1, 1, 1), 0.4, O, B));
			L.push_back(Row("combustion_cylinder", "Combustion cylinder", "Fluid and Engine", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("compressor", "Compressor", "Fluid and Engine", Vec3(1, 1, 1), 0.6, O, B));
			L.push_back(Row("combustor", "Combustor", "Fluid and Engine", Vec3(1, 1, 1), 0.4, O, B));
			L.push_back(Row("turbine", "Turbine", "Fluid and Engine", Vec3(1, 1, 1), 0.5, O, B));
			L.push_back(Row("nozzle", "Nozzle", "Fluid and Engine", Vec3(1, 1, 1), 0.3, O, B));
			L.push_back(Row("thruster", "Thruster", "Fluid and Engine", Vec3(1, 1, 1), 0.4, O, B));
			L.push_back(Row("solid_rocket", "Solid rocket", "Fluid and Engine", Vec3(1, 1, 2), 0.5, O, B));
			L.push_back(Row("aero_panel", "Square aero panel", "Aero", Vec3(1, 0.125, 1), 1.0, O, B));
			L.push_back(Row("aero_panel_triangle", "Triangle aero panel", "Aero", Vec3(1, 0.125, 1), 0.5, Vec3(-0.1667, 0, 0.1667), C));
			L.push_back(Row("aero_panel_dragless", "Dragless square panel", "Aero", Vec3(1, 0.125, 1), 1.0, O, B));
			L.push_back(Row("aero_panel_triangle_dragless", "Dragless triangle panel", "Aero", Vec3(1, 0.125, 1), 0.5, Vec3(-0.1667, 0, 0.1667), C));
			L.push_back(Row("propeller", "Propeller", "Aero", Vec3(1, 0.25, 1), 0.3, O, B));
			L.push_back(Row("lift_gas_tank", "Lift-gas tank", "Aero", Vec3(1, 1, 1), 0.1, O, B));
			L.push_back(Row("input_block", "Input", "Logic", Vec3(1, 0.5, 1), 0.5, O, B));
			L.push_back(Row("mouse_click", "Mouse click", "Logic", Vec3(1, 0.5, 1), 0.5, O, B));
			L.push_back(Row("boolean_gate", "Boolean gate", "Logic", Vec3(1, 0.5, 1), 0.5, O, B));
			L.push_back(Row("number_gate", "Number logic", "Logic", Vec3(1, 0.5, 1), 0.5, O, B));
			L.push_back(Row("pid_controller", "PID controller", "Logic", Vec3(1, 0.5, 1), 0.5, O, B));
			L.push_back(Row("pwm_generator", "PWM generator", "Logic", Vec3(1, 0.5, 1), 0.5, O, B));
			L.push_back(Row("speed_sensor", "Speed sensor", "Logic", Vec3(1, 1, 1), 0.3, O, B));
			L.push_back(Row("altitude_sensor", "Altitude sensor", "Logic", Vec3(1, 1, 1), 0.3, O, B));
			L.push_back(Row("attitude_sensor", "Attitude sensor", "Logic", Vec3(1, 1, 1), 0.3, O, B));
			L.push_back(Row("distance_sensor", "Distance sensor", "Logic", Vec3(1, 1, 1), 0.3, O, B));
			L.push_back(Row("graph_display", "Graph screen", "Logic", Vec3(4, 3, 0.25), 0.6, O, B));
			L.push_back(Row("instrument", "Instrument", "Logic", Vec3(2, 2, 0.25), 0.6, O, B));
			L.push_back(Row("seat", "Seat", "Misc", Vec3(3, 3, 3), 0.4, O, B));
			L.push_back(Row("anchor_block", "Anchor block", "Misc", Vec3(1, 1, 1), 0.9, O, B));
			L.push_back(Row("dynamic_offsetter", "Dynamic offsetter", "Misc", Vec3(1, 1, 1), 0.8, O, B));
			L.push_back(Row("magnet", "Electromagnet", "Misc", Vec3(2, 0.5, 2), 0.9, O, B));
			L.push_back(Row("winch", "Winch and hook", "Misc", Vec3(2, 1, 1), 0.6, O, B));
			L.push_back(Row("coupler", "Coupler", "Misc", Vec3(1, 1, 0.5), 0.8, O, B));

			// Parts that start in their own colour instead of the brush colour.
			const std::map<std::string, Color> Defaults = {
				{"vanish_block", Color(1.0f, 1.0f, 1.0f)},
				{"light_panel", Color(0.20f, 0.21f, 0.23f)},
				{"impact_detector", Color(0.95f, 0.75f, 0.15f)},
				{"explosive_block", Color(0.78f, 0.12f, 0.10f)},
				{"machine_gun", Color(0.30f, 0.33f, 0.27f)},
				{"hitscan_gun", Color(0.22f, 0.24f, 0.30f)},
				{"large_cannon", Color(0.30f, 0.33f, 0.27f)},
				{"ammo_generator", Color(0.30f, 0.33f, 0.27f)},
				{"lift_gas_tank", Color(0.88f, 0.92f, 0.96f)},
				{"aero_panel_dragless", Color(0.55f, 0.90f, 1.0f)},
				{"aero_panel_triangle_dragless", Color(0.55f, 0.90f, 1.0f)},
				{"anchor_block", Color(0.55f, 0.57f, 0.60f)},
				{"input_block", Color(0.20f, 0.21f, 0.23f)},
				{"mouse_click", Color(0.20f, 0.21f, 0.23f)},
				{"boolean_gate", Color(0.20f, 0.21f, 0.23f)},
				{"number_gate", Color(0.20f, 0.21f, 0.23f)},
				{"pid_controller", Color(0.20f, 0.21f, 0.23f)},
				{"pwm_generator", Color(0.20f, 0.21f, 0.23f)},
				{"speed_sensor", Color(0.20f, 0.21f, 0.23f)},
				{"altitude_sensor", Color(0.20f, 0.21f, 0.23f)},
				{"attitude_sensor", Color(0.20f, 0.21f, 0.23f)},
				{"distance_sensor", Color(0.20f, 0.21f, 0.23f)},
				{"graph_display", Color(0.16f, 0.17f, 0.19f)},
				{"instrument", Color(0.12f, 0.12f, 0.13f)},
				{"magnet", Color(0.62f, 0.12f, 0.10f)},
				{"winch", Color(0.95f, 0.72f, 0.10f)},
				{"coupler", Color(0.35f, 0.37f, 0.40f)},
			};
			for (PartType& T : L)
			{
				const auto Found = Defaults.find(T.Id);
				if (Found != Defaults.end())
				{
					T.HasDefaultColor = true;
					T.DefaultColor = Found->second;
				}
			}
			return L;
		}
	}

	namespace Parts
	{
		const std::vector<std::string>& Categories()
		{
			static const std::vector<std::string> List = {
				"Basic", "Structural", "Advanced", "Mechanical", "Fluid and Engine", "Aero", "Logic", "Combat", "Misc"};
			return List;
		}

		const std::vector<PartType>& All()
		{
			static const std::vector<PartType> List = PartsDetail::BuildParts();
			return List;
		}

		bool Has(const std::string& Id)
		{
			for (const PartType& T : All())
			{
				if (T.Id == Id) { return true; }
			}
			return false;
		}

		const PartType& Get(const std::string& Id)
		{
			static std::map<std::string, size_t> Index;
			const std::vector<PartType>& List = All();
			if (Index.empty())
			{
				for (size_t At = 0; At < List.size(); ++At) { Index[List[At].Id] = At; }
			}
			const auto Found = Index.find(Id);
			return Found == Index.end() ? List.front() : List[Found->second];
		}

		std::string Name(const std::string& Id) { return Get(Id).Name; }

		Color PlacementColor(const std::string& Id, const Color& BrushColor)
		{
			const PartType& T = Get(Id);
			return T.HasDefaultColor ? T.DefaultColor : BrushColor;
		}

		bool IsAeroPanel(const std::string& Id) { return Id.rfind("aero_panel", 0) == 0; }
		bool IsTrianglePanel(const std::string& Id) { return Id.rfind("aero_panel_triangle", 0) == 0; }
		bool IsDragless(const std::string& Id)
		{
			const std::string Tail = "_dragless";
			return Id.size() >= Tail.size() && Id.compare(Id.size() - Tail.size(), Tail.size(), Tail) == 0;
		}
		bool SitsMidCell(const std::string& Id) { return IsAeroPanel(Id); }

		std::vector<std::string> SizeCaptions(const std::string& Id)
		{
			if (Id == "large_cannon")
			{
				return {"Bore size X (blocks)", "Bore size Y (blocks)", "Barrel length (blocks)"};
			}
			return {"Size X (blocks)", "Size Y (blocks)", "Size Z (blocks)"};
		}

		bool IsControlPart(const std::string& Id)
		{
			return Id == "input_block" || Id == "boolean_gate" || Id == "number_gate" || Id == "pid_controller"
				|| Id == "pwm_generator" || Id == "mouse_click";
		}
	}

	// --- Settings -------------------------------------------------------------

	namespace SettingsDetail
	{
		SettingDef Number(const char* Id, const char* Name, const char* Unit, double Initial, double Low, double High, double Step)
		{
			SettingDef S;
			S.Id = Id; S.Name = Name; S.Kind = SettingKind::Number; S.Unit = Unit;
			S.Min = Low; S.Max = High; S.Step = Step;
			S.Default = PropValue::MakeNumber(Initial);
			return S;
		}
		SettingDef FlagDef(const char* Id, const char* Name, bool Initial)
		{
			SettingDef S;
			S.Id = Id; S.Name = Name; S.Kind = SettingKind::Flag;
			S.Default = PropValue::MakeFlag(Initial);
			return S;
		}
		SettingDef Choice(const char* Id, const char* Name, const std::vector<std::string>& Options)
		{
			SettingDef S;
			S.Id = Id; S.Name = Name; S.Kind = SettingKind::Choice; S.Options = Options;
			S.Default = PropValue::MakeChoice(Options.front());
			return S;
		}
		SettingDef ColorDef(const char* Id, const char* Name, Color Initial)
		{
			SettingDef S;
			S.Id = Id; S.Name = Name; S.Kind = SettingKind::Color;
			S.Default = PropValue::MakeColor(Initial);
			return S;
		}
		SettingDef TextDef(const char* Id, const char* Name, const char* Initial)
		{
			SettingDef S;
			S.Id = Id; S.Name = Name; S.Kind = SettingKind::Text;
			S.Default = PropValue::MakeText(Initial);
			return S;
		}

		std::map<std::string, std::vector<SettingDef>> BuildTable()
		{
			std::map<std::string, std::vector<SettingDef>> T;
			const std::vector<std::string> IdleLinear = {"Hold position", "Return to starting length"};
			const std::vector<std::string> IdleAngular = {"Return to default angle", "Hold position"};

			// --- Mechanical ---
			T["motor"] = {
				Number("max_torque", "Max torque", "N\xC2\xB7m", 100, 0, 1000000, 1),
				Number("max_speed", "Max speed", "RPM", 600, 0, 100000, 1),
				Number("acceleration", "Acceleration", "RPM/s", 600, 0, 1000000, 1),
				Choice("when_idle", "With no input", {"Hold still", "Spin freely"}),
			};
			const std::vector<SettingDef> Servo = {
				Number("default_angle", "Default angle", "\xC2\xB0", 0, -180, 180, 1),
				Number("positive_angle", "Positive target", "\xC2\xB0", 90, -180, 180, 1),
				Number("negative_angle", "Negative target", "\xC2\xB0", -90, -180, 180, 1),
				Number("speed", "Speed", "\xC2\xB0/s", 90, 0, 10000, 1),
				Number("easing", "Easing", "\xC2\xB0/s\xC2\xB2", 360, 0, 100000, 1),
				Number("max_torque", "Max torque", "N\xC2\xB7m", 100, 0, 1000000, 1),
				Choice("when_idle", "With no input", IdleAngular),
			};
			T["servo"] = Servo;
			T["door_hinge_servo"] = Servo;
			T["hinge"] = {
				FlagDef("limited", "Use angle limits", false),
				Number("min_angle", "Lowest angle", "\xC2\xB0", -90, -180, 180, 1),
				Number("max_angle", "Highest angle", "\xC2\xB0", 90, -180, 180, 1),
				Number("friction", "Friction torque", "N\xC2\xB7m", 0, 0, 100000, 0.1),
			};
			T["ball_joint"] = {
				FlagDef("limited", "Use angle limit", false),
				Number("max_angle", "Largest tilt", "\xC2\xB0", 45, 0, 180, 1),
				FlagDef("carries_rotation", "Carries rotation through", false),
			};
			T["bearing"] = {Number("friction", "Friction torque", "N\xC2\xB7m", 0, 0, 100000, 0.1)};
			T["piston"] = {
				Number("current_length", "Current length", "blocks", 0, 0, 64, 0.005),
				Number("target_length", "Target length", "blocks", 1, 0, 64, 0.005),
				Number("max_length", "Longest reach", "blocks", 1, 0, 64, 0.005),
				Number("speed", "Speed", "m/s", 0.5, 0, 1000, 0.01),
				Number("easing", "Easing speed", "m/s\xC2\xB2", 2, 0, 10000, 0.01),
				Number("max_force", "Max force", "N", 1000, 0, 100000000, 1),
				Choice("when_idle", "With no input", IdleLinear),
				FlagDef("sensor_mode", "Sensor mode (outputs pressure)", false),
			};
			T["solenoid"] = {
				Number("stroke", "Stroke", "blocks", 0.5, 0, 16, 0.005),
				Number("force", "Force", "N", 500, 0, 100000000, 1),
			};
			T["spring"] = {
				Number("rest_length", "Rest length", "blocks", 1, 0, 64, 0.005),
				Number("stiffness", "Stiffness", "N/m", 1000, 0, 100000000, 1),
				Number("damping", "Damping", "N\xC2\xB7s/m", 10, 0, 1000000, 0.1),
			};
			T["gear"] = {Number("teeth", "Teeth", "", 12, 4, 400, 1)};
			T["bevel_gear"] = {Number("teeth", "Teeth", "", 12, 4, 400, 1)};
			T["rack"] = {Number("teeth_per_block", "Teeth per block", "", 5, 1, 100, 1)};
			T["crankshaft"] = {
				Number("throw_radius", "Throw radius", "blocks", 0.35, 0.005, 16, 0.005),
				Number("rod_length", "Rod length", "blocks", 1.5, 0.005, 64, 0.005),
			};
			T["clutch"] = {
				Number("max_torque", "Max torque", "N\xC2\xB7m", 200, 0, 1000000, 1),
				FlagDef("engaged", "Engaged with no input", true),
			};
			T["brake"] = {Number("max_torque", "Max torque", "N\xC2\xB7m", 200, 0, 1000000, 1)};
			T["wheel"] = {
				FlagDef("free_spinning", "Spins freely on its own hub", true),
				Number("friction", "Hub friction torque", "N\xC2\xB7m", 0, 0, 100000, 0.1),
			};
			T["pulley"] = {Number("belt", "Belt number (0 for no belt)", "", 1, 0, 99, 1)};
			T["linkage_bar"] = {Choice("ends", "End joints", {"Ball joints", "Hinges about Y", "Hinges about Z"})};

			// --- Fluid and engine ---
			T["fluid_cylinder"] = {
				Number("bore", "Bore", "blocks", 0.6, 0.005, 16, 0.005),
				Number("stroke", "Stroke", "blocks", 1, 0.005, 64, 0.005),
				Choice("fluid", "Fluid", {"Hydraulic oil", "Gas"}),
				FlagDef("sensor_mode", "Sensor mode (outputs pressure)", false),
			};
			T["pump"] = {
				Number("max_flow", "Max flow", "L/s", 1, 0, 10000, 0.01),
				Number("max_pressure", "Max pressure", "MPa", 10, 0, 1000, 0.1),
			};
			T["valve"] = {FlagDef("open", "Open with no input", false)};
			T["fuel_tank"] = {
				Number("energy_density", "Energy per volume", "MJ/L", 34, 0, 1000, 0.1),
				Number("fuel_density", "Fuel density", "kg/L", 0.75, 0, 30, 0.01),
				Number("fill", "Starting fill", "%", 100, 0, 100, 1),
			};
			T["pressure_tank"] = {Number("pressure", "Starting pressure", "kPa", 101.3, 0, 1000000, 0.1)};
			T["combustion_cylinder"] = {
				Number("bore", "Bore", "blocks", 0.6, 0.005, 16, 0.005),
				Number("stroke", "Stroke", "blocks", 0.6, 0.005, 64, 0.005),
				FlagDef("auto_spark", "Fires by itself at the top (when Spark is not linked)", true),
			};
			T["compressor"] = {Number("pressure_ratio", "Pressure ratio", "", 8, 1, 100, 0.1)};
			T["combustor"] = {Number("max_fuel_flow", "Max fuel flow", "L/s", 0.1, 0, 1000, 0.001)};
			T["thruster"] = {
				Number("thrust", "Thrust", "N", 1000, 0, 1000000000, 1),
				FlagDef("ideal", "Runs without fuel", true),
			};
			T["solid_rocket"] = {
				Number("thrust", "Thrust", "N", 5000, 0, 1000000000, 1),
				Number("burn_time", "Burn time", "s", 10, 0, 100000, 0.1),
				FlagDef("ideal", "Never burns out", false),
			};

			// --- Aero ---
			T["propeller"] = {
				Number("blades", "Blade count", "", 4, 2, 12, 1),
				Number("pitch", "Blade pitch", "\xC2\xB0", 20, 0, 90, 1),
			};
			T["lift_gas_tank"] = {Number("lift_per_block", "Lift at sea level", "N per block", 1, 0, 1000000, 0.1)};

			// --- Logic ---
			const SettingDef Shown = FlagDef("visible_after_spawn", "Visible after spawn (weighs and collides, for checking)", false);
			T["input_block"] = {
				TextDef("key", "Key", "W"),
				Choice("mode", "Mode", {"Hold", "Toggle"}),
				Choice("output", "Output", {"Boolean", "Number from two keys"}),
				TextDef("second_key", "Second key", "S"),
				Shown,
			};
			T["mouse_click"] = {
				Choice("button", "Mouse button", {"Left", "Right", "Middle"}),
				Choice("mode", "Mode", {"Hold", "Toggle", "Pulse"}),
				Shown,
			};
			T["boolean_gate"] = {
				Choice("operation", "Gate", {"AND", "OR", "NOT", "XOR", "NAND", "NOR", "Latch", "Timer"}),
				Number("seconds", "Timer length", "s", 1, 0, 100000, 0.01),
				Shown,
			};
			T["pwm_generator"] = {
				Number("frequency", "Frequency", "Hz", 2, 0.01, 120, 0.01),
				Number("duty", "Duty cycle", "%", 50, 0, 100, 0.1),
				Shown,
			};
			T["number_gate"] = {
				Choice("operation", "Operation", {"Constant", "Add", "Subtract", "Multiply", "Divide", "Absolute value",
					"Min", "Max", "Clamp", "Compare", "Select", "Map range", "Modulo", "Between"}),
				Number("value", "Constant", "", 0, -1000000000, 1000000000, 0.001),
				Shown,
			};
			T["pid_controller"] = {
				Number("kp", "Kp", "", 1, -1000000, 1000000, 0.001),
				Number("ki", "Ki", "", 0, -1000000, 1000000, 0.001),
				Number("kd", "Kd", "", 0, -1000000, 1000000, 0.001),
				Number("output_min", "Lowest output", "", -1, -1000000000, 1000000000, 0.001),
				Number("output_max", "Highest output", "", 1, -1000000000, 1000000000, 0.001),
				Shown,
			};
			T["distance_sensor"] = {Number("range", "Range", "m", 100, 0, 100000, 0.1)};
			T["instrument"] = {
				Choice("kind", "Shows", {"Airspeed", "Altitude", "Artificial horizon", "Fuel", "Value"}),
				Number("top", "Top of the dial (Airspeed m/s, Altitude m, Value)", "", 100, 0.000001, 1000000000, 0.001),
				Number("bottom", "Bottom of the dial (Value)", "", 0, -1000000000, 1000000000, 0.001),
				TextDef("label", "Name on the dial", ""),
			};
			T["graph_display"] = {
				Number("window", "Time shown", "s", 10, 1, 600, 0.5),
				Number("low", "Bottom of the scale (same as top: automatic)", "", 0, -1000000000, 1000000000, 0.001),
				Number("high", "Top of the scale", "", 0, -1000000000, 1000000000, 0.001),
				TextDef("label_a", "Name of line A", "A"),
				TextDef("label_b", "Name of line B", "B"),
				TextDef("label_c", "Name of line C", "C"),
				TextDef("label_d", "Name of line D", "D"),
				FlagDef("on_screen", "Also show on screen while seated", true),
			};
			T["impact_detector"] = {
				Number("threshold", "Trigger above", "g", 10, 0, 100000, 0.1),
				Number("pulse", "Signal length", "s", 0.1, 0, 1000, 0.01),
			};
			T["light_panel"] = {
				ColorDef("color", "Light color", Color(1.0f, 1.0f, 1.0f)),
				Number("brightness", "Brightness", "", 1, 0, 100, 0.1),
			};
			T["vanish_block"] = {FlagDef("returns", "Comes back when the signal ends", false)};

			// --- Combat ---
			T["explosive_block"] = {Number("blast_radius", "Blast radius", "m", 2, 0, 1000, 0.05)};
			T["machine_gun"] = {
				Number("fire_rate", "Fire rate", "rounds/s", 10, 0, 1000, 0.1),
				Number("range", "Range", "m", 500, 1, 10000, 1),
				FlagDef("unlimited_ammo", "Unlimited ammo", true),
			};
			T["hitscan_gun"] = {
				Number("fire_rate", "Fire rate", "rounds/s", 10, 0, 1000, 0.1),
				Number("range", "Range", "m", 500, 1, 10000, 1),
				FlagDef("unlimited_ammo", "Unlimited ammo", true),
				Number("cone", "Aiming cone (full angle)", "\xC2\xB0", 60, 1, 360, 1),
				Choice("trigger", "Fires on", {"Left mouse button", "Right mouse button", "Fire input only"}),
			};
			std::vector<std::string> MaterialNames;
			for (const MaterialInfo& M : Materials::All()) { MaterialNames.push_back(M.Name); }
			SettingDef Projectile = Choice("projectile_material", "Projectile material", MaterialNames);
			Projectile.Default = PropValue::MakeChoice("Steel");
			T["large_cannon"] = {Projectile, FlagDef("unlimited_ammo", "Unlimited ammo", true)};
			T["ammo_generator"] = {
				Choice("ammo_type", "Ammo type", {"Machine gun rounds", "Cannon shells"}),
				Number("rate", "Made per second", "", 5, 0, 10000, 0.1),
				Number("storage", "Total storage", "", 200, 0, 10000000, 1),
			};

			// --- Misc ---
			T["magnet"] = {
				Number("strength", "Holding force", "N", 50000, 0, 1000000000, 100),
				FlagDef("on_unlinked", "On with no input", false),
				FlagDef("grabs_ground", "Grabs the ground and buildings too", false),
			};
			T["winch"] = {
				Number("start_length", "Starting rope length", "m", 4, 0.5, 500, 0.1),
				Number("max_length", "Longest rope", "m", 40, 0.5, 500, 0.1),
				Number("reel_speed", "Reel speed", "m/s", 2, 0, 100, 0.1),
				Number("strength", "Breaking tension", "N", 100000, 0, 1000000000, 100),
				FlagDef("grabs_unlinked", "Hook grabs with no input", true),
			};
			T["coupler"] = {
				FlagDef("couples_unlinked", "Couples with no input", true),
				Choice("fuel", "Fuel through the coupling", {"None", "Give fuel", "Take fuel"}),
				Number("fuel_rate", "Fuel flow", "L/s", 5, 0, 10000, 0.1),
			};
			T["dynamic_offsetter"] = {
				Number("shift_x", "Shift X", "blocks", 0, -5, 5, 0.005),
				Number("shift_y", "Shift Y", "blocks", 0, -5, 5, 0.005),
				Number("shift_z", "Shift Z", "blocks", 0, -5, 5, 0.005),
				Number("turn_x", "Turn about X", "\xC2\xB0", 0, -180, 180, 1),
				Number("turn_y", "Turn about Y", "\xC2\xB0", 0, -180, 180, 1),
				Number("turn_z", "Turn about Z", "\xC2\xB0", 0, -180, 180, 1),
			};
			return T;
		}

		const std::map<std::string, std::vector<SettingDef>>& Table()
		{
			static const std::map<std::string, std::vector<SettingDef>> T = BuildTable();
			return T;
		}
	}

	namespace Settings
	{
		const std::vector<SettingDef>& ForType(const std::string& Type)
		{
			static const std::vector<SettingDef> None;
			const auto& T = SettingsDetail::Table();
			const auto Found = T.find(Type);
			return Found == T.end() ? None : Found->second;
		}

		Props Defaults(const std::string& Type)
		{
			Props P;
			for (const SettingDef& S : ForType(Type)) { P[S.Id] = S.Default; }
			return P;
		}

		const SettingDef* Find(const std::string& Type, const std::string& Id)
		{
			for (const SettingDef& S : ForType(Type))
			{
				if (S.Id == Id) { return &S; }
			}
			return nullptr;
		}

		PropValue Clean(const SettingDef& Setting, const PropValue& Value)
		{
			switch (Setting.Kind)
			{
			case SettingKind::Number:
				if (Value.Kind != SettingKind::Number || !std::isfinite(Value.Number)) { return Setting.Default; }
				return PropValue::MakeNumber(Clamp(Value.Number, Setting.Min, Setting.Max));
			case SettingKind::Flag:
				return Value.Kind == SettingKind::Flag ? Value : Setting.Default;
			case SettingKind::Choice:
				if (Value.Kind == SettingKind::Choice || Value.Kind == SettingKind::Text)
				{
					for (const std::string& Option : Setting.Options)
					{
						if (Option == Value.Text) { return PropValue::MakeChoice(Value.Text); }
					}
				}
				return Setting.Default;
			case SettingKind::Color:
				if (Value.Kind == SettingKind::Color) { return Value; }
				if (Value.Kind == SettingKind::Text || Value.Kind == SettingKind::Choice)
				{
					Color Parsed;
					if (Materials::ColorFromHtml(Value.Text, Parsed)) { return PropValue::MakeColor(Parsed); }
				}
				return Setting.Default;
			case SettingKind::Text:
				if (Value.Kind == SettingKind::Text || Value.Kind == SettingKind::Choice)
				{
					return PropValue::MakeText(Value.Text.substr(0, 24));
				}
				return Setting.Default;
			}
			return Setting.Default;
		}

		double Num(const Props& P, const std::string& Id, double Fallback)
		{
			const auto Found = P.find(Id);
			if (Found == P.end()) { return Fallback; }
			if (Found->second.Kind == SettingKind::Number) { return Found->second.Number; }
			if (Found->second.Kind == SettingKind::Flag) { return Found->second.Flag ? 1.0 : 0.0; }
			return Fallback;
		}

		bool Flag(const Props& P, const std::string& Id, bool Fallback)
		{
			const auto Found = P.find(Id);
			if (Found == P.end()) { return Fallback; }
			if (Found->second.Kind == SettingKind::Flag) { return Found->second.Flag; }
			if (Found->second.Kind == SettingKind::Number) { return Found->second.Number != 0.0; }
			return Fallback;
		}

		std::string Text(const Props& P, const std::string& Id, const std::string& Fallback)
		{
			const auto Found = P.find(Id);
			if (Found == P.end()) { return Fallback; }
			if (Found->second.Kind == SettingKind::Choice || Found->second.Kind == SettingKind::Text) { return Found->second.Text; }
			return Fallback;
		}

		IW::Color ColorOf(const Props& P, const std::string& Id, const IW::Color& Fallback)
		{
			const auto Found = P.find(Id);
			if (Found == P.end()) { return Fallback; }
			if (Found->second.Kind == SettingKind::Color) { return Found->second.ColorValue; }
			IW::Color Parsed;
			if (Materials::ColorFromHtml(Found->second.Text, Parsed)) { return Parsed; }
			return Fallback;
		}
	}

	// --- Ports ----------------------------------------------------------------

	namespace PortsDetail
	{
		PortDef P(const char* Id, const char* Name, PortKind Kind)
		{
			PortDef D;
			D.Id = Id; D.Name = Name; D.Kind = Kind;
			return D;
		}

		struct PortPair
		{
			std::vector<PortDef> Inputs;
			std::vector<PortDef> Outputs;
		};

		std::map<std::string, PortPair> BuildPorts()
		{
			const PortKind Bo = PortKind::Bool;
			const PortKind Nu = PortKind::Number;
			const PortKind Am = PortKind::Ammo;
			const std::vector<PortDef> SpeedOut = {P("speed", "Speed", Nu)};
			const std::vector<PortDef> ServoIn = {
				P("go_positive", "Go to positive target", Bo), P("go_negative", "Go to negative target", Bo), P("angle", "Angle", Nu)};
			const std::vector<PortDef> AngleOut = {P("angle", "Angle", Nu)};
			const std::vector<PortDef> GunIn = {P("fire", "Fire", Bo), P("ammo", "Ammo supply", Am)};
			std::map<std::string, PortPair> M;
			// Mechanical
			M["motor"] = {{P("clockwise", "Clockwise", Bo), P("counterclockwise", "Counterclockwise", Bo), P("speed", "Speed", Nu)},
				{P("shaft_speed", "Shaft speed", Nu)}};
			M["servo"] = {ServoIn, AngleOut};
			M["door_hinge_servo"] = {ServoIn, AngleOut};
			M["hinge"] = {{}, AngleOut};
			M["bearing"] = {{}, SpeedOut};
			M["piston"] = {{P("extend", "Go to target length", Bo), P("length", "Length", Nu)},
				{P("length", "Length", Nu), P("pressure", "Pressure", Nu)}};
			M["solenoid"] = {{P("on", "On", Bo)}, {P("state", "State", Bo)}};
			M["spring"] = {{}, {P("length", "Length", Nu)}};
			M["gear"] = {{}, SpeedOut};
			M["bevel_gear"] = {{}, SpeedOut};
			M["rack"] = {{}, {P("position", "Position", Nu)}};
			M["pulley"] = {{}, SpeedOut};
			M["crankshaft"] = {{}, {P("angle", "Angle", Nu), P("speed", "Speed", Nu),
				P("position", "Rod end position (m)", Nu), P("near_top", "Near the top", Bo)}};
			M["wheel"] = {{}, SpeedOut};
			M["clutch"] = {{P("engage", "Engage", Bo), P("amount", "Amount", Nu)}, {}};
			M["brake"] = {{P("brake", "Brake", Bo), P("amount", "Amount", Nu)}, {}};
			// Fluid and engine
			M["fluid_cylinder"] = {{}, {P("pressure", "Pressure", Nu), P("length", "Length", Nu)}};
			M["pump"] = {{P("on", "On", Bo), P("rate", "Rate", Nu)}, {P("flow", "Flow", Nu)}};
			M["valve"] = {{P("open", "Open", Bo), P("amount", "Amount", Nu)}, {}};
			M["fuel_tank"] = {{}, {P("level", "Level", Nu)}};
			M["pressure_tank"] = {{}, {P("pressure", "Pressure", Nu)}};
			M["combustion_cylinder"] = {{P("spark", "Spark", Bo)}, {P("pressure", "Pressure", Nu)}};
			M["compressor"] = {{}, {P("shaft_speed", "Shaft speed", Nu)}};
			M["combustor"] = {{P("fuel_rate", "Fuel rate", Nu)}, {}};
			M["turbine"] = {{}, {P("shaft_speed", "Shaft speed", Nu)}};
			M["nozzle"] = {{}, {P("thrust", "Thrust", Nu)}};
			M["thruster"] = {{P("on", "On", Bo), P("throttle", "Throttle", Nu)}, {P("thrust", "Thrust", Nu)}};
			M["solid_rocket"] = {{P("ignite", "Ignite", Bo)}, {P("thrust", "Thrust", Nu)}};
			// Aero
			M["propeller"] = {{}, SpeedOut};
			M["lift_gas_tank"] = {{P("amount", "Amount (0 to 1)", Nu)}, {}};
			// Logic
			M["input_block"] = {{}, {P("pressed", "Pressed", Bo), P("value", "Value", Nu)}};
			M["mouse_click"] = {{}, {P("pressed", "Pressed", Bo), P("x", "Cursor X (-1 to 1)", Nu), P("y", "Cursor Y (-1 to 1)", Nu)}};
			M["boolean_gate"] = {{P("a", "A", Bo), P("b", "B", Bo)}, {P("result", "Result", Bo)}};
			M["number_gate"] = {{P("a", "A", Nu), P("b", "B", Nu), P("c", "C", Nu)},
				{P("result", "Result", Nu), P("is_true", "Comparison is true", Bo)}};
			M["pid_controller"] = {{P("setpoint", "Setpoint", Nu), P("measurement", "Measurement", Nu)}, {P("output", "Output", Nu)}};
			M["pwm_generator"] = {{P("enable", "Enable", Bo), P("duty", "Duty (0 to 1)", Nu), P("frequency", "Frequency (Hz)", Nu)},
				{P("output", "Output", Bo), P("phase", "Phase (0 to 1)", Nu)}};
			M["speed_sensor"] = {{}, SpeedOut};
			M["altitude_sensor"] = {{}, {P("altitude", "Altitude", Nu)}};
			M["attitude_sensor"] = {{}, {P("pitch", "Pitch", Nu), P("roll", "Roll", Nu), P("yaw", "Yaw", Nu),
				P("pitch_rate", "Pitch rate", Nu), P("roll_rate", "Roll rate", Nu), P("yaw_rate", "Yaw rate", Nu)}};
			M["distance_sensor"] = {{}, {P("distance", "Distance", Nu)}};
			M["instrument"] = {{P("value", "Value (Value dial)", Nu)}, {}};
			M["graph_display"] = {{P("a", "Line A", Nu), P("b", "Line B", Nu), P("c", "Line C", Nu), P("d", "Line D", Nu)}, {}};
			M["impact_detector"] = {{}, {P("triggered", "Triggered", Bo), P("acceleration", "Acceleration", Nu)}};
			M["light_panel"] = {{P("on", "On", Bo), P("red", "Red (0 to 255)", Nu), P("green", "Green (0 to 255)", Nu),
				P("blue", "Blue (0 to 255)", Nu)}, {}};
			M["vanish_block"] = {{P("vanish", "Vanish", Bo)}, {}};
			// Combat
			M["explosive_block"] = {{P("detonate", "Detonate", Bo)}, {}};
			M["machine_gun"] = {GunIn, {}};
			M["hitscan_gun"] = {GunIn, {P("firing", "Firing", Bo), P("hit_distance", "Distance to aim point", Nu)}};
			M["large_cannon"] = {GunIn, {}};
			M["ammo_generator"] = {{}, {P("ammo", "Ammo supply", Am), P("stored", "Ammo stored", Nu)}};
			// Misc
			M["seat"] = {{}, {P("occupied", "Occupied", Bo)}};
			M["magnet"] = {{P("on", "On", Bo)}, {P("holding", "Holding", Bo)}};
			M["winch"] = {{P("reel", "Reel (+1 in, -1 out)", Nu), P("grab", "Hook grabs", Bo)},
				{P("length", "Rope length (m)", Nu), P("tension", "Tension (N)", Nu), P("holding", "Hook holding", Bo)}};
			M["coupler"] = {{P("couple", "Couple", Bo)}, {P("coupled", "Coupled", Bo), P("fuel_moved", "Fuel moved (L)", Nu)}};
			return M;
		}

		const std::map<std::string, PortPair>& Table()
		{
			static const std::map<std::string, PortPair> M = BuildPorts();
			return M;
		}
	}

	namespace Ports
	{
		const std::vector<PortDef>& InputsOf(const std::string& Type)
		{
			static const std::vector<PortDef> None;
			const auto& M = PortsDetail::Table();
			const auto Found = M.find(Type);
			return Found == M.end() ? None : Found->second.Inputs;
		}

		const std::vector<PortDef>& OutputsOf(const std::string& Type)
		{
			static const std::vector<PortDef> None;
			const auto& M = PortsDetail::Table();
			const auto Found = M.find(Type);
			return Found == M.end() ? None : Found->second.Outputs;
		}

		const PortDef* Find(const std::vector<PortDef>& List, const std::string& Id)
		{
			for (const PortDef& Port : List)
			{
				if (Port.Id == Id) { return &Port; }
			}
			return nullptr;
		}

		std::string Label(const PortDef& Port)
		{
			const char* KindName = Port.Kind == PortKind::Bool ? "on/off" : (Port.Kind == PortKind::Ammo ? "ammo" : "number");
			return Port.Name + " (" + KindName + ")";
		}

		bool Compatible(const PortDef& Input, const PortDef& Output)
		{
			return (Input.Kind == PortKind::Ammo) == (Output.Kind == PortKind::Ammo);
		}

		std::vector<PortDef> OutputsFor(const PortDef& Input, const std::string& SourceType)
		{
			std::vector<PortDef> Fitting;
			for (const PortDef& Output : OutputsOf(SourceType))
			{
				if (Compatible(Input, Output)) { Fitting.push_back(Output); }
			}
			return Fitting;
		}
	}
}
