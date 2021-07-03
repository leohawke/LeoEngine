#pragma once

#include <LBase/ldef.h>
#include <filesystem>

namespace LeoEngine
{
	namespace fs = std::filesystem;

	using path = fs::path;

	class PathSet
	{
	public:
		static path WorkingRoot();

		static path EngineDir();

		static path EngineIntermediateDir();
	};
}