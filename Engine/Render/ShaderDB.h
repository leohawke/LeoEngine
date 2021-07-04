#pragma once

#include <chrono>
#include <optional>
#include <filesystem>
namespace LeoEngine
{
	class ShaderDB {
	public:
		static std::optional<std::filesystem::file_time_type> QueryTime(const std::filesystem::path& path);
	};
}