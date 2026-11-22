// Ideal World - engine-free core.
//
// Build files: plain-text JSON, the same format as the Godot version
// (save_manager.gd, saves/README.txt), so builds move freely between the two.
// The core only turns builds into text and back; the game layer reads and
// writes the files.

#pragma once

#include "IWBuildModel.h"
#include "IWJson.h"

#include <string>
#include <vector>

namespace IW
{
	namespace SaveFormat
	{
		extern const char* Format;         // "ideal-world-build"
		extern const char* ModuleFormat;   // "ideal-world-module"
		constexpr int Version = 1;
		constexpr int ModuleLinkBase = 1000000;   // stand-in uids for a module's own parts

		struct LoadResult
		{
			std::string Error;
			int Loaded = 0;
			int Skipped = 0;
		};

		/** Turns whatever was typed into a safe file name, without the extension. */
		std::string CleanName(const std::string& Typed);

		/** The whole build as file text. */
		std::string SaveBuild(const BuildModel& Model, const std::string& Name);
		/** Replaces the build with the one in Text. On an error the build is left untouched. */
		LoadResult LoadBuild(BuildModel& Model, const std::string& Text, const std::string& Name);

		/** Copied records (positions relative to an anchor, CopiedFrom set) as a module file. */
		std::string SaveModule(const std::vector<PartRecord>& Records, const std::string& Name);
		/** A module's records, ready to paste (CopiedFrom = ModuleLinkBase + index). */
		std::vector<PartRecord> LoadModule(const std::string& Text);

		/** One part entry from a file, or false if it is unusable. */
		bool EntryToRecord(const JsonValue& Entry, PartRecord& Out);
		JsonValue RecordToEntry(const PartRecord& R);
	}
}
