#pragma once

#include "Render/ICommandList.h"

namespace LeoEngine
{
	namespace lr = platform::Render;

	class ProjectedShadowInfo
	{

	};

	void SetupShadowDepthPass(const ProjectedShadowInfo& ShadowInfo, lr::CommandList& CmdList);
}