#pragma once
#ifndef LE_RENDER_ShaderCore_h
#define LE_RENDER_ShaderCore_h 1

#include <LBase/linttype.hpp>

namespace platform::Render {
	struct ShaderCodeResourceCounts
	{
		leo::uint16 NumSamplers = 0;
		leo::uint16 NumSRVs = 0;
		leo::uint16 NumUAVs = 0;
		leo::uint16 NumCBs;
	};
}

#endif