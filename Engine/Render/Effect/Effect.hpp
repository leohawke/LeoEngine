/*! \file Engine\Render\Effect\Effect.hpp
\ingroup Engine\Render\Effect
\brief Effect公共集。
*/
#ifndef LE_RENDER_EFFECT_h
#define LE_RENDER_EFFECT_h 1

#include <LBase/linttype.hpp>
#include <LBase/lmathtype.hpp>
#include <LBase/any.h>
#include <LFramework/LCLib/Debug.h>

#include "../PipleState.h"
#include "../../Core/ResourcesHolder.h"

#include <tuple>
#include <vector>
#include <memory>
#include <optional>

namespace asset {
	class EffectAsset;

	enum EffectParamType
	{
		EPT_texture1D,
		EPT_texture2D,
		EPT_texture3D,
		EPT_textureCUBE,
		EPT_texture1DArray,
		EPT_texture2DArray,
		EPT_texture3DArray,
		EPT_textureCUBEArray,
		EPT_buffer,
		EPT_StructuredBuffer,
		EPT_rwbuffer,
		EPT_rwstructured_buffer,
		EPT_rwtexture1D,
		EPT_rwtexture2D,
		EPT_rwtexture3D,
		EPT_rwtexture1DArray,
		EPT_rwtexture2DArray,
		EPT_AppendStructuredBuffer,
		EPT_ConsumeStructuredBuffer,
		EPT_byteAddressBuffer,
		EPT_rwbyteAddressBuffer,
		EPT_sampler,
		EPT_shader,
		EPT_bool,
		EPT_string,
		EPT_uint,
		EPT_uint2,
		EPT_uint3,
		EPT_uint4,
		EPT_int,
		EPT_int2,
		EPT_int3,
		EPT_int4,
		EPT_float,
		EPT_float2,
		EPT_float2x2,
		EPT_float2x3,
		EPT_float2x4,
		EPT_float3,
		EPT_float3x2,
		EPT_float3x3,
		EPT_float3x4,
		EPT_float4,
		EPT_float4x2,
		EPT_float4x3,
		EPT_float4x4,

		EPT_ElemEmpty,
	};
}

namespace platform {
	class Material;
}

namespace platform::Render {
	class Texture;

	struct  TextureSubresource
	{
		std::shared_ptr<Texture> tex;

		uint32_t first_array_index;

		uint32_t num_items;

		uint32_t first_level;

		uint32_t num_levels;


		TextureSubresource()
		{
		}


		TextureSubresource(std::shared_ptr<Texture> const & t, uint32_t fai, uint32_t ni, uint32_t fl, uint32_t nl)
			: tex(t), first_array_index(fai), num_items(ni), first_level(fl), num_levels(nl)

		{
		}
	};


	using EffectParamType = asset::EffectParamType;

	class ShaderCompose {
	public:
		virtual ~ShaderCompose();

		enum class Type : leo::uint8
		{
			VertexShader,
			PixelShader,
			GeometryShader,
			ComputeShader,
			HullShader,
			DomainShader,
		};

		static const leo::uint8 NumTypes = (leo::uint8)Type::DomainShader + 1;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		using ShaderBlob = std::pair<std::unique_ptr<stdex::byte[]>, std::size_t>;
	};

	struct ShaderInfo {
		ShaderCompose::Type Type;

		ShaderInfo(ShaderCompose::Type t);

		struct ConstantBufferInfo
		{
			struct VariableInfo
			{
				std::string name;
				uint32_t start_offset;
				uint8_t type;
				uint8_t rows;
				uint8_t columns;
				uint16_t elements;
			};
			std::vector<VariableInfo> var_desc;

			std::string name;
			size_t name_hash;
			uint32_t size = 0;
		};
		std::vector<ConstantBufferInfo> ConstantBufferInfos;

		struct BoundResourceInfo
		{
			std::string name;
			uint8_t type;
			uint8_t dimension;
			uint16_t bind_point;
		};
		std::vector<BoundResourceInfo> BoundResourceInfos;

		uint16_t NumSamplers = 0;
		uint16_t NumSrvs = 0;
		uint16_t NumUavs = 0;

		std::optional<size_t> InputSignature = std::nullopt;
		std::optional<leo::math::data_storage<uint16,3>> CSBlockSize = std::nullopt;
	};

	class GraphicsBuffer;
}

namespace platform::Render::Effect {
	struct NameKey : public leo::noncopyable{
		NameKey(const std::string& name)
			:NameKey(name,std::hash<std::string>()(name))
		{}

		NameKey(const std::string& name,size_t hash)
			:Name(name),Hash(hash)
		{}

	

		const std::string Name;
		const size_t Hash;
	};

	//TODO 持有一下自己的Parameter
	class ConstantBuffer :public NameKey {
	public:
		using NameKey::NameKey;

		void Update();

		void Dirty(bool val)
		{
			dirty = val;
		}

		GraphicsBuffer* GetGraphicsBuffer() const lnothrow;

		friend class Effect;
	protected:

		template <typename T>
		T const & VariableInBuff(uint32_t offset) const
		{
			union Raw2T
			{
				uint8_t const * raw;
				T const * t;
			} r2t;
			r2t.raw = &cpu_buffer[offset];
			return *r2t.t;
		}
		template <typename T>
		T& VariableInBuff(uint32_t offset)
		{
			union Raw2T
			{
				uint8_t* raw;
				T* t;
			} r2t;
			r2t.raw = &cpu_buffer[offset];
			return *r2t.t;
		}

