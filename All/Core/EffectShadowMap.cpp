#include "EffectShadowMap.hpp"
#include "EngineConfig.h"
#include "FileSearch.h"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"

namespace leo {
	using matrix = std::array<__m128, 4>;

	class EffectShadowMapDelegate :CONCRETE(EffectShadowMap), public Singleton<EffectShadowMapDelegate>
	{
	public:
		EffectShadowMapDelegate(ID3D11Device* device)
		:mVSCBPerLight(device), mVSCBPerModel(device) {
			leo::ShaderMgr sm;
			mVS = sm.CreateVertexShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"shadowmap", D3D11_VERTEX_SHADER)));

			RenderStates rss;
			mRS = rss.GetRasterizerState(L"ShadowMapRS");
		}

		~EffectShadowMapDelegate(){}

		void Apply(ID3D11DeviceContext* context) {
			mVSCBPerModel.Update(context);
			mVSCBPerLight.Update(context);

			ID3D11Buffer* mvscbs[] = {
				mVSCBPerModel.mBuffer,
				mVSCBPerLight.mBuffer
			};

			context->VSSetShader(mVS, nullptr, 0);
			context->VSSetConstantBuffers(0, 2, mvscbs);
			context->RSSetState(mRS);
		}
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow {
			return true;
		}

		void WorldMatrix(const float4x4& matrix, ID3D11DeviceContext* context ) {
			mVSCBPerModel.mWolrd = Transpose(load(matrix));
			if (context)
				mVSCBPerModel.Update(context);
		}

		void ViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context) {
			mVSCBPerLight.mViewProj = Transpose(load(matrix));
			if (context)
				mVSCBPerLight.Update(context);
		}
	private:
		struct VScbPerModel
		{
			matrix mWolrd;
		public:
			const static std::uint8_t slot = 0;
		};
		struct VScbPerLight
		{
			matrix mViewProj;
		public:
			const static std::uint8_t slot = 1;
		};
		ShaderConstantBuffer<VScbPerModel> mVSCBPerModel;
		ShaderConstantBuffer<VScbPerLight> mVSCBPerLight;

		ID3D11VertexShader* mVS = nullptr;
		ID3D11RasterizerState* mRS = nullptr;
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

	void EffectShadowMap::ViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context) {
		lassume(dynamic_cast<EffectShadowMapDelegate *>(this));

		return ((EffectShadowMapDelegate *)this)->ViewProjMatrix(
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

	const std::unique_ptr<EffectShadowMap>& EffectShadowMap::GetInstance(ID3D11Device* device) {
		static auto mInstance = std::unique_ptr<EffectShadowMap>(new EffectShadowMapDelegate(device));
		return mInstance;
	}
}