#pragma once
#ifndef LE_RENDER_ShaderCore_h
#define LE_RENDER_ShaderCore_h 1

#include <LBase/linttype.hpp>
#include <LBase/lmemory.hpp>

namespace platform::Render {

	inline namespace Shader
	{
		struct ShaderCodeResourceCounts
		{
			leo::uint16 NumSamplers = 0;
			leo::uint16 NumSRVs = 0;
			leo::uint16 NumUAVs = 0;
			leo::uint16 NumCBs;
		};

		enum ShaderType : leo::uint8
		{
			VertexShader,
			PixelShader,
			HullShader,
			DomainShader,
			GeometryShader,
			VisibilityAll = 5,
			ComputeShader = 5,

			NumStandardType = 6,

			RayGen = 6,
			RayMiss,
			RayHitGroup,
			RayCallable
		};


		using ShaderBlob = std::pair<std::unique_ptr<stdex::byte[]>, std::size_t>;
	}

}

#endif