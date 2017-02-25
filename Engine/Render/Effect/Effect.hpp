/*! \file Engine\Render\Effect\Effect.hpp
\ingroup Engine\Render\Effect
\brief Effect¹«¹²¼¯¡£
*/
#ifndef LE_RENDER_EFFECT_h
#define LE_RENDER_EFFECT_h 1

#include <LBase/linttype.hpp>
#include <LBase/lmathtype.hpp>
#include <LBase/any.h>

#include <tuple>
#include <vector>
#include <memory>
#include <optional>

#include "../PipleState.h"

namespace asset {
	class EffectAsset;
}

namespace platform::Render {

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
	struct NameKey{
		NameKey(const std::string& name)
			:NameKey(name,std::hash<std::string>()(name))
		{}

		NameKey(const std::string& name,size_t hash)
			:Name(name),Hash(hash)
		{}

	

		const std::string Name;
		const size_t Hash;
	};

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
		using NameKey::NameKey;

		friend class Effect;
	private:
		Variable var;
	};

	class Pass {
	public:
		void Bind(Effect &);
		void UnBind(Effect &);
		ShaderCompose& GetShader(Effect&);
		PipleState& GetState();

		friend class Effect;
	private:
		PipleState state;
		leo::uint8 bind_index;
	};

	class Technique : public NameKey {
	public:
		using NameKey::NameKey;
		
		Pass& GetPass(leo::uint8 index);

		friend class Effect;
	private:
		std::vector<Pass> passes;
	};

	class Effect :public NameKey {
	public:
		Effect(const std::string& name);

		void Bind(leo::uint8 index);
		ShaderCompose& GetShader(leo::uint8 index);

		const Technique& GetTechnique(const std::string& name) const;
		const Technique& GetTechnique(size_t hash) const;
		const Technique& GetTechniqueByIndex(size_t index) const;

		Parameter& GetParameter(const std::string& name);
		Parameter& GetParameter(size_t hash);
		
		ConstantBuffer& GetConstantBuffer(const std::string& name);
		ConstantBuffer& GetConstantBuffer(size_t hash);
	private:
		void LoadAsset(leo::observer_ptr<asset::EffectAsset> pEffectAsset);
	private:
		std::vector<Technique> techniques;
		std::unordered_map<size_t,Parameter> parameters;
		std::vector<std::shared_ptr<ConstantBuffer>> constantbuffs;
		std::vector<std::unique_ptr<ShaderCompose>> shaders;
	};
}

#endif