		friend class Variable;
	private:
		std::unique_ptr<GraphicsBuffer> gpu_buffer;
		std::vector<stdex::byte> cpu_buffer;
		bool dirty;
	};

	class Variable {
	public:
		//T must be value semantic!
		template<typename T>
		Variable& operator=(T const & val) {
			if (bind.target) {
				T& bindvalue = bind.target->template VariableInBuff<T>(bind.offset);
				if (!leo::examiners::equal_examiner::are_equal(bindvalue, val)) {
					bindvalue = val;
					bind.target->Dirty(true);
				}
			}
			else {
				value = val;
			}
			return *this;
		}

		template<typename T>
		T Get() const {
			if (bind.target)
				return bind.target->template VariableInBuff<T>(bind.offset);
			else
				return leo::any_cast<T>(value);
		}

#if ENGINE_TOOL
		void Bind(std::shared_ptr<ConstantBuffer> target, uint32 offset, uint32 stride)
		{
			//save value?
			bind.target = target;
			bind.offset = offset;
			bind.stride = stride;
		}
#endif

		friend class Effect;
		friend class Parameter;
	private:
		Variable& operator=(void*  const & pointer) {
			LAssert(bind.target, "Memory Assign Must Has CBufferBind");
			std::memcpy(&bind.target->cpu_buffer[bind.offset],pointer,bind.stride);
			bind.target->Dirty(true);
			return *this;
		}

		template<typename T>
		T& Ref() {
			if (bind.target)
				return bind.target->template VariableInBuff<T>(bind.offset);
			else
				return Deref(*leo::any_cast<T*>(&value));
		}
	private:
		leo::any value;
		struct CBufferBind {
			std::shared_ptr<ConstantBuffer> target;
			uint32 offset =0;
			uint32 stride =0;
		} bind;
	};

	class Parameter :public NameKey {
	public:
		Parameter(const std::string& name, EffectParamType type_)
			:NameKey(name, std::hash<std::string>()(name)),type(type_)
		{}

		Parameter(const std::string& name, size_t hash, EffectParamType type_)
			:NameKey(name,hash), type(type_)
		{}

		DefGetter(const lnothrow, EffectParamType,Type,type)

		template<typename T>
		void Value(T& value) {
			value = var.Get<T>();
		}

		template<typename T>
		Parameter& operator=(T const & val) {
			var = val;
			return *this;
		}

		friend class Effect;
		friend class Material;
	private:
		Parameter& operator=(const leo::any& val);
	private:
		Variable var;

		EffectParamType type;
	};

	class Pass {
	public:
		Pass() = default;
		Pass(Pass&&) = default;
		void Bind(const Effect &) const;
		void UnBind(const Effect &) const;
		ShaderCompose& GetShader(const Effect&) const;
		const PipleState& GetState() const;

		friend class Effect;
	private:
		std::unique_ptr<PipleState> state;
		leo::uint8 bind_index;
	};

	class Technique : public NameKey {
	public:
		Technique() = default;
		Technique(Technique&&) = default;

		using NameKey::NameKey;
		
		const Pass& GetPass(leo::uint8 index) const;
		Pass& GetPass(leo::uint8 index);

		std::size_t NumPasses() const {
			return passes.size();
		}

		bool HasTessellation() const{
			return false;
		}

		friend class Effect;
	private:
		std::vector<Pass> passes;
	};

	class Effect :public NameKey,public Resource {
	public:
		Effect(const std::string& name);
		virtual ~Effect();

		const std::string& GetName()  const lnothrow override;

		void Bind(leo::uint8 index);
		ShaderCompose& GetShader(leo::uint8 index) const ;

		const Technique& GetTechnique(const std::string& name) const;
		const Technique& GetTechnique(size_t hash) const;
		const Technique& GetTechniqueByIndex(size_t index) const;

		Parameter& GetParameter(const std::string_view& name);
		Parameter& GetParameter(size_t hash);


		ConstantBuffer& GetConstantBuffer(size_t index);
		size_t ConstantBufferIndex(const std::string& name);
		size_t ConstantBufferIndex(size_t hash);
	protected:
		Technique& GetTechnique(const std::string& name);

		template<typename T>
		T& GetParameterRef(const std::string& name){
			return GetParameter(name).var.Ref<T>();
		}
	private:
		void LoadAsset(leo::observer_ptr<asset::EffectAsset> pEffectAsset);
	private:
		std::vector<Technique> techniques;
		std::unordered_map<size_t,Parameter> parameters;
		std::vector<std::shared_ptr<ConstantBuffer>> constantbuffs;
		std::vector<std::unique_ptr<ShaderCompose>> shaders;
	};

	class EffectsHolder :ResourcesHolder<Effect> {
	public:
		EffectsHolder();
		~EffectsHolder();
		
		std::shared_ptr<void> FindResource(const leo::any& key) override;
		std::shared_ptr<Effect> FindResource(const std::string& name);
		std::shared_ptr<Effect> FindResource(const std::shared_ptr<asset::EffectAsset>& asset);

		void Connect(const std::shared_ptr<asset::EffectAsset>& asset, const std::shared_ptr<Effect>& effect);

		static EffectsHolder& Instance();
	private:
		std::pmr::vector<std::pair<std::weak_ptr<asset::EffectAsset>,std::shared_ptr<Effect>>> loaded_effects;
	};
}

#endif