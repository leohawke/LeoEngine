#include "Path.h"
#include <LFramework/Win32/LCLib/Mingw32.h>
using namespace LeoEngine;

struct PathSetStaticData
{
	path WorkingRoot;
	path EngineRoot;
	path EngineDirectory;

	PathSetStaticData()
	{
		WorkingRoot = platform_ex::Windows::FetchModuleFileName();
		WorkingRoot.remove_filename();

		EngineRoot = (WorkingRoot / ".." / "..").lexically_normal();

		EngineDirectory = EngineRoot / "Engine";
	}
} StaticData;

path LeoEngine::PathSet::WorkingRoot()
{
	return StaticData.WorkingRoot;
}

path LeoEngine::PathSet::EngineDir()
{
	return StaticData.EngineDirectory;

}

struct PathSetStaticDataEx
{
	PathSetStaticDataEx(const path& engineroot)
	{
		IntermediateDirectory = engineroot / "Intermediate";
		fs::create_directories(IntermediateDirectory);
	}

	path IntermediateDirectory;
};

PathSetStaticDataEx& GetStaticDataEx()
{
	static PathSetStaticDataEx StaticDataEx{ StaticData.EngineRoot };
	return StaticDataEx;
}

path LeoEngine::PathSet::EngineIntermediateDir()
{
	return GetStaticDataEx().IntermediateDirectory;
}
