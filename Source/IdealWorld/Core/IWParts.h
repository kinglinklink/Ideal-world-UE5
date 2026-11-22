// Ideal World - engine-free core.
//
// The part catalogue (part_library.gd), every part's settings
// (part_settings.gd) and every part's signal inputs and outputs
// (part_ports.gd).

#pragma once

#include "IWMath.h"

#include <map>
#include <string>
#include <vector>

namespace IW
{
	enum class CollisionKind : uint8_t { Box, Convex, Concave };

	struct PartType
	{
		std::string Id;
		std::string Name;
		std::string Category;
		Vec3 DefaultSizeCells;       // along the part's own axes
		double VolumeFraction = 1.0; // of its bounding box
		Vec3 UnitCentroid;           // centre of mass in unit-box coordinates (-0.5 .. 0.5)
		CollisionKind Collision = CollisionKind::Box;
		bool HasDefaultColor = false;
		Color DefaultColor;
	};

	namespace Parts
	{
		const std::vector<std::string>& Categories();
		const std::vector<PartType>& All();
		bool Has(const std::string& Id);
		/** The part type with this id, or the block. */
		const PartType& Get(const std::string& Id);
		std::string Name(const std::string& Id);
		/** Colour a newly placed part gets: its own default, or the brush colour. */
		Color PlacementColor(const std::string& Id, const Color& BrushColor);
		bool IsAeroPanel(const std::string& Id);
		bool IsTrianglePanel(const std::string& Id);
		bool IsDragless(const std::string& Id);
		/** Aero panels sit on the middle of the cell under the cursor. */
		bool SitsMidCell(const std::string& Id);
		/** Captions for a part's three dimensions in the Config tool. */
		std::vector<std::string> SizeCaptions(const std::string& Id);
		/** Control parts weigh nothing and vanish once spawned (unless set visible). */
		bool IsControlPart(const std::string& Id);

		constexpr double RingInnerRadius = 0.3;
		constexpr double PipeInnerRadius = 0.4;
		constexpr double ArchRadius = 0.4;
		constexpr int CurveSteps = 10;
	}

	// --- Settings ---------------------------------------------------------------

	enum class SettingKind : uint8_t { Number, Flag, Choice, Color, Text };

	/** One setting's value. Which field is used depends on the setting's kind. */
	struct PropValue
	{
		SettingKind Kind = SettingKind::Number;
		double Number = 0.0;
		bool Flag = false;
		std::string Text;            // choice or text
		IW::Color ColorValue;

		static PropValue MakeNumber(double V) { PropValue P; P.Kind = SettingKind::Number; P.Number = V; return P; }
		static PropValue MakeFlag(bool V) { PropValue P; P.Kind = SettingKind::Flag; P.Flag = V; return P; }
		static PropValue MakeChoice(const std::string& V) { PropValue P; P.Kind = SettingKind::Choice; P.Text = V; return P; }
		static PropValue MakeText(const std::string& V) { PropValue P; P.Kind = SettingKind::Text; P.Text = V; return P; }
		static PropValue MakeColor(const IW::Color& V) { PropValue P; P.Kind = SettingKind::Color; P.ColorValue = V; return P; }

		bool operator==(const PropValue& O) const
		{
			return Kind == O.Kind && Number == O.Number && Flag == O.Flag && Text == O.Text && ColorValue == O.ColorValue;
		}
		bool operator!=(const PropValue& O) const { return !(*this == O); }
	};

	using Props = std::map<std::string, PropValue>;

	struct SettingDef
	{
		std::string Id;
		std::string Name;
		SettingKind Kind = SettingKind::Number;
		std::string Unit;
		double Min = 0.0, Max = 0.0, Step = 1.0;
		std::vector<std::string> Options;
		PropValue Default;
	};

	namespace Settings
	{
		/** The settings for one kind of part, in display order. Empty for plain shapes. */
		const std::vector<SettingDef>& ForType(const std::string& Type);
		Props Defaults(const std::string& Type);
		const SettingDef* Find(const std::string& Type, const std::string& Id);
		/** Forces one value into the form its setting allows. */
		PropValue Clean(const SettingDef& Setting, const PropValue& Value);

		// Reading props with defaults, the way the Godot code's props.get(id, d) does.
		double Num(const Props& P, const std::string& Id, double Fallback);
		bool Flag(const Props& P, const std::string& Id, bool Fallback);
		std::string Text(const Props& P, const std::string& Id, const std::string& Fallback);
		IW::Color ColorOf(const Props& P, const std::string& Id, const IW::Color& Fallback);
	}

	// --- Signal ports -----------------------------------------------------------

	enum class PortKind : uint8_t { Bool, Number, Ammo };

	struct PortDef
	{
		std::string Id;
		std::string Name;
		PortKind Kind = PortKind::Number;
	};

	namespace Ports
	{
		const std::vector<PortDef>& InputsOf(const std::string& Type);
		const std::vector<PortDef>& OutputsOf(const std::string& Type);
		const PortDef* Find(const std::vector<PortDef>& List, const std::string& Id);
		/** "Shaft speed (number)" */
		std::string Label(const PortDef& Port);
		/** Any signal can feed any signal input. Ammo only connects to ammo. */
		bool Compatible(const PortDef& Input, const PortDef& Output);
		std::vector<PortDef> OutputsFor(const PortDef& Input, const std::string& SourceType);
	}
}
