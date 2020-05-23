#pragma once

#include "Shader.h"

#define PR_NAMESPACE_BEGIN  namespace platform::Render {
#define PR_NAMESPACE_END }

PR_NAMESPACE_BEGIN
inline namespace Shader
{
	class BuiltInShaderMeta : public ShaderMeta
	{
	public:
		using CompiledShaderInitializer = RenderShader::CompiledShaderInitializer;

		using ShaderMeta::ShaderMeta;
	};

	class BuiltInShader :public RenderShader
	{
		EXPORTED_SHADER_TYPE(BuiltInShader,BuiltIn)
	public:
		BuiltInShader(const ShaderMetaType::CompiledShaderInitializer& Initializer)
			:RenderShader(Initializer)
		{}
	};

#define EXPORTED_BUILTIN_SHADER(ShaderClass) \
public:\
	using ShaderMetaType = platform::Render::BuiltInShaderMeta;\
	static ShaderMetaType StaticType; \
	static RenderShader* ConstructInstance() { return new ShaderClass();} \
	static RenderShader* ConstructCompiledInstance(const ShaderMetaType::CompiledShaderInitializer& Initializer) { return new ShaderClass(Initializer);} \
	static constexpr bool HasParameters =  platform::Render::ShaderParametersType<ShaderClass>::HasParameters;\
	ShaderClass() \
	{\
		platform::Render::BindForLegacyShaderParameters<platform::Render::ShaderParametersType_t<ShaderClass>>(this,{});\
	}\
	ShaderClass(const ShaderMetaType::CompiledShaderInitializer& Initializer) \
		:ShaderClass()\
	{\
	}
}
PR_NAMESPACE_END

#undef PR_NAMESPACE_BEGIN
#undef PR_NAMESPACE_END