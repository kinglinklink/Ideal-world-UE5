#include "IWSaveFormat.h"

#include "IWMaterials.h"

#include <cctype>
#include <map>

namespace IW
{
	namespace SaveFormat
	{
		const char* Format = "ideal-world-build";
		const char* ModuleFormat = "ideal-world-module";

		namespace SaveDetail
		{
			JsonValue Numbers(double A, double B, double C)
			{
				JsonValue List = JsonValue::MakeArray();
				List.Push(JsonValue::MakeNumber(A));
				List.Push(JsonValue::MakeNumber(B));
				List.Push(JsonValue::MakeNumber(C));
				return List;
			}

			JsonValue InBlocks(const IVec3& U)
			{
				const double PerBlock = (double)Units::UnitsPerCell;
				return Numbers(U.X / PerBlock, U.Y / PerBlock, U.Z / PerBlock);
			}

			JsonValue Whole(const Vec3& Axis)
			{
				return Numbers(std::round(Axis.X), std::round(Axis.Y), std::round(Axis.Z));
			}

			bool ThreeNumbers(const JsonValue& Value, Vec3& Out)
			{
				if (!Value.IsArray() || Value.Size() != 3) { return false; }
				for (size_t Index = 0; Index < 3; ++Index)
				{
					if (!Value.At(Index).IsNumber()) { return false; }
				}
				Out = Vec3(Value.At(0).AsNumber(), Value.At(1).AsNumber(), Value.At(2).AsNumber());
				return true;
			}

			Basis QuarterTurns(int AxisIndex, double Degrees)
			{
				const int Quarters = (int)std::lround(Degrees / 90.0);
				return QuarterTurn(AxisIndex, Quarters);
			}

			/**
			 * A part's rotation, written either as "rotation": [x, y, z] in steps
			 * of 90 degrees (about world X, then Y, then Z, like R, T, Y), or as
			 * "x_axis", "y_axis", "z_axis". Anything else means "not rotated".
			 */
			Basis ReadRotation(const JsonValue& Entry)
			{
				Vec3 Turns;
				if (ThreeNumbers(Entry.Get("rotation"), Turns))
				{
					const Basis Combined = QuarterTurns(2, Turns.Z) * QuarterTurns(1, Turns.Y) * QuarterTurns(0, Turns.X);
					return Combined.Rounded();
				}
				Vec3 AxisX, AxisY, AxisZ;
				if (!ThreeNumbers(Entry.Get("x_axis"), AxisX) || !ThreeNumbers(Entry.Get("y_axis"), AxisY)
					|| !ThreeNumbers(Entry.Get("z_axis"), AxisZ))
				{
					return Basis();
				}
				const Basis Rot(AxisX.Round(), AxisY.Round(), AxisZ.Round());
				if (std::fabs(Rot.Determinant() - 1.0) > 0.01 || !Rot.IsNearlyEqual(Rot.Orthonormalized(), 1e-4))
				{
					return Basis();
				}
				return Rot;
			}

			JsonValue PropToJson(const SettingDef* Def, const PropValue& Value)
			{
				switch (Value.Kind)
				{
				case SettingKind::Number: return JsonValue::MakeNumber(Value.Number);
				case SettingKind::Flag: return JsonValue::MakeBool(Value.Flag);
				case SettingKind::Color: return JsonValue::MakeString(Materials::ColorToHtml(Value.ColorValue));
				default: break;
				}
				(void)Def;
				return JsonValue::MakeString(Value.Text);
			}

			PropValue JsonToProp(const JsonValue& Value)
			{
				if (Value.IsNumber()) { return PropValue::MakeNumber(Value.AsNumber()); }
				if (Value.IsBool()) { return PropValue::MakeFlag(Value.AsBool()); }
				if (Value.IsString()) { return PropValue::MakeText(Value.AsString()); }
				PropValue Bad;
				Bad.Kind = SettingKind::Number;
				Bad.Number = std::nan("");
				return Bad;
			}

