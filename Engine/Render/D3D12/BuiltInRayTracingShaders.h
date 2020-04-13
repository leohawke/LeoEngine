#pragma once

#include "../Shader.h"
#include <list>

namespace platform_ex::Windows::D3D12 {
	class ShaderMeta
	{
	public:
		typedef class Shader* (*ConstructType)();

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

		Shader* Construct() const;
	private:
		std::string TypeName;
		std::string SourceFileName;
		std::string EntryPoint;
		platform::Render::ShaderType Frequency;

		ConstructType ConstructRef;
	};

	class Shader
	{

	};

#define EXPORTED_SHADER_TYPE(ShaderClass) \
public:\
	using ShaderMetaType = ShaderMeta;\
	static ShaderMetaType StaticType; \
	static Shader* ConstructInstance() { return new ShaderClass();}

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
		std::vector<Shader*> SerializedShaders;
	public:
		template<typename ShaderType>
		ShaderType* GetShader() const
		{
			auto find_itr = Shaders.find(&ShaderType::StaticType);

			LAssert(find_itr != Shaders.end(),"Failed to find shader type");

			return static_cast<ShaderType*>(find_itr->second.get());
		}

		void AddShader(ShaderMeta* Type, Shader* Shader)
		{
			Shaders.emplace(Type, leo::share_raw(Shader));
		}

		bool IsEmpty() const
		{
			return Shaders.empty();
		}

	protected:
		std::unordered_map< ShaderMeta*, std::shared_ptr<Shader>> Shaders;
	};

	ShaderMap<ShaderMeta>* GetGlobalShaderMap();

	void CompileGlobalShaderMap();
	
	class RayTracingShader;

	class BuiltInRayTracingShader :public Shader
	{
	public:
		RayTracingShader* GetRayTracingShader();
		void SetRayTracingShader(RayTracingShader* pShader);
	private:
		std::shared_ptr<RayTracingShader> pRayTracingShader;
	};

	template<typename ShaderType>
	inline RayTracingShader* GetBuildInRayTracingShader()
	{
		auto ShaderMap = GetGlobalShaderMap();

		auto Shader = ShaderMap->GetShader<ShaderType>();

		auto RayTracingShader = Shader->GetRayTracingShader();

		return RayTracingShader;
	}

	class DefaultCHS :public BuiltInRayTracingShader
	{
		EXPORTED_SHADER_TYPE(DefaultCHS);
	};

	class DefaultMS :public BuiltInRayTracingShader
	{
		EXPORTED_SHADER_TYPE(DefaultMS);
	};

	class ShadowRG : public BuiltInRayTracingShader
	{
		EXPORTED_SHADER_TYPE(ShadowRG);
	};
}
