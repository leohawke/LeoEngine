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
		typedef RenderShader* (*ConstructCompiledType)(const CompiledShaderInitializer&);

		BuiltInShaderMeta(
			const char* InName,
			const char* InSourceFileName,
			const char* InEntryPoint,
			platform::Render::ShaderType  InFrequency,
			ConstructType InConstructRef,
			ConstructCompiledType InConstructCompiledRef
		):
		 ShaderMeta(EShaderMetaForDownCast::BuitlIn,InName,InSourceFileName,InEntryPoint,InFrequency,InConstructRef),
			ConstructCompiledRef(InConstructCompiledRef)
		{}

		RenderShader* Construct(const CompiledShaderInitializer& initializer) const
		{
			return (*ConstructCompiledRef)(initializer);
		}

	private:
		ConstructCompiledType ConstructCompiledRef;
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
	ShaderClass(const ShaderMetaType::CompiledShaderInitializer& Initializer) \
	{\
		platform::Render::BindForLegacyShaderParameters<platform::Render::ShaderParametersType_t<ShaderClass>>(this,Initializer.ParameterMap);\
	}\
	ShaderClass() \
	{ }

#define IMPLEMENT_BUILTIN_SHADER(ShaderClass,SourceFileName,FunctionName,Frequency) \
	ShaderClass::ShaderMetaType ShaderClass::StaticType( \
		#ShaderClass, \
		SourceFileName, \
		FunctionName, \
		Frequency, \
		ShaderClass::ConstructInstance, \
		ShaderClass::ConstructCompiledInstance\
	)
}
PR_NAMESPACE_END

#undef PR_NAMESPACE_BEGIN
#undef PR_NAMESPACE_END