			/** Settings as read from a file: missing ones get defaults, unknown ones are dropped. */
			Props Sanitize(const std::string& Type, const JsonValue& Raw)
			{
				Props Values;
				for (const SettingDef& Setting : Settings::ForType(Type))
				{
					if (Raw.IsObject() && Raw.Has(Setting.Id))
					{
						Values[Setting.Id] = Settings::Clean(Setting, JsonToProp(Raw.Get(Setting.Id)));
					}
					else
					{
						Values[Setting.Id] = Setting.Default;
					}
				}
				return Values;
			}

			JsonValue LinksToJson(const IW::Links& L, const std::map<int, int>& IndexOf)
			{
				JsonValue Out = JsonValue::MakeObject();
				for (const auto& Pair : L)
				{
					const auto Found = IndexOf.find(Pair.second.Uid);
					if (Found == IndexOf.end()) { continue; }
					JsonValue Link = JsonValue::MakeObject();
					Link.Set("part", JsonValue::MakeNumber(Found->second));
					Link.Set("output", JsonValue::MakeString(Pair.second.Output));
					Out.Set(Pair.first, Link);
				}
				return Out;
			}
		}

		std::string CleanName(const std::string& Typed)
		{
			std::string Cleaned;
			const std::string Forbidden = "\\/:*?\"<>|";
			size_t Start = 0;
			size_t End = Typed.size();
			while (Start < End && std::isspace((unsigned char)Typed[Start])) { ++Start; }
			while (End > Start && std::isspace((unsigned char)Typed[End - 1])) { --End; }
			for (size_t Index = Start; Index < End; ++Index)
			{
				const char Ch = Typed[Index];
				Cleaned += Forbidden.find(Ch) != std::string::npos ? '_' : Ch;
			}
			std::string Lower = Cleaned;
			for (char& Ch : Lower) { Ch = (char)std::tolower((unsigned char)Ch); }
			if (Lower.size() >= 5 && Lower.compare(Lower.size() - 5, 5, ".json") == 0)
			{
				Cleaned = Cleaned.substr(0, Cleaned.size() - 5);
			}
			while (!Cleaned.empty() && std::isspace((unsigned char)Cleaned.back())) { Cleaned.pop_back(); }
			return Cleaned;
		}

		JsonValue RecordToEntry(const PartRecord& R)
		{
			JsonValue Entry = JsonValue::MakeObject();
			Entry.Set("type", JsonValue::MakeString(R.Type));
			Entry.Set("pos", SaveDetail::InBlocks(R.Pos));
			Entry.Set("size", SaveDetail::InBlocks(R.Size));
			Entry.Set("x_axis", SaveDetail::Whole(R.Rot.Col[0]));
			Entry.Set("y_axis", SaveDetail::Whole(R.Rot.Col[1]));
			Entry.Set("z_axis", SaveDetail::Whole(R.Rot.Col[2]));
			Entry.Set("material", JsonValue::MakeString(R.Material));
			Entry.Set("color", JsonValue::MakeString(Materials::ColorToHtml(R.PartColor)));
			if (!R.Props.empty())
			{
				JsonValue SettingsJson = JsonValue::MakeObject();
				for (const auto& Pair : R.Props)
				{
					SettingsJson.Set(Pair.first, SaveDetail::PropToJson(Settings::Find(R.Type, Pair.first), Pair.second));
				}
				Entry.Set("settings", SettingsJson);
			}
			return Entry;
		}

