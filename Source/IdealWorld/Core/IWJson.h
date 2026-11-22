// Ideal World - engine-free core.
//
// A small JSON value, reader and writer for build files. Kept in the core
// (rather than using Unreal's Json module) so that loading and saving are
// unit-tested on their own, and so files stay byte-for-byte compatible with
// the Godot version. No exceptions: a parse error returns false.

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace IW
{
	class JsonValue
	{
	public:
		enum class Kind : uint8_t { Null, Bool, Number, String, Array, Object };

		JsonValue() = default;
		static JsonValue MakeBool(bool B) { JsonValue V; V.KindValue = Kind::Bool; V.BoolValue = B; return V; }
		static JsonValue MakeNumber(double N) { JsonValue V; V.KindValue = Kind::Number; V.NumberValue = N; return V; }
		static JsonValue MakeString(const std::string& S) { JsonValue V; V.KindValue = Kind::String; V.StringValue = S; return V; }
		static JsonValue MakeArray() { JsonValue V; V.KindValue = Kind::Array; return V; }
		static JsonValue MakeObject() { JsonValue V; V.KindValue = Kind::Object; return V; }

		Kind GetKind() const { return KindValue; }
		bool IsNull() const { return KindValue == Kind::Null; }
		bool IsBool() const { return KindValue == Kind::Bool; }
		bool IsNumber() const { return KindValue == Kind::Number; }
		bool IsString() const { return KindValue == Kind::String; }
		bool IsArray() const { return KindValue == Kind::Array; }
		bool IsObject() const { return KindValue == Kind::Object; }

		bool AsBool(bool Fallback = false) const { return KindValue == Kind::Bool ? BoolValue : Fallback; }
		double AsNumber(double Fallback = 0.0) const { return KindValue == Kind::Number ? NumberValue : Fallback; }
		const std::string& AsString() const { return StringValue; }
		/** Text of a string, or a number written as text, or Fallback. */
		std::string ToText(const std::string& Fallback = "") const;

		// Arrays
		size_t Size() const { return KindValue == Kind::Array ? Items.size() : (KindValue == Kind::Object ? Keys.size() : 0); }
		const JsonValue& At(size_t Index) const;
		void Push(const JsonValue& V) { Items.push_back(V); }
		const std::vector<JsonValue>& GetItems() const { return Items; }

		// Objects keep their keys in the order they were added, as Godot does.
		bool Has(const std::string& Key) const;
		const JsonValue& Get(const std::string& Key) const;
		void Set(const std::string& Key, const JsonValue& V);
		const std::vector<std::string>& GetKeys() const { return Keys; }

		/** Writes JSON. Indent "" writes one line; "\t" writes Godot-style tabs. */
		std::string Write(const std::string& Indent = "\t") const;
		/** Parses text. Returns false (and leaves Out null) on any error. */
		static bool Parse(const std::string& Text, JsonValue& Out);

	private:
		void WriteTo(std::string& Out, const std::string& Indent, int Depth) const;

		Kind KindValue = Kind::Null;
		bool BoolValue = false;
		double NumberValue = 0.0;
		std::string StringValue;
		std::vector<JsonValue> Items;
		std::vector<std::string> Keys;
		std::map<std::string, size_t> KeyIndex;   // key -> index into Items
	};
}
