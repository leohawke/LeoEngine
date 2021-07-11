#include "ShaderDB.h"
#include "Core/Path.h"
#define SQLITE_ORM_OMITS_CODECVT 1
#include <sqlite/sqlite_orm.h>
#include <mutex>
#include "System/SystemEnvironment.h"
#include "spdlog/spdlog.h"

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
	static std::once_flag init_db;

	std::call_once(init_db, [] {
		if (!fs::exists(StoragePath()))
		{
			auto internal_storage = make_storage(StoragePath().string(),
				make_table("filetimes",
					make_column("filename", &FileTime::FileName, primary_key()),
					make_column("filetime", &FileTime::TimePoint)
				)
			);

			internal_storage.sync_schema();
		}
	});

	thread_local static auto& storage = []() -> decltype(auto) {
		auto path = StoragePath().string();

		thread_local static auto internal_storage = make_storage(path,
			make_table("filetimes",
				make_column("filename", &FileTime::FileName, primary_key()),
				make_column("filetime", &FileTime::TimePoint)
			)
		);

		return (internal_storage);
	}();

	return storage;
}

std::optional<std::filesystem::file_time_type> LeoEngine::ShaderDB::QueryTime(const std::string& path)
{
	auto& storage = Storage();

	try
	{
		auto filetime = storage.get_optional<FileTime>(path);

		if (filetime)
			return std::filesystem::file_time_type(std::chrono::file_clock::duration(filetime->TimePoint));
	}
	catch (std::system_error& error)
	{
		spdlog::debug("QueryTime sqlite3 {} ",error.what());
	}
	return std::nullopt;
}

leo::coroutine::Task<void> LeoEngine::ShaderDB::UpdateTime(const std::string& path, std::filesystem::file_time_type time)
{
	auto& storage = Storage();

	try
	{
		storage.replace(FileTime{ .FileName = path,.TimePoint = time.time_since_epoch().count() });
	}
	catch (std::system_error& error)
	{
		spdlog::debug("UpdateTime sqlite3 {}", error.what());
	}

	co_return;
}
