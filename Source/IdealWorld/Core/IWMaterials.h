// Ideal World - engine-free core.
//
// Building materials, lightest first. Densities are real. Friction and
// strength are starting values to tune in play. Health of a block = its mass
// x strength. Matches material_library.gd.

#pragma once

#include "IWMath.h"

#include <string>
#include <vector>

namespace IW
{
	struct MaterialInfo
	{
		std::string Id;
		std::string Name;
		double Density = 1000.0;     // kg/m^3
		double Friction = 0.5;       // coefficient
		double Strength = 5.0;       // HP per kg
		Color DefaultColor;
		float Metallic = 0.0f;
		float Roughness = 0.5f;
		bool Immovable = false;      // the frictionless ideal surface
		bool Clear = false;          // glass, acrylic and diamond: see-through
		double StrengthMPa = 0.0;    // real breaking stress, for the stress view
	};

	namespace Materials
	{
		extern const char* DefaultId;   // "plastic"

		const std::vector<MaterialInfo>& All();
		/** The material with this id, or the default material. */
		const MaterialInfo& Info(const std::string& Id);
		bool Has(const std::string& Id);
		std::string IdForName(const std::string& ShownName);
		std::string HealthText(const std::string& Id, double Mass);
		std::string Tooltip(const std::string& Id);
		/** "#rrggbb" in lower case, as Godot's to_html(false) with a hash. */
		std::string ColorToHtml(const Color& C);
		/** Reads "#rgb", "#rrggbb" or "#rrggbbaa" (hash optional). False if not a colour. */
		bool ColorFromHtml(const std::string& Text, Color& Out);
	}
}