		bool EntryToRecord(const JsonValue& Entry, PartRecord& Out)
		{
			if (!Entry.IsObject()) { return false; }
			const std::string Type = Entry.Get("type").ToText();
			if (!Parts::Has(Type)) { return false; }
			Vec3 PosBlocks;
			if (!SaveDetail::ThreeNumbers(Entry.Get("pos"), PosBlocks)) { return false; }
			const double PerBlock = (double)Units::UnitsPerCell;
			const IVec3 Pos = IVec3::FromRounded(PosBlocks * PerBlock);

			IVec3 Size = Records::DefaultSizeUnits(Type);
			Vec3 SizeBlocks;
			if (SaveDetail::ThreeNumbers(Entry.Get("size"), SizeBlocks))
			{
				Size = IVec3::FromRounded(SizeBlocks * PerBlock);
				for (int Axis = 0; Axis < 3; ++Axis) { Size[Axis] = ClampInt(Size[Axis], 1, GridSize::MaxSizeUnits); }
			}

			std::string Material = Entry.Get("material").ToText(Materials::DefaultId);
			if (!Materials::Has(Material)) { Material = Materials::DefaultId; }

			IW::Color C = Parts::PlacementColor(Type, Materials::Info(Material).DefaultColor);
			const std::string ColorText = Entry.Get("color").ToText();
			IW::Color Parsed;
			if (!ColorText.empty() && Materials::ColorFromHtml(ColorText, Parsed)) { C = Parsed; }

			Out = Records::Make(Type, Pos, SaveDetail::ReadRotation(Entry), Size, Material, C);
			Out.Props = SaveDetail::Sanitize(Type, Entry.Get("settings"));
			return true;
		}

		std::string SaveBuild(const BuildModel& Model, const std::string& Name)
		{
			std::map<int, int> IndexOf;   // uid -> position in the file's parts list
			const std::vector<int> Uids = Model.AllUids();
			for (size_t Index = 0; Index < Uids.size(); ++Index) { IndexOf[Uids[Index]] = (int)Index; }
			JsonValue Entries = JsonValue::MakeArray();
			for (int Uid : Uids)
			{
				const PartRecord R = Model.Get(Uid);
				JsonValue Entry = RecordToEntry(R);
				const JsonValue LinksJson = SaveDetail::LinksToJson(R.Links, IndexOf);
				if (LinksJson.Size() > 0) { Entry.Set("links", LinksJson); }
				Entries.Push(Entry);
			}
			JsonValue Data = JsonValue::MakeObject();
			Data.Set("format", JsonValue::MakeString(Format));
			Data.Set("version", JsonValue::MakeNumber(Version));
			Data.Set("name", JsonValue::MakeString(Name));
			Data.Set("block_size_m", JsonValue::MakeNumber(Units::Cell));
			Data.Set("parts", Entries);
			return Data.Write("\t");
		}

