#pragma once

#include "ShaderCore.h"

#define PR_NAMESPACE_BEGIN  namespace platform::Render {
#define PR_NAMESPACE_END }

PR_NAMESPACE_BEGIN
class HardwareShader;
class VertexHWShader;
class PixelHWShader;
class GeometryHWShader;

inline namespace Shader
{
	using leo::int32;
	using leo::uint16;

	class BuiltInShaderMeta;
	class ShaderMeta;
	class RenderShader;
	class ShaderParametersMetadata;

	/** Define a shader permutation uniquely according to its type, and permutation id.*/
	template<typename MetaShaderType>
	struct ShaderTypePermutation
	{
		MetaShaderType* const Type;
		const int32 PermutationId;

		ShaderTypePermutation(MetaShaderType* InType, int32 InPermutationId)
			: Type(InType)
			, PermutationId(InPermutationId)
		{
		}

		bool operator==(const ShaderTypePermutation& Other)const
		{
			return Type == Other.Type && PermutationId == Other.PermutationId;
		}

		bool operator!=(const ShaderTypePermutation& Other)const
		{
			return !(*this == Other);
		}
	};

	using ShaderPermutation = ShaderTypePermutation<ShaderMeta>;

	//shader parameter bindings and their offset and size in the shader's parameters struct.
	class RenderShaderParameterBindings
	{
	public:
		struct Parameter
		{
			uint16 BufferIndex;
			uint16 BaseIndex;
			//shader's parameters struct
			uint16 ByteOffset;
			uint16 ByteSize;
		};

		struct ResourceParameter
		{
			uint16 BaseIndex;
			//shader's parameters struct
			uint16 ByteOffset;
		};

		std::vector<Parameter> Paramters;
		std::vector<ResourceParameter> Textures;
		std::vector<ResourceParameter> Samplers;

		void BindForLegacyShaderParameters(const RenderShader* Shader, const ShaderParameterMap& ParameterMaps, const ShaderParametersMetadata& StructMetaData);
	};

	
	class RenderShader
	{
	public:
		struct CompiledShaderInitializer
		{
			HardwareShader* Shader;
		};

		RenderShader();

		virtual ~RenderShader();

		RenderShader(const CompiledShaderInitializer& initializer);

		VertexHWShader* GetVertexShader() const
		{
			return GetHardwareShader<VertexHWShader>();
		}
		GeometryHWShader* GetGeometryShader() const
		{
			return GetHardwareShader<GeometryHWShader>();
		}
		PixelHWShader* GetPixelShader() const
		{
			return GetHardwareShader<PixelHWShader>();
		}
	private:
		template<class THardwareShader>
		THardwareShader* GetHardwareShader() const
		{
			return (THardwareShader*)(Shader.get());
		}
	public:
		std::unique_ptr<HardwareShader> Shader;
		RenderShaderParameterBindings Bindings;
	};

	class BuiltInShaderMeta;

	class ShaderMeta
	{
	public:
		enum class EShaderMetaForDownCast :uint32
		{
			Reserve,
			BuitlIn,
		};

		typedef class RenderShader* (*ConstructType)();

		static std::list<ShaderMeta*>& GetTypeList();
	public:
		ShaderMeta(
			EShaderMetaForDownCast InShaderMetaForDownCast,
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

		BuiltInShaderMeta* GetBuiltInShaderType()
		{
			return ShaderMetaForDownCast == EShaderMetaForDownCast::BuitlIn ? (BuiltInShaderMeta*)(this) : nullptr;
		}
	private:
		EShaderMetaForDownCast ShaderMetaForDownCast;

		std::string TypeName;
		std::string SourceFileName;
		std::string EntryPoint;
		platform::Render::ShaderType Frequency;

		ConstructType ConstructRef;
	};

	template <typename ParameterStruct>
	inline void BindForLegacyShaderParameters(RenderShader* Shader, const ShaderParameterMap& ParameterMap)
	{
		Shader->Bindings.BindForLegacyShaderParameters(Shader, ParameterMap, *FParameterStruct::TypeInfo::GetStructMetadata());
	}

	template<>
	inline void BindForLegacyShaderParameters<void>(RenderShader* Shader, const ShaderParameterMap& ParameterMap)
	{
	}

	template<class>
	struct ShaderParametersType
	{
		static constexpr bool HasParameters = false;
		using type = void;
	};

	template<class ShaderClass>
		requires requires{typename ShaderClass::Parameters; }
	struct ShaderParametersType<ShaderClass>
	{
		static constexpr bool HasParameters = true;

		using type = typename ShaderClass::Parameters;
	};

	template<class ShaderClass>
	using ShaderParametersType_t = typename ShaderParametersType<ShaderClass>::type;

#define EXPORTED_SHADER_TYPE(ShaderClass,ShaderMetaTypeShortcut) \
public:\
	using ShaderMetaType = platform::Render::##ShaderMetaTypeShortcut##ShaderMeta;\
	static ShaderMetaType StaticType; \
	static RenderShader* ConstructInstance() { return new ShaderClass();} \
	static constexpr bool HasParameters =  platform::Render::ShaderParametersType<ShaderClass>::HasParameters;\
	ShaderClass() \
	{\
		platform::Render::BindForLegacyShaderParameters<platform::Render::ShaderParametersType_t<ShaderClass>>(this,{});\
	}

#define IMPLEMENT_SHADER(ShaderClass,SourceFileName,FunctionName,Frequency) \
	ShaderClass::ShaderMetaType ShaderClass::StaticType( \
		ShaderClass::ShaderMetaType::EShaderMetaForDownCast::Reserve,\
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

}

class ShaderInitializer
{
public:
	const platform::Render::ShaderBlob* pBlob;
	const platform::Render::ShaderInfo* pInfo;
};

PR_NAMESPACE_END

#undef PR_NAMESPACE_BEGIN
#undef PR_NAMESPACE_END
