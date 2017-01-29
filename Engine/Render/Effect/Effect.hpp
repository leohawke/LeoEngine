/*! \file Engine\Render\Effect\Effect.hpp
\ingroup Engine\Render\Effect
\brief Effect¹«¹²¼¯¡£
*/
#ifndef LE_RENDER_EFFECT_h
#define LE_RENDER_EFFECT_h 1

#include <LBase/linttype.hpp>
#include <LBase/any.h>

#include <tuple>
#include <vector>
#include <memory>

#include "../PipleState.h"

namespace asset {
	class EffectAsset;
}

namespace platform::Render {

	class ShaderCompose {
	public:
		enum class Type : leo::uint8
		{
			VertexShader,
			PixelShader,
			GeometryShader,
			ComputeShader,
			HullShader,
			DomainShader,
		};

		const leo::uint8 NumTypes = (leo::uint8)Type::DomainShader + 1;


		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		using ShaderBlob = std::pair<std::unique_ptr<stdex::byte[]>, std::size_t>;
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
		std::shared_ptr<GraphicsBuffer> gpu_buffer;
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

#if ENGINE_TOOL
		//why need get value?
		template<typename T>
		T Get() const {
			if (bind.target)
				return bind.target->template VariableInBuff<T>(bind.offset);
			else
				return leo::any_cast<T>(value);
		}

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
			uint32 offset;
			uint32 stride;
		} bind;
	};

	class Parameter :public NameKey {
	public:
		using NameKey::NameKey;
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
		
	private:
		void LoadAsset(leo::observer_ptr<asset::EffectAsset> pEffectAsset);
	private:
		std::vector<Technique> techniques;
		std::unordered_map<size_t,Parameter> parameters;

		std::vector<std::unique_ptr<ShaderCompose>> shaders;
	};
}

#endif