#pragma once

#include "Core/Coroutine/Task.h"

#include <chrono>
#include <optional>
#include <filesystem>
#include <string>
namespace LeoEngine
{
	class ShaderDB {
	public:
		static std::optional<std::filesystem::file_time_type> QueryTime(const std::string& key);

		static leo::coroutine::Task<void> UpdateTime(const std::string& key, std::filesystem::file_time_type time);
	};
}