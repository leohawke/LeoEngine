#pragma once

#include "../Render/Shader.h"
#include <compare>

namespace platform::Render {
	inline namespace Shader
	{
		

		struct ShaderCompilerInput
		{
			ShaderType Type;
			std::string_view Code;
			std::string_view EntryPoint;
			std::string_view SourceName;

			struct RootParameterBinding
			{
				/** Name of the constant buffer stored parameter. */
				std::string Name;

				/** Type expected in the shader code to ensure the binding is bug free. */
				std::string ExpectedShaderType;

				/** The offset of the parameter in the root shader parameter struct. */
				uint16 ByteOffset;

				friend auto operator<=>(const RootParameterBinding& lhs, const RootParameterBinding& rhs)
				{
					return lhs.ByteOffset <=> rhs.ByteOffset;
				}
			};

			FShaderCompilerEnvironment Environment;

			std::vector<RootParameterBinding> RootParameterBindings;
		};
	}
}

namespace D3DFlags {
	enum COMPILER_FLAGS
	{
		D3DCOMPILE_DEBUG = (1 << 0),
		D3DCOMPILE_SKIP_VALIDATION = (1 << 1),
		D3DCOMPILE_SKIP_OPTIMIZATION = (1 << 2),
		D3DCOMPILE_PACK_MATRIX_ROW_MAJOR = (1 << 3),
		D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR = (1 << 4),
		D3DCOMPILE_PARTIAL_PRECISION = (1 << 5),
		D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT = (1 << 6),
		D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT = (1 << 7),
		D3DCOMPILE_NO_PRESHADER = (1 << 8),
		D3DCOMPILE_AVOID_FLOW_CONTROL = (1 << 9),
		D3DCOMPILE_PREFER_FLOW_CONTROL = (1 << 10),
		D3DCOMPILE_ENABLE_STRICTNESS = (1 << 11),
		D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY = (1 << 12),
		D3DCOMPILE_IEEE_STRICTNESS = (1 << 13),
		D3DCOMPILE_OPTIMIZATION_LEVEL0 = (1 << 14),
		D3DCOMPILE_OPTIMIZATION_LEVEL1 = 0,
		D3DCOMPILE_OPTIMIZATION_LEVEL2 = ((1 << 14) | (1 << 15)),
		D3DCOMPILE_OPTIMIZATION_LEVEL3 = (1 << 15),
		D3DCOMPILE_RESERVED16 = (1 << 16),
		D3DCOMPILE_RESERVED17 = (1 << 17),
		D3DCOMPILE_WARNINGS_ARE_ERRORS = (1 << 18),
		D3DCOMPILE_RESOURCES_MAY_ALIAS = (1 << 19),
		D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES = (1 << 20),
		D3DCOMPILE_ALL_RESOURCES_BOUND = (1 << 21),
	};

	enum COMPILER_STRIP_FLAGS
	{
		D3DCOMPILER_STRIP_REFLECTION_DATA = 0x00000001,
		D3DCOMPILER_STRIP_DEBUG_INFO = 0x00000002,
		D3DCOMPILER_STRIP_TEST_BLOBS = 0x00000004,
		D3DCOMPILER_STRIP_PRIVATE_DATA = 0x00000008,
		D3DCOMPILER_STRIP_ROOT_SIGNATURE = 0x00000010,
		D3DCOMPILER_STRIP_FORCE_DWORD = 0x7fffffff,
	};
}

namespace asset::X::Shader
{
	using namespace platform::Render::Shader;

	inline namespace DXBC
	{
		ShaderBlob CompileToDXBC(const ShaderCompilerInput& input,
			 leo::uint32 flags);
		void ReflectDXBC(const ShaderBlob& blob, ShaderType type,ShaderInfo* pInfo);
		ShaderBlob StripDXBC(const ShaderBlob& blob, leo::uint32 flags);
	}

	inline namespace DXIL
	{
		ShaderBlob CompileAndReflectDXIL(const ShaderCompilerInput& input,
			leo::uint32 flags,ShaderInfo* pInfo=nullptr);

		std::string PreprocessShader(const std::string& code, const ShaderCompilerInput& input);
	}

	ShaderBlob CompileAndReflect(const ShaderCompilerInput& input, 
		leo::uint32 flags, ShaderInfo* pInfo = nullptr);

	ShaderBlob Strip(const ShaderBlob& blob, ShaderType type, leo::uint32 flags);

	void AppendCompilerEnvironment(FShaderCompilerEnvironment& environment, ShaderType type);
	std::string_view CompileProfile(ShaderType type);
}