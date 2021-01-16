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
	public:
		using DerivedType = BuiltInShader;
		using ShaderMetaType = BuiltInShaderMeta;
	public:
		BuiltInShader(const ShaderMetaType::CompiledShaderInitializer& Initializer)
			:RenderShader(Initializer)
		{}
	};

	class BuiltInShaderMapContent :public ShaderMapContent
	{
		friend class BuiltInShaderMap;
		friend class BuiltInShaderMapSection;
	public:
		std::size_t GetSourceFileNameHash() const {
			return HashedSourceFilename;
		}

	private:
		BuiltInShaderMapContent(std::size_t InHashedSourceFilename)
			:ShaderMapContent()
			, HashedSourceFilename(InHashedSourceFilename)
		{}
	private:
		std::size_t HashedSourceFilename;
	};

	class BuiltInShaderMapSection
	{
	public:
		friend class BuiltInShaderMap;

	private:
		BuiltInShaderMapSection(std::size_t InHashedSourceFilename)
			:Content(InHashedSourceFilename)
		{}

		ShaderRef<RenderShader> GetShader(ShaderMeta* ShaderType, int32 PermutationId = 0) const;

		BuiltInShaderMapContent Content;
	};

	class BuiltInShaderMap
	{
	public:
		~BuiltInShaderMap();

		ShaderRef<RenderShader> GetShader(ShaderMeta* ShaderType, int32 PermutationId = 0) const;

		/** Finds the shader with the given type.  Asserts on failure. */
		template<typename ShaderType>
		ShaderType* GetShader(int32 PermutationId = 0) const
		{
			auto Shader = GetShader(&ShaderType::StaticType, PermutationId);
			LAssert(Shader != nullptr, leo::sfmt("Failed to find shader type %s in Platform %s", ShaderType::StaticType.GetName(), "PCD3D_SM5").c_str());
			return static_cast<ShaderType*>(Shader);
		}

		/** Finds the shader with the given type.  Asserts on failure. */
		template<typename ShaderType>
		ShaderType* GetShader(const typename ShaderType::FPermutationDomain& PermutationVector) const
		{
			return GetShader<ShaderType>(PermutationVector.ToDimensionValueId());
		}

		RenderShader* FindOrAddShader(const ShaderMeta* ShaderType, int32 PermutationId, RenderShader* Shader);

	private:
		std::unordered_map<std::size_t, BuiltInShaderMapSection*> SectionMap;
	};

	BuiltInShaderMap* GetBuiltInShaderMap();


#define EXPORTED_BUILTIN_SHADER(ShaderClass) \
public:\
	using ShaderMetaType = platform::Render::BuiltInShaderMeta;\
	using ShaderMapType = ShaderMap<ShaderMeta>;\
	static ShaderMetaType StaticType; \
	static RenderShader* ConstructInstance() { return new ShaderClass();} \
	static RenderShader* ConstructCompiledInstance(const ShaderMetaType::CompiledShaderInitializer& Initializer) { return new ShaderClass(Initializer);} \
	static constexpr bool HasParameters =  platform::Render::ShaderParametersType<ShaderClass>::HasParameters;\
	ShaderClass(const ShaderMetaType::CompiledShaderInitializer& Initializer) \
		:DerivedType(Initializer)\
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