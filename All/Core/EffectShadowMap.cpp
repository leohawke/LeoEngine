#include "EffectShadowMap.hpp"

namespace leo {
	using matrix = std::array<__m128, 4>;

	class EffectShadowMapDelegate :CONCRETE(EffectShadowMap), public Singleton<EffectShadowMapDelegate>
	{
	public:
		EffectShadowMapDelegate(ID3D11Device*) {

		}

		~EffectShadowMapDelegate(){}

		void Apply(ID3D11DeviceContext* context) {

		}
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow {
			return true;
		}

		void WorldMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr) {

		}

		void ViewProjTexMatrix(const float4x4& matrix, ID3D11DeviceContext* context) {

		}
	private:
		struct VScbPerFrame
		{
			matrix Wolrd;
			matrix VPT;
		public:
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<VScbPerFrame> mVertexShaderConstantBufferPerFrame;
	};

	void EffectShadowMap::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectShadowMapDelegate *>(this));

		return ((EffectShadowMapDelegate *)this)->Apply(
			context
			);
	}

	bool EffectShadowMap::SetLevel(EffectConfig::EffectLevel l) lnothrow
	{
		lassume(dynamic_cast<EffectShadowMapDelegate *>(this));

		return ((EffectShadowMapDelegate *)this)->SetLevel(
			l
			);
	}

	void EffectShadowMap::ViewProjTexMatrix(const float4x4& matrix, ID3D11DeviceContext* context) {
		lassume(dynamic_cast<EffectShadowMapDelegate *>(this));

		return ((EffectShadowMapDelegate *)this)->ViewProjTexMatrix(
			matrix,
			context
			);
	}

	void EffectShadowMap::WorldMatrix(const float4x4& matrix, ID3D11DeviceContext* context) {
		lassume(dynamic_cast<EffectShadowMapDelegate *>(this));

		return ((EffectShadowMapDelegate *)this)->WorldMatrix(
			matrix,
			context
			);
	}
}