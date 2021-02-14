#pragma once

#include "BuiltInShader.h"

#define PR_NAMESPACE_BEGIN  namespace platform::Render {
#define PR_NAMESPACE_END }

PR_NAMESPACE_BEGIN
class RayTracingShader;

inline namespace Shader
{
	bool IsRayTracingShader(platform::Render::ShaderType type);


	class BuiltInRayTracingShader :public BuiltInShader
	{
	public:
		using DerivedType = BuiltInRayTracingShader;
		static constexpr bool RootParameterStruct = true;
	public:
		BuiltInRayTracingShader(const ShaderMetaType::CompiledShaderInitializer& Initializer)
			:BuiltInShader(Initializer)
		{}

		BuiltInRayTracingShader(){}

		RayTracingShader* GetRayTracingShader();
	private:
	};
}

#define EXPORTED_RAYTRACING_SHADER(ShaderClass) EXPORTED_SHADER_TYPE(ShaderClass,BuiltIn) \
	static inline const ShaderParametersMetadata* GetRootParametersMetadata() {return platform::Render::ParametersMetadata<ShaderClass>();}

PR_NAMESPACE_END

#undef PR_NAMESPACE_BEGIN
#undef PR_NAMESPACE_END