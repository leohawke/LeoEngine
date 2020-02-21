#pragma once

#include "../Render/ShaderCore.h"

namespace asset::X::Shader
{
	using namespace platform::Render::Shader;

	inline namespace DXBC
	{
		ShaderBlob CompileToDXBC(ShaderType type, std::string_view Code,
			std::string_view entry_point, const std::vector<ShaderMacro>& macros,
			std::string_view profile, leo::uint32 flags, std::string_view SourceName);
		ShaderInfo* ReflectDXBC(const ShaderBlob& blob, ShaderType type);
		ShaderBlob StripDXBC(const ShaderBlob& blob, leo::uint32 flags);
	}

	inline namespace DXIL
	{
		ShaderBlob CompileToDXIL(ShaderType type, std::string_view Code,
			std::string_view entry_point, const std::vector<ShaderMacro>& macros,
			std::string_view profile, leo::uint32 flags, std::string_view SourceName);
		ShaderInfo* ReflectDXIL (const ShaderBlob& blob, ShaderType type);
	}

	ShaderBlob Compile(ShaderType type, std::string_view Code,
		std::string_view entry_point, const std::vector<ShaderMacro>& macros,
		std::string_view profile, leo::uint32 flags, std::string_view SourceName);
	ShaderInfo* Reflect(const ShaderBlob& blob, ShaderType type);
}