#include "ShaderDB.h"
#include "Core/Path.h"
#define SQLITE_ORM_OMITS_CODECVT 1
#include <sqlite/sqlite_orm.h>
#include <mutex>

using namespace LeoEngine;
using namespace sqlite_orm;
namespace fs = std::filesystem;

struct FileTime
{
	std::string FileName;
	long long TimePoint;
};

const fs::path& StoragePath() {
	auto static path = (PathSet::EngineIntermediateDir() / "Shaders.db");
	return path;
}

decltype(auto) Storage()
{
	static auto& storage = []() -> decltype(auto) {
		auto path = StoragePath().string();

		static auto internal_storage = make_storage(path,
			make_table("filetimes",
				make_column("filename", &FileTime::FileName, primary_key()),
				make_column("filetime", &FileTime::TimePoint)
			)
		);

		internal_storage.sync_schema();

		return (internal_storage);
	}();

	return storage;
}

std::optional<std::filesystem::file_time_type> LeoEngine::ShaderDB::QueryTime(const std::filesystem::path& path)
{
	if (!fs::exists(StoragePath()))
		return std::nullopt;

	auto& storage = Storage();

	auto filetime = storage.get_optional<FileTime>(path.string());

	if (filetime)
		return std::filesystem::file_time_type(std::chrono::file_clock::duration(filetime->TimePoint));
	return std::nullopt;
}