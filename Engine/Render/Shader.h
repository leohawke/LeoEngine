#pragma once

#include "ShaderCore.h"

#define PR_NAMESPACE_BEGIN  namespace platform::Render {
#define PR_NAMESPACE_END }

PR_NAMESPACE_BEGIN
inline namespace Shader
{
	class BuiltInShaderMeta;

	class ShaderMeta
	{
	public:
		typedef class RenderShader* (*ConstructType)();

		static std::list<ShaderMeta*>& GetTypeList();
	public:
		ShaderMeta(
			const char* InName,
			const char* InSourceFileName,
			const char* InEntryPoint,
			platform::Render::ShaderType  InFrequency,
			ConstructType InConstructRef
		);

		const std::string& GetTypeName() const { return TypeName; }
		const std::string& GetSourceFileName() const { return SourceFileName; }
		const std::string& GetEntryPoint() const { return EntryPoint; }
		platform::Render::ShaderType GetShaderType() const { return Frequency; }

		RenderShader* Construct() const;
	private:
		std::string TypeName;
		std::string SourceFileName;
		std::string EntryPoint;
		platform::Render::ShaderType Frequency;

		ConstructType ConstructRef;
	};

	class RenderShader
	{

	};

#define EXPORTED_SHADER_TYPE(ShaderClass) \
public:\
	using ShaderMetaType = ShaderMeta;\
	static ShaderMetaType StaticType; \
	static RenderShader* ConstructInstance() { return new ShaderClass();}

#define IMPLEMENT_SHADER(ShaderClass,SourceFileName,FunctionName,Frequency) \
	ShaderClass::ShaderMetaType ShaderClass::StaticType( \
		#ShaderClass, \
		SourceFileName, \
		FunctionName, \
		Frequency, \
		ShaderClass::ConstructInstance \
	)

	template<typename ShaderMetaType>
	class ShaderMap
	{
		std::vector<RenderShader*> SerializedShaders;
	public:
		template<typename ShaderType>
		ShaderType* GetShader() const
		{
			auto find_itr = Shaders.find(&ShaderType::StaticType);

			LAssert(find_itr != Shaders.end(), "Failed to find shader type");

			return static_cast<ShaderType*>(find_itr->second.get());
		}

		void AddShader(ShaderMeta* Type, RenderShader* Shader)
		{
			Shaders.emplace(Type, leo::share_raw(Shader));
		}

		bool IsEmpty() const
		{
			return Shaders.empty();
		}

	protected:
		std::unordered_map< ShaderMeta*, std::shared_ptr<RenderShader>> Shaders;
	};

	ShaderMap<ShaderMeta>* GetGlobalShaderMap();

	void CompileGlobalShaderMap();

	bool IsRayTracingShader(platform::Render::ShaderType type);

	class RayTracingShader;

	class BuiltInRayTracingShader :public RenderShader
	{
	public:
		RayTracingShader* GetRayTracingShader();
		void SetRayTracingShader(RayTracingShader* pShader);
	private:
		std::shared_ptr<RayTracingShader> pRayTracingShader;
	};
}
PR_NAMESPACE_END

#undef PR_NAMESPACE_BEGIN
#undef PR_NAMESPACE_END
