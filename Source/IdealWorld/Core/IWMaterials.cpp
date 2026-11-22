#include "IWMaterials.h"

#include <cstdio>

namespace IW
{
	namespace Materials
	{
		const char* DefaultId = "plastic";

		namespace MaterialsDetail
		{
			MaterialInfo Make(const char* Id, const char* Name, double Density, double Friction, double Strength,
				Color C, float Metallic, float Roughness, double MPa)
			{
				MaterialInfo M;
				M.Id = Id;
				M.Name = Name;
				M.Density = Density;
				M.Friction = Friction;
				M.Strength = Strength;
				M.DefaultColor = C;
				M.Metallic = Metallic;
				M.Roughness = Roughness;
				M.StrengthMPa = MPa;
				M.Clear = (M.Id == "glass" || M.Id == "acrylic" || M.Id == "diamond");
				return M;
			}

			std::vector<MaterialInfo> Build()
			{
				std::vector<MaterialInfo> L;
				L.push_back(Make("foam", "Foam", 50, 0.5, 4, Color(0.93f, 0.90f, 0.78f), 0.0f, 0.95f, 1));
				L.push_back(Make("balsa", "Balsa", 150, 0.4, 8, Color(0.87f, 0.76f, 0.58f), 0.0f, 0.9f, 10));
				L.push_back(Make("cork", "Cork", 240, 0.7, 5, Color(0.70f, 0.55f, 0.38f), 0.0f, 0.95f, 2));
				L.push_back(Make("wood", "Wood", 600, 0.4, 10, Color(0.62f, 0.44f, 0.27f), 0.0f, 0.85f, 40));
				L.push_back(Make("oak", "Oak", 750, 0.45, 11, Color(0.50f, 0.34f, 0.20f), 0.0f, 0.8f, 50));
				L.push_back(Make("ice", "Ice", 917, 0.05, 1, Color(0.72f, 0.88f, 0.96f), 0.0f, 0.1f, 1));
				L.push_back(Make("plastic", "Plastic", 1050, 0.35, 6, Color(0.80f, 0.82f, 0.86f), 0.0f, 0.6f, 40));
				L.push_back(Make("rubber", "Rubber", 1100, 1.1, 8, Color(0.14f, 0.14f, 0.15f), 0.0f, 0.95f, 15));
				L.push_back(Make("nylon", "Nylon", 1150, 0.25, 9, Color(0.90f, 0.89f, 0.84f), 0.0f, 0.5f, 75));
				L.push_back(Make("acrylic", "Acrylic", 1180, 0.4, 5, Color(0.75f, 0.90f, 0.95f), 0.0f, 0.08f, 65));
				L.push_back(Make("kevlar", "Kevlar", 1440, 0.35, 45, Color(0.85f, 0.72f, 0.25f), 0.0f, 0.7f, 1500));
				L.push_back(Make("carbon_fiber", "Carbon fiber", 1600, 0.3, 40, Color(0.22f, 0.23f, 0.26f), 0.2f, 0.45f, 600));
				L.push_back(Make("magnesium", "Magnesium", 1740, 0.45, 14, Color(0.74f, 0.75f, 0.77f), 0.75f, 0.45f, 200));
				L.push_back(Make("fiberglass", "Fiberglass", 1900, 0.35, 20, Color(0.82f, 0.86f, 0.78f), 0.0f, 0.55f, 300));
				L.push_back(Make("brick", "Brick", 1900, 0.7, 2, Color(0.66f, 0.30f, 0.22f), 0.0f, 1.0f, 10));
				L.push_back(Make("concrete", "Concrete", 2400, 0.8, 2, Color(0.62f, 0.62f, 0.60f), 0.0f, 1.0f, 30));
				L.push_back(Make("glass", "Glass", 2500, 0.4, 1, Color(0.70f, 0.86f, 0.90f), 0.0f, 0.05f, 50));
				L.push_back(Make("aluminum", "Aluminum", 2700, 0.6, 15, Color(0.80f, 0.82f, 0.84f), 0.8f, 0.35f, 270));
				L.push_back(Make("granite", "Granite", 2700, 0.65, 3, Color(0.50f, 0.48f, 0.50f), 0.0f, 0.85f, 130));
				L.push_back(Make("diamond", "Diamond", 3510, 0.1, 30, Color(0.80f, 0.95f, 1.00f), 0.1f, 0.02f, 2800));
				L.push_back(Make("titanium", "Titanium", 4500, 0.5, 25, Color(0.60f, 0.61f, 0.64f), 0.8f, 0.4f, 880));
				L.push_back(Make("cast_iron", "Cast iron", 7200, 0.5, 5, Color(0.26f, 0.27f, 0.29f), 0.6f, 0.75f, 200));
				L.push_back(Make("steel", "Steel", 7850, 0.6, 10, Color(0.48f, 0.50f, 0.54f), 0.85f, 0.4f, 400));
				L.push_back(Make("brass", "Brass", 8500, 0.4, 7, Color(0.80f, 0.65f, 0.30f), 0.9f, 0.3f, 300));
				L.push_back(Make("copper", "Copper", 8960, 0.5, 6, Color(0.80f, 0.47f, 0.30f), 0.9f, 0.3f, 210));
				L.push_back(Make("silver", "Silver", 10490, 0.45, 3, Color(0.88f, 0.88f, 0.90f), 0.95f, 0.2f, 140));
				L.push_back(Make("lead", "Lead", 11340, 0.5, 1, Color(0.36f, 0.37f, 0.42f), 0.7f, 0.6f, 15));
				L.push_back(Make("tungsten", "Tungsten", 19250, 0.4, 8, Color(0.40f, 0.41f, 0.43f), 0.85f, 0.45f, 980));
				L.push_back(Make("gold", "Gold", 19300, 0.45, 1, Color(0.95f, 0.77f, 0.25f), 1.0f, 0.2f, 120));
				// Not a real material: an ideal surface for experiments. Nothing grips
				// it, nothing moves it and nothing damages it. Once spawned, anything
				// made of it, and anything welded to it, is fixed in place.
				MaterialInfo Slick = Make("frictionless", "Frictionless surface", 1000, 0.0, 0.0,
					Color(0.90f, 0.96f, 1.00f), 0.3f, 0.0f, 0);
				Slick.Immovable = true;
				L.push_back(Slick);
				return L;
			}
		}

		const std::vector<MaterialInfo>& All()
		{
			static const std::vector<MaterialInfo> List = MaterialsDetail::Build();
			return List;
		}

		const MaterialInfo& Info(const std::string& Id)
		{
			const std::vector<MaterialInfo>& List = All();
			for (const MaterialInfo& M : List)
			{
				if (M.Id == Id) { return M; }
			}
			for (const MaterialInfo& M : List)
			{
				if (M.Id == DefaultId) { return M; }
			}
			return List.front();
		}

		bool Has(const std::string& Id)
		{
			for (const MaterialInfo& M : All())
			{
				if (M.Id == Id) { return true; }
			}
			return false;
		}

		std::string IdForName(const std::string& ShownName)
		{
			for (const MaterialInfo& M : All())
			{
				if (M.Name == ShownName) { return M.Id; }
			}
			return DefaultId;
		}

		std::string HealthText(const std::string& Id, double Mass)
		{
			const MaterialInfo& M = Info(Id);
			if (M.Immovable) { return "infinite"; }
			char Buffer[64];
			std::snprintf(Buffer, sizeof(Buffer), "%d HP", (int)std::lround(Mass * M.Strength));
			return Buffer;
		}

		std::string Tooltip(const std::string& Id)
		{
			const MaterialInfo& M = Info(Id);
			if (M.Immovable)
			{
				return M.Name + "\nFriction: none, against anything\nCannot be moved or damaged once spawned";
			}
			char Buffer[256];
			std::snprintf(Buffer, sizeof(Buffer), "%s\nDensity: %d kg/m\xC2\xB3\nFriction: %.2f\nStrength: %d HP per kg",
				M.Name.c_str(), (int)M.Density, M.Friction, (int)M.Strength);
			return Buffer;
		}

		std::string ColorToHtml(const Color& C)
		{
			auto Byte = [](float V) { return (int)std::lround(Clamp((double)V, 0.0, 1.0) * 255.0); };
			char Buffer[16];
			std::snprintf(Buffer, sizeof(Buffer), "#%02x%02x%02x", Byte(C.R), Byte(C.G), Byte(C.B));
			return Buffer;
		}

		bool ColorFromHtml(const std::string& Text, Color& Out)
		{
			std::string Hex = Text;
			if (!Hex.empty() && Hex[0] == '#') { Hex = Hex.substr(1); }
			auto Digit = [](char Ch, int& Value)
			{
				if (Ch >= '0' && Ch <= '9') { Value = Ch - '0'; return true; }
				if (Ch >= 'a' && Ch <= 'f') { Value = Ch - 'a' + 10; return true; }
				if (Ch >= 'A' && Ch <= 'F') { Value = Ch - 'A' + 10; return true; }
				return false;
			};
			std::vector<int> Values;
			for (char Ch : Hex)
			{
				int Value = 0;
				if (!Digit(Ch, Value)) { return false; }
				Values.push_back(Value);
			}
			if (Values.size() == 3 || Values.size() == 4)
			{
				Out = Color(Values[0] / 15.0f, Values[1] / 15.0f, Values[2] / 15.0f, Values.size() == 4 ? Values[3] / 15.0f : 1.0f);
				return true;
			}
			if (Values.size() == 6 || Values.size() == 8)
			{
				auto Pair = [&Values](size_t At) { return (Values[At] * 16 + Values[At + 1]) / 255.0f; };
				Out = Color(Pair(0), Pair(2), Pair(4), Values.size() == 8 ? Pair(6) : 1.0f);
				return true;
			}
			return false;
		}
	}
}