		LoadResult LoadBuild(BuildModel& Model, const std::string& Text, const std::string& Name)
		{
			LoadResult Result;
			JsonValue Data;
			if (!JsonValue::Parse(Text, Data) || !Data.IsObject())
			{
				Result.Error = "\"" + Name + "\" is not a readable build file.";
				return Result;
			}
			const JsonValue& Entries = Data.Get("parts");
			if (Data.Get("format").ToText() != Format || !Entries.IsArray())
			{
				Result.Error = "\"" + Name + "\" is not an Ideal World build file.";
				return Result;
			}

			struct Pending
			{
				int Position;
				PartRecord Record;
				const JsonValue* WrittenLinks;
			};
			std::vector<Pending> List;
			int Skipped = 0;
			for (size_t Position = 0; Position < Entries.Size(); ++Position)
			{
				const JsonValue& Entry = Entries.At(Position);
				PartRecord R;
				if (EntryToRecord(Entry, R))
				{
					List.push_back({(int)Position, R, &Entry.Get("links")});
				}
				else
				{
					++Skipped;
				}
			}

			Model.BeginBatch();
			Model.Clear();
			int Loaded = 0;
			std::map<int, int> UidAt;   // position in the file -> the uid that part was given
			for (const Pending& Item : List)
			{
				const int Uid = Model.AddPart(Item.Record);
				if (Uid == 0) { ++Skipped; }   // outside the grid, or overlapping a different material
				else
				{
					++Loaded;
					UidAt[Item.Position] = Uid;
				}
			}
			// With every part in place, connect the links the file describes.
			for (const Pending& Item : List)
			{
				const auto Placed = UidAt.find(Item.Position);
				if (Placed == UidAt.end() || !Item.WrittenLinks->IsObject()) { continue; }
				const int Uid = Placed->second;
				const PartRecord Own = Model.Get(Uid);
				const std::vector<PortDef>& Inputs = Ports::InputsOf(Own.Type);
				IW::Links NewLinks;
				for (const std::string& InputId : Item.WrittenLinks->GetKeys())
				{
					const PortDef* InputPort = Ports::Find(Inputs, InputId);
					const JsonValue& LinkJson = Item.WrittenLinks->Get(InputId);
					if (!InputPort || !LinkJson.IsObject() || !LinkJson.Get("part").IsNumber()) { continue; }
					const auto Source = UidAt.find((int)LinkJson.Get("part").AsNumber());
					if (Source == UidAt.end()) { continue; }
					const PartRecord SourceRecord = Model.Get(Source->second);
					const std::string OutputId = LinkJson.Get("output").ToText();
					const PortDef* OutputPort = Ports::Find(Ports::OutputsOf(SourceRecord.Type), OutputId);
					if (!OutputPort || !Ports::Compatible(*InputPort, *OutputPort)) { continue; }
					Link L;
					L.Uid = Source->second;
					L.Output = OutputId;
					NewLinks[InputId] = L;
				}
				if (!NewLinks.empty()) { Model.SetLinks(Uid, NewLinks); }
			}
			Model.EndBatch();
			Result.Loaded = Loaded;
			Result.Skipped = Skipped;
			return Result;
		}

		std::string SaveModule(const std::vector<PartRecord>& RecordList, const std::string& Name)
		{
			std::map<int, int> IndexOf;
			for (size_t Index = 0; Index < RecordList.size(); ++Index) { IndexOf[RecordList[Index].CopiedFrom] = (int)Index; }
			JsonValue Entries = JsonValue::MakeArray();
			for (const PartRecord& R : RecordList)
			{
				JsonValue Entry = RecordToEntry(R);
				const JsonValue LinksJson = SaveDetail::LinksToJson(R.Links, IndexOf);
				if (LinksJson.Size() > 0) { Entry.Set("links", LinksJson); }
				Entries.Push(Entry);
			}
			JsonValue Data = JsonValue::MakeObject();
			Data.Set("format", JsonValue::MakeString(ModuleFormat));
			Data.Set("version", JsonValue::MakeNumber(Version));
			Data.Set("name", JsonValue::MakeString(Name));
			Data.Set("block_size_m", JsonValue::MakeNumber(Units::Cell));
			Data.Set("parts", Entries);
			return Data.Write("\t");
		}

		std::vector<PartRecord> LoadModule(const std::string& Text)
		{
			std::vector<PartRecord> Out;
			JsonValue Data;
			if (!JsonValue::Parse(Text, Data) || !Data.Get("parts").IsArray()) { return Out; }
			const JsonValue& Entries = Data.Get("parts");
			for (size_t Index = 0; Index < Entries.Size(); ++Index)
			{
				const JsonValue& Entry = Entries.At(Index);
				PartRecord R;
				if (!EntryToRecord(Entry, R)) { continue; }
				R.CopiedFrom = ModuleLinkBase + (int)Index;
				const JsonValue& Written = Entry.Get("links");
				if (Written.IsObject())
				{
					for (const std::string& InputId : Written.GetKeys())
					{
						const JsonValue& LinkJson = Written.Get(InputId);
						if (!LinkJson.IsObject() || !LinkJson.Get("part").IsNumber()) { continue; }
						Link L;
						L.Uid = ModuleLinkBase + (int)LinkJson.Get("part").AsNumber();
						L.Output = LinkJson.Get("output").ToText();
						R.Links[InputId] = L;
					}
				}
				Out.push_back(R);
			}
			return Out;
		}
	}
}
