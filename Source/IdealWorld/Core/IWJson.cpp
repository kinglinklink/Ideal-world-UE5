#include "IWJson.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace IW
{
	namespace JsonDetail
	{
		const JsonValue& NullValue()
		{
			static const JsonValue Null;
			return Null;
		}

		void WriteString(std::string& Out, const std::string& S)
		{
			Out += '"';
			for (const char Ch : S)
			{
				switch (Ch)
				{
				case '"': Out += "\\\""; break;
				case '\\': Out += "\\\\"; break;
				case '\n': Out += "\\n"; break;
				case '\r': Out += "\\r"; break;
				case '\t': Out += "\\t"; break;
				case '\b': Out += "\\b"; break;
				case '\f': Out += "\\f"; break;
				default:
					if ((unsigned char)Ch < 0x20)
					{
						char Buffer[8];
						std::snprintf(Buffer, sizeof(Buffer), "\\u%04x", (unsigned)(unsigned char)Ch);
						Out += Buffer;
					}
					else
					{
						Out += Ch;
					}
				}
			}
			Out += '"';
		}

		std::string NumberText(double N)
		{
			if (!std::isfinite(N)) { return "0"; }
			if (std::fabs(N - std::round(N)) < 1e-12 && std::fabs(N) < 1e15)
			{
				char Buffer[32];
				std::snprintf(Buffer, sizeof(Buffer), "%lld", (long long)std::llround(N));
				return Buffer;
			}
			// Shortest text that reads back to the same double.
			char Buffer[40];
			for (int Digits = 6; Digits <= 17; ++Digits)
			{
				std::snprintf(Buffer, sizeof(Buffer), "%.*g", Digits, N);
				if (std::strtod(Buffer, nullptr) == N) { break; }
			}
			return Buffer;
		}

		struct Parser
		{
			const std::string& Text;
			size_t At = 0;
			bool Failed = false;

			explicit Parser(const std::string& InText) : Text(InText) {}

			void SkipSpace()
			{
				while (At < Text.size())
				{
					const char Ch = Text[At];
					if (Ch == ' ' || Ch == '\t' || Ch == '\n' || Ch == '\r') { ++At; }
					else { break; }
				}
			}

			bool Expect(const char* Word)
			{
				size_t Index = 0;
				while (Word[Index] != 0)
				{
					if (At + Index >= Text.size() || Text[At + Index] != Word[Index]) { return false; }
					++Index;
				}
				At += Index;
				return true;
			}

			static void AppendUtf8(std::string& Out, unsigned Code)
			{
				if (Code < 0x80) { Out += (char)Code; }
				else if (Code < 0x800) { Out += (char)(0xC0 | (Code >> 6)); Out += (char)(0x80 | (Code & 0x3F)); }
				else if (Code < 0x10000)
				{
					Out += (char)(0xE0 | (Code >> 12)); Out += (char)(0x80 | ((Code >> 6) & 0x3F)); Out += (char)(0x80 | (Code & 0x3F));
				}
				else
				{
					Out += (char)(0xF0 | (Code >> 18)); Out += (char)(0x80 | ((Code >> 12) & 0x3F));
					Out += (char)(0x80 | ((Code >> 6) & 0x3F)); Out += (char)(0x80 | (Code & 0x3F));
				}
			}

			bool ReadHex4(unsigned& Code)
			{
				if (At + 4 > Text.size()) { return false; }
				Code = 0;
				for (int Index = 0; Index < 4; ++Index)
				{
					const char Ch = Text[At + Index];
					Code <<= 4;
					if (Ch >= '0' && Ch <= '9') { Code |= (unsigned)(Ch - '0'); }
					else if (Ch >= 'a' && Ch <= 'f') { Code |= (unsigned)(Ch - 'a' + 10); }
					else if (Ch >= 'A' && Ch <= 'F') { Code |= (unsigned)(Ch - 'A' + 10); }
					else { return false; }
				}
				At += 4;
				return true;
			}

			bool ReadString(std::string& Out)
			{
				if (At >= Text.size() || Text[At] != '"') { return false; }
				++At;
				while (At < Text.size())
				{
					const char Ch = Text[At++];
					if (Ch == '"') { return true; }
					if (Ch != '\\') { Out += Ch; continue; }
					if (At >= Text.size()) { return false; }
					const char Esc = Text[At++];
					switch (Esc)
					{
					case '"': Out += '"'; break;
					case '\\': Out += '\\'; break;
					case '/': Out += '/'; break;
					case 'b': Out += '\b'; break;
					case 'f': Out += '\f'; break;
					case 'n': Out += '\n'; break;
					case 'r': Out += '\r'; break;
					case 't': Out += '\t'; break;
					case 'u':
					{
						unsigned Code = 0;
						if (!ReadHex4(Code)) { return false; }
						if (Code >= 0xD800 && Code <= 0xDBFF && At + 6 <= Text.size() && Text[At] == '\\' && Text[At + 1] == 'u')
						{
							At += 2;
							unsigned Low = 0;
							if (!ReadHex4(Low)) { return false; }
							Code = 0x10000 + ((Code - 0xD800) << 10) + (Low - 0xDC00);
						}
						AppendUtf8(Out, Code);
						break;
					}
					default: return false;
					}
				}
				return false;
			}

			bool ReadValue(JsonValue& Out, int Depth)
			{
				if (Depth > 200) { return false; }
				SkipSpace();
				if (At >= Text.size()) { return false; }
				const char Ch = Text[At];
				if (Ch == '{')
				{
					++At;
					Out = JsonValue::MakeObject();
					SkipSpace();
					if (At < Text.size() && Text[At] == '}') { ++At; return true; }
					while (true)
					{
						SkipSpace();
						std::string Key;
						if (!ReadString(Key)) { return false; }
						SkipSpace();
						if (At >= Text.size() || Text[At] != ':') { return false; }
						++At;
						JsonValue Item;
						if (!ReadValue(Item, Depth + 1)) { return false; }
						Out.Set(Key, Item);
						SkipSpace();
						if (At < Text.size() && Text[At] == ',') { ++At; continue; }
						if (At < Text.size() && Text[At] == '}') { ++At; return true; }
						return false;
					}
				}
				if (Ch == '[')
				{
					++At;
					Out = JsonValue::MakeArray();
					SkipSpace();
					if (At < Text.size() && Text[At] == ']') { ++At; return true; }
					while (true)
					{
						JsonValue Item;
						if (!ReadValue(Item, Depth + 1)) { return false; }
						Out.Push(Item);
						SkipSpace();
						if (At < Text.size() && Text[At] == ',') { ++At; continue; }
						if (At < Text.size() && Text[At] == ']') { ++At; return true; }
						return false;
					}
				}
				if (Ch == '"')
				{
					std::string S;
					if (!ReadString(S)) { return false; }
					Out = JsonValue::MakeString(S);
					return true;
				}
				if (Expect("true")) { Out = JsonValue::MakeBool(true); return true; }
				if (Expect("false")) { Out = JsonValue::MakeBool(false); return true; }
				if (Expect("null")) { Out = JsonValue(); return true; }
				// A number.
				const char* Start = Text.c_str() + At;
				char* End = nullptr;
				const double N = std::strtod(Start, &End);
				if (End == Start) { return false; }
				At += (size_t)(End - Start);
				Out = JsonValue::MakeNumber(N);
				return true;
			}
		};
	}

	std::string JsonValue::ToText(const std::string& Fallback) const
	{
		if (KindValue == Kind::String) { return StringValue; }
		if (KindValue == Kind::Number) { return JsonDetail::NumberText(NumberValue); }
		if (KindValue == Kind::Bool) { return BoolValue ? "true" : "false"; }
		return Fallback;
	}

	const JsonValue& JsonValue::At(size_t Index) const
	{
		if (KindValue != Kind::Array || Index >= Items.size()) { return JsonDetail::NullValue(); }
		return Items[Index];
	}

	bool JsonValue::Has(const std::string& Key) const
	{
		return KindValue == Kind::Object && KeyIndex.find(Key) != KeyIndex.end();
	}

	const JsonValue& JsonValue::Get(const std::string& Key) const
	{
		if (KindValue != Kind::Object) { return JsonDetail::NullValue(); }
		const auto Found = KeyIndex.find(Key);
		if (Found == KeyIndex.end()) { return JsonDetail::NullValue(); }
		return Items[Found->second];
	}

	void JsonValue::Set(const std::string& Key, const JsonValue& V)
	{
		if (KindValue != Kind::Object)
		{
			*this = MakeObject();
		}
		const auto Found = KeyIndex.find(Key);
		if (Found != KeyIndex.end())
		{
			Items[Found->second] = V;
			return;
		}
		KeyIndex[Key] = Items.size();
		Keys.push_back(Key);
		Items.push_back(V);
	}

	void JsonValue::WriteTo(std::string& Out, const std::string& Indent, int Depth) const
	{
		const bool Pretty = !Indent.empty();
		auto NewLine = [&Out, &Indent, Pretty](int Level)
		{
			if (!Pretty) { return; }
			Out += '\n';
			for (int Index = 0; Index < Level; ++Index) { Out += Indent; }
		};
		switch (KindValue)
		{
		case Kind::Null: Out += "null"; break;
		case Kind::Bool: Out += BoolValue ? "true" : "false"; break;
		case Kind::Number: Out += JsonDetail::NumberText(NumberValue); break;
		case Kind::String: JsonDetail::WriteString(Out, StringValue); break;
		case Kind::Array:
		{
			// Short arrays of numbers (positions, sizes, axes) stay on one line.
			bool Flat = Items.size() <= 4;
			for (const JsonValue& Item : Items)
			{
				if (!Item.IsNumber() && !Item.IsBool()) { Flat = false; }
			}
			Out += '[';
			for (size_t Index = 0; Index < Items.size(); ++Index)
			{
				if (Index > 0) { Out += Flat && Pretty ? ", " : ","; }
				if (!Flat) { NewLine(Depth + 1); }
				Items[Index].WriteTo(Out, Indent, Depth + 1);
			}
			if (!Flat && !Items.empty()) { NewLine(Depth); }
			Out += ']';
			break;
		}
		case Kind::Object:
		{
			Out += '{';
			for (size_t Index = 0; Index < Keys.size(); ++Index)
			{
				if (Index > 0) { Out += ','; }
				NewLine(Depth + 1);
				JsonDetail::WriteString(Out, Keys[Index]);
				Out += Pretty ? ": " : ":";
				Items[Index].WriteTo(Out, Indent, Depth + 1);
			}
			if (!Keys.empty()) { NewLine(Depth); }
			Out += '}';
			break;
		}
		}
	}

	std::string JsonValue::Write(const std::string& Indent) const
	{
		std::string Out;
		WriteTo(Out, Indent, 0);
		return Out;
	}

	bool JsonValue::Parse(const std::string& Text, JsonValue& Out)
	{
		JsonDetail::Parser Reader(Text);
		JsonValue Value;
		// Skip a UTF-8 byte order mark if a text editor added one.
		if (Text.size() >= 3 && (unsigned char)Text[0] == 0xEF && (unsigned char)Text[1] == 0xBB && (unsigned char)Text[2] == 0xBF)
		{
			Reader.At = 3;
		}
		if (!Reader.ReadValue(Value, 0))
		{
			Out = JsonValue();
			return false;
		}
		Reader.SkipSpace();
		if (Reader.At != Text.size())
		{
			Out = JsonValue();
			return false;
		}
		Out = Value;
		return true;
	}
}
