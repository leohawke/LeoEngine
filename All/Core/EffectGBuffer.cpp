#include "EffectGBuffer.hpp"
#include "RenderSystem\ShaderMgr.h"
#include "RenderSystem\RenderStates.hpp"
#include "Vertex.hpp"
#include "FileSearch.h"
#include "EngineConfig.h"
#include "leomathutility.hpp"

namespace leo {
#pragma region EffectGBuffer
	using vector = __m128;
	using matrix = std::array < __m128, 4 >;

	class EffectGBufferDelegate :CONCRETE(EffectGBuffer), public Singleton<EffectGBufferDelegate>
	{
	public:
		EffectGBufferDelegate(ID3D11Device* device)
			:mVSCBPerFrame(device),mPSCBPerMatrial(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(
				FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"gbuffer", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::NormalMap, 4, &layout);

			auto blob = sm.CreateBlob(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"gbuffer", D3D11_PIXEL_SHADER)));
			mPixelShader = sm.CreatePixelShader(blob);

			RenderStates rss;
			anisoSampler = rss.GetSamplerState(L"aniso_sampler");
		}

		~EffectGBufferDelegate()
		{
			
		}
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"gbuffer");
			mVSCBPerFrame.Update(context);

			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.VSSetConstantBuffers(0, 1, &mVSCBPerFrame.mBuffer);

			ID3D11SamplerState* msss[] = {
				anisoSampler
			};

			ID3D11ShaderResourceView* mrss[] = {
				mPixelShaderDiffuseSRV
			};

			mPSCBPerMatrial.Update(context);
			pContext.PSSetShader(mPixelShader, nullptr, 0);
			pContext.PSSetConstantBuffers(0, 1, &mPSCBPerMatrial.mBuffer);
			pContext.PSSetSamplers(0, arrlen(msss), msss);
			pContext.PSSetShaderResources(0, arrlen(mrss), mrss);

		}


		void LM_VECTOR_CALL InvTransposeWorldViewMatrix(matrix Matrix, ID3D11DeviceContext* context = nullptr) {
			mVSCBPerFrame.InvTransposeWorldView = Matrix;
			if (context)
				mVSCBPerFrame.Update(context);
		}


		void  LM_VECTOR_CALL WorldViewProjMatrix(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVSCBPerFrame.WorldViewProj = Transpose(Matrix);
			if (context)
				mVSCBPerFrame.Update(context);
		}
		

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context)
		{
			mPixelShaderDiffuseSRV = diff;

		}
		
		void Specular(const float4& specular_power, ID3D11DeviceContext* context) {
			mPSCBPerMatrial.specular_pow = specular_power;
			if (context)
				mPSCBPerMatrial.Update(context);
		}
		
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
	private:
		struct VScbPerFrame
		{
			matrix InvTransposeWorldView;
			matrix WorldViewProj;
		public:
			const static std::uint8_t slot = 0;
		};

		ShaderConstantBuffer<VScbPerFrame> mVSCBPerFrame;
		
		struct PScbPerMaterial {
			float4 specular_pow;
		public:
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<PScbPerMaterial> mPSCBPerMatrial;

		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11PixelShader*	mPixelShader = nullptr;

		ID3D11ShaderResourceView *mPixelShaderDiffuseSRV = nullptr;

		ID3D11SamplerState* anisoSampler = nullptr;
	};

	

	const std::unique_ptr<EffectGBuffer>& EffectGBuffer::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = std::unique_ptr<EffectGBuffer>(new EffectGBufferDelegate(device));
		return mInstance;
	}

	void EffectGBuffer::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->Apply(
			context
			);
	}


	void EffectGBuffer::WorldViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->WorldViewProjMatrix(
			load(matrix), context
			);
	}



	void LM_VECTOR_CALL  EffectGBuffer::InvTransposeWorldViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context) {
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->InvTransposeWorldViewMatrix(
			matrix, context
			);
	}

	void LM_VECTOR_CALL EffectGBuffer::WorldViewProjMatrix(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->WorldViewProjMatrix(
			Matrix, context
			);
	}


	void EffectGBuffer::DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->DiffuseSRV(
			diff, context
			);
	}

	void EffectGBuffer::Specular(const float4 & specular_power, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->Specular(
			specular_power, context
			);
	}

	bool EffectGBuffer::SetLevel(EffectConfig::EffectLevel l)  lnothrow
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->SetLevel(
			l
			);
	}
#pragma endregion
}