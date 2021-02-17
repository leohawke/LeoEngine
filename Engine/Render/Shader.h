#pragma once

#include <LBase/smart_ptr.hpp>
#include "ShaderCore.h"
#include "ShaderPermutation.h"
#include <concepts>
#include <shared_mutex>

#define PR_NAMESPACE_BEGIN  namespace platform::Render {
#define PR_NAMESPACE_END }

PR_NAMESPACE_BEGIN
class HardwareShader;
class VertexHWShader;
class PixelHWShader;
class GeometryHWShader;
class ComputeHWShader;

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

		std::vector<Parameter> Parameters;
		//TODO:RaytracingAccelerationStructure
		std::vector<ResourceParameter> Textures;
		std::vector<ResourceParameter> Samplers;
		std::vector<ResourceParameter> UAVs;

		static constexpr uint16 kInvalidBufferIndex = 0xFFFF;

		uint16 RootParameterBufferIndex = kInvalidBufferIndex;



		void BindForLegacyShaderParameters(const RenderShader* Shader, const ShaderParameterMap& ParameterMaps, const ShaderParametersMetadata& StructMetaData);
		void BindForRootShaderParameters(const RenderShader* Shader, const ShaderParameterMap& ParameterMaps);
	};

	struct FShaderPermutationParameters
	{
		/** Unique permutation identifier of the material shader type. */
		const int32 PermutationId;

		explicit FShaderPermutationParameters(int32 InPermutationId = 0)
			:PermutationId(InPermutationId)
		{}
	};

	
	class RenderShader
	{
	public:
		using FPermutationDomain = FShaderPermutationNone;
		using FPermutationParameters = FShaderPermutationParameters;
		static constexpr bool RootParameterStruct = false;

		struct CompiledShaderInitializer
		{
			HardwareShader* Shader;
			ShaderParameterMap ParameterMap;
			ShaderMeta* Meta;
		};

		RenderShader();

		virtual ~RenderShader();

		RenderShader(const CompiledShaderInitializer& initializer);


		inline ShaderMeta* GetMeta() const { return Meta; }

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

		ComputeHWShader* GetComputeShader() const
		{
			return GetHardwareShader<ComputeHWShader>();
		}

		/** Returns the meta data for the root shader parameter struct. */
		static inline const ShaderParametersMetadata* GetRootParametersMetadata()
		{
			return nullptr;
		}

		/** Can be overridden by FShader subclasses to modify their compile environment just before compilation occurs. */
		static void ModifyCompilationEnvironment(const FShaderPermutationParameters&, FShaderCompilerEnvironment&) {}
		/** Can be overridden by FShader subclasses to determine whether a specific permutation should be compiled. */
		static bool ShouldCompilePermutation(const FShaderPermutationParameters&) { return true; }
	protected:
		template<class THardwareShader>
		THardwareShader* GetHardwareShader() const
		{
			return (THardwareShader*)(Shader.get());
		}
	public:
		std::unique_ptr<HardwareShader> Shader;
		RenderShaderParameterBindings Bindings;
		ShaderMeta* Meta;
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
		typedef void (*ModifyCompilationEnvironmentType)(const FShaderPermutationParameters&, FShaderCompilerEnvironment&);
		typedef bool (*ShouldCompilePermutationType)(const FShaderPermutationParameters&);


		static std::list<ShaderMeta*>& GetTypeList();
	public:
		ShaderMeta(
			EShaderMetaForDownCast InShaderMetaForDownCast,
			const char* InName,
			const char* InSourceFileName,
			const char* InEntryPoint,
			platform::Render::ShaderType  InFrequency,
			int32 TotalPermutationCount,
			ConstructType InConstructRef,//InConstructCompiledRef
			ModifyCompilationEnvironmentType InModifyCompilationEnvironmentRef,
			ShouldCompilePermutationType InShouldCompilePermutationRef,
			uint32 InTypeSize,
			const ShaderParametersMetadata* InRootParametersMetadata
		);

		const std::string& GetTypeName() const { return TypeName; }
		std::size_t GetHash() const { return Hash; }
		const std::string& GetSourceFileName() const { return SourceFileName; }
		std::size_t GetHashedShaderFilename() const { return HashedShaderFilename; }
		const std::string& GetEntryPoint() const { return EntryPoint; }
		platform::Render::ShaderType GetShaderType() const { return Frequency; }

		inline int32 GetPermutationCount() const
		{
			return TotalPermutationCount;
		}

		RenderShader* Construct() const;

		bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters) const;
		void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment) const;


		BuiltInShaderMeta* GetBuiltInShaderType()
		{
			return ShaderMetaForDownCast == EShaderMetaForDownCast::BuitlIn ? (BuiltInShaderMeta*)(this) : nullptr;
		}

		/** Returns the meta data for the root shader parameter struct. */
		inline const ShaderParametersMetadata* GetRootParametersMetadata() const
		{
			return RootParametersMetadata;
		}
	private:
		EShaderMetaForDownCast ShaderMetaForDownCast;

		std::string TypeName;
		std::size_t Hash;
		std::string SourceFileName;
		std::size_t HashedShaderFilename;
		std::string EntryPoint;
		platform::Render::ShaderType Frequency;
		int32 TotalPermutationCount;

		ConstructType ConstructRef;
		ModifyCompilationEnvironmentType ModifyCompilationEnvironmentRef;
		ShouldCompilePermutationType ShouldCompilePermutationRef;

		const ShaderParametersMetadata* const RootParametersMetadata;
	};

	template <typename ParameterStruct>
	inline void BindForLegacyShaderParameters(RenderShader* Shader, const ShaderParameterMap& ParameterMap)
	{
		Shader->Bindings.BindForLegacyShaderParameters(Shader, ParameterMap, *ParameterStruct::TypeInfo::GetStructMetadata());
	}

	template<>
	inline void BindForLegacyShaderParameters<void>(RenderShader* Shader, const ShaderParameterMap& ParameterMap)
	{
	}

	inline void BindForRootShaderParameters(RenderShader* Shader, const ShaderParameterMap& ParameterMap)
	{
		Shader->Bindings.BindForRootShaderParameters(Shader, ParameterMap);
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

	template <typename ShaderClass>
	requires requires{typename ShaderClass::Parameters; }
	inline const ShaderParametersMetadata* ParametersMetadata()
	{
		return ShaderClass::Parameters::TypeInfo::GetStructMetadata();
	}

	template<typename ShaderClass>
	inline const ShaderParametersMetadata* ParametersMetadata()
	{
		return nullptr;
	}


	template<typename ShaderType>
	class ShaderRefBase
	{
	public:
		ShaderRefBase():ShaderContent(nullptr)
		{}

		ShaderRefBase(ShaderType* InShader) :ShaderContent(InShader)
		{}

		template<std::derived_from<ShaderType> OtherShaderType>
		ShaderRefBase(const ShaderRefBase<OtherShaderType>& Rhs) 
		: ShaderContent(Rhs.GetShader()) 
		{
		}

		inline ShaderType* operator->() const { return ShaderContent; }

		inline ShaderType* GetShader() const { return ShaderContent; }



		template<typename OtherShaderType>
		static ShaderRefBase<ShaderType> Cast(const ShaderRefBase<OtherShaderType>& Rhs)
		{
			return ShaderRefBase<ShaderType>(static_cast<ShaderType*>(Rhs.GetShader()));
		}

		inline bool IsValid() const { return ShaderContent != nullptr; }

		inline explicit operator bool() const { return IsValid(); }
	private:
		ShaderType* ShaderContent;
	};

	template<typename ShaderType>
	using ShaderRef = ShaderRefBase< ShaderType>;

	

#define EXPORTED_SHADER_VTABLE(ShaderClass) \
	static RenderShader* ConstructInstance() { return new ShaderClass();} \
	static RenderShader* ConstructCompiledInstance(const ShaderMetaType::CompiledShaderInitializer& Initializer) { return new ShaderClass(Initializer);} \
	static constexpr bool HasParameters =  platform::Render::ShaderParametersType<ShaderClass>::HasParameters;\
	static void ModifyCompilationEnvironmentImpl(const platform::Render::FShaderPermutationParameters& Parameters, platform::Render::FShaderCompilerEnvironment& OutEnvironment) \
	{ \
		const typename ShaderClass::FPermutationDomain PermutationVector(Parameters.PermutationId); \
		PermutationVector.ModifyCompilationEnvironment(OutEnvironment); \
		ShaderClass::ModifyCompilationEnvironment(static_cast<const typename ShaderClass::FPermutationParameters&>(Parameters), OutEnvironment); \
	} \
	static bool ShouldCompilePermutationImpl(const platform::Render::FShaderPermutationParameters& Parameters) \
	{ return ShaderClass::ShouldCompilePermutation(static_cast<const typename ShaderClass::FPermutationParameters&>(Parameters)); }

#define EXPORTED_SHADER_TYPE(ShaderClass,ShaderMetaTypeShortcut) \
public:\
	using ShaderMetaType = platform::Render::##ShaderMetaTypeShortcut##ShaderMeta;\
	using ShaderMapType = platform::Render::##ShaderMetaTypeShortcut##ShaderMap;\
	static ShaderMetaType StaticType; \
	EXPORTED_SHADER_VTABLE(ShaderClass)\
	ShaderClass(const ShaderMetaType::CompiledShaderInitializer& Initializer) \
		:DerivedType(Initializer)\
	{\
		if constexpr(ShaderClass::RootParameterStruct)\
			platform::Render::BindForRootShaderParameters(this, Initializer.ParameterMap); \
		else if constexpr(platform::Render::ShaderParametersType<ShaderClass>::HasParameters)\
			platform::Render::BindForLegacyShaderParameters<platform::Render::ShaderParametersType_t<ShaderClass>>(this,Initializer.ParameterMap);\
	}\
	ShaderClass() \
	{ }

#define SHADER_VTABLE(ShaderClass) \
	ShaderClass::FPermutationDomain::PermutationCount,\
	ShaderClass::ConstructInstance, \
	ShaderClass::ModifyCompilationEnvironmentImpl,\
	ShaderClass::ShouldCompilePermutationImpl

#define IMPLEMENT_SHADER(ShaderClass,SourceFileName,FunctionName,Frequency) \
	ShaderClass::ShaderMetaType ShaderClass::StaticType( \
		ShaderClass::ShaderMetaType::EShaderMetaForDownCast::Reserve,\
		#ShaderClass, \
		SourceFileName, \
		FunctionName, \
		Frequency, \
		SHADER_VTABLE(ShaderClass),\
		sizeof(ShaderClass),\
		ShaderClass::GetRootParametersMetadata()\
	)

	/** A reference which is initialized with the requested shader type from a shader map. */
	template<typename ShaderType>
	class ShaderMapRef : public ShaderRef<ShaderType>
	{
	public:
		ShaderMapRef(const typename ShaderType::ShaderMapType* ShaderIndex)
			: ShaderRef<ShaderType>(ShaderIndex->template GetShader<ShaderType>(/* PermutationId = */ 0)) // gcc3 needs the template quantifier so it knows the < is not a less-than
		{
			static_assert(
				std::is_same_v<typename ShaderType::FPermutationDomain, FShaderPermutationNone>,
				"Missing permutation vector argument for shader that have a permutation domain.");
		}

		ShaderMapRef(
			const typename ShaderType::ShaderMapType* ShaderIndex,
			const typename ShaderType::FPermutationDomain& PermutationVector)
			: ShaderRef<ShaderType>(ShaderIndex->template GetShader<ShaderType>(PermutationVector.ToDimensionValueId())) // gcc3 needs the template quantifier so it knows the < is not a less-than
		{ }
	};

	class ShaderMapContent
	{
	public:
		~ShaderMapContent()
		{
			Clear();
		}

		/** Finds the shader with the given type.  Asserts on failure. */
		template<typename ShaderType>
		ShaderType* GetShader(int32 PermutationId = 0) const
		{
			auto Shader = GetShader(&ShaderType::StaticType, PermutationId);
			LAssert(Shader != nullptr, leo::sfmt("Failed to find shader type %s in Platform %s", ShaderType::StaticType.GetName(),"PCD3D_SM5").c_str());
			return static_cast<ShaderType*>(Shader);
		}

		/** Finds the shader with the given type.  Asserts on failure. */
		template<typename ShaderType>
		ShaderType* GetShader(const typename ShaderType::FPermutationDomain& PermutationVector) const
		{
			return GetShader<ShaderType>(PermutationVector.ToDimensionValueId());
		}

		/** Finds the shader with the given type.  May return NULL. */
		RenderShader* GetShader(ShaderMeta* ShaderType, int32 PermutationId = 0) const
		{
			return GetShader(ShaderType->GetHash(), PermutationId);
		}

		/** Finds the shader with the given type name.  May return NULL. */
		RenderShader* GetShader(size_t TypeNameHash, int32 PermutationId = 0) const;

		/** Finds the shader with the given type. */
		bool HasShader(size_t TypeNameHash, int32 PermutationId) const
		{
			auto Shader = GetShader(TypeNameHash, PermutationId);
			return Shader != nullptr;
		}

		bool HasShader(const ShaderMeta* Type, int32 PermutationId) const
		{
			return HasShader(Type->GetHash(), PermutationId);
		}

		RenderShader* FindOrAddShader(size_t TypeNameHash, int32 PermutationId, RenderShader* Shader);

		/** clears out all shaders and deletes shader pipelines held in the map */
		void Clear();

		/** @return true if the map is empty */
		inline bool IsEmpty() const
		{
			return Shaders.empty();
		}

		/** @return The number of shaders in the map. */
		uint32 GetNumShaders() const;
	private:
		std::vector<RenderShader*> Shaders;
		std::vector<size_t> ShaderTypes;
		std::vector<int32> ShaderPermutations;
		std::unordered_multimap<leo::uint16,leo::uint32> ShaderHash;
		std::shared_mutex ShaderMutex;
	};


	void CompileShaderMap();

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
