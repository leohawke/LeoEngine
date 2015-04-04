#include "EffectGBuffer.hpp"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"
#include "..\DeviceMgr.h"
#include "Vertex.hpp"
#include "FileSearch.h"
#include "EngineConfig.h"
#include "..\TextureMgr.h"
#include "..\leomath.hpp"

namespace leo {
#pragma region EffectGBuffer
	using vector = __m128;
	using matrix = std::array < __m128, 4 >;

	class EffectGBufferDelegate :CONCRETE(EffectGBuffer), public Singleton<EffectGBufferDelegate>
	{
	public:
		EffectGBufferDelegate(ID3D11Device* device)
			:mVertexShaderConstantBufferPerFrame(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(
				FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"gbuffer", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::NormalMap, 4, &layout);

			auto blob = sm.CreateBlob(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"gbuffer", D3D11_PIXEL_SHADER)));
			mPixelShader = sm.CreatePixelShader(blob);

			RenderStates rss;
			anisoSampler = rss.GetSamplerState(L"anisoSampler");
			normalSampler = rss.GetSamplerState(L"trilinearSampler");

			leo::TextureMgr texmgr;
			mPixelShaderNormalsSRV =texmgr.LoadTextureSRV(FileSearch::Search(L"NormalsFitting.dds"));
		}

		~EffectGBufferDelegate()
		{
			
		}
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"gbuffer");
			mVertexShaderConstantBufferPerFrame.Update(context);

			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.VSSetConstantBuffers(0, 1, &mVertexShaderConstantBufferPerFrame.mBuffer);

			ID3D11SamplerState* msss[] = {
				normalSampler,
				anisoSampler
			};

			ID3D11ShaderResourceView* mrss[] = {
				mPixelShaderNormalsSRV,
				mPixelShaderDiffuseSRV
			};

			pContext.PSSetShader(mPixelShader, nullptr, 0);
			pContext.PSSetSamplers(0, 2, msss);
			pContext.PSSetShaderResources(0, 2, mrss);

		}

		void LM_VECTOR_CALL WorldViewMatrix(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVertexShaderConstantBufferPerFrame.world = Transpose(Matrix);
			if (context)
				mVertexShaderConstantBufferPerFrame.Update(context);
		}

		void LM_VECTOR_CALL WorldInvTransposeViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr) {
			mVertexShaderConstantBufferPerFrame.worldinvtranspose = Transpose(matrix);
			if (context)
				mVertexShaderConstantBufferPerFrame.Update(context);
		}


		void  LM_VECTOR_CALL WorldViewProjMatrix(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVertexShaderConstantBufferPerFrame.worldviewproj = Transpose(Matrix);
			if (context)
				mVertexShaderConstantBufferPerFrame.Update(context);
		}
		

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context)
		{
			mPixelShaderDiffuseSRV = diff;

		}
		void NormalsSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context)
		{
			mPixelShaderNormalsSRV = nmap;
		}

		void OMSetMRT(ID3D11RenderTargetView* rt0, ID3D11RenderTargetView* rt1) {
			mMRTs[0] = rt0;
			mMRTs[1] = rt1;
			DeviceMgr().GetDeviceContext()->OMSetRenderTargets(2, mMRTs, DeviceMgr().GetDepthStencilView());
		}
		
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
	private:
		struct VScbPerFrame
		{
			matrix world;
			matrix worldinvtranspose;
			matrix worldviewproj;
			matrix shadowviewprojtex;
		public:
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<VScbPerFrame> mVertexShaderConstantBufferPerFrame;
		

		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11PixelShader*	mPixelShader = nullptr;

		ID3D11ShaderResourceView *mPixelShaderDiffuseSRV = nullptr;
		ID3D11ShaderResourceView *mPixelShaderNormalsSRV = nullptr;

		ID3D11SamplerState* normalSampler = nullptr;
		ID3D11SamplerState* anisoSampler = nullptr;

		ID3D11RenderTargetView* mMRTs[2];
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

	void EffectGBuffer::WorldViewMatrix(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->WorldViewMatrix(
			load(matrix), context
			);
	}

	void EffectGBuffer::WorldViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->WorldViewProjMatrix(
			load(matrix), context
			);
	}


	void LM_VECTOR_CALL EffectGBuffer::WorldViewMatrix(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->WorldViewMatrix(
			Matrix, context
			);
	}

	void LM_VECTOR_CALL  EffectGBuffer::WorldInvTransposeViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context) {
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->WorldInvTransposeViewMatrix(
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

	void EffectGBuffer::NormalsSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->NormalsSRV(
			nmap, context
			);
	}

	bool EffectGBuffer::SetLevel(EffectConfig::EffectLevel l)  lnothrow
	{
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->SetLevel(
			l
			);
	}

	void EffectGBuffer::OMSetMRT(ID3D11RenderTargetView* rt0, ID3D11RenderTargetView* rt1) {
		lassume(dynamic_cast<EffectGBufferDelegate *>(this));

		return ((EffectGBufferDelegate *)this)->OMSetMRT(
			rt0,rt1
			);
	}
#pragma endregion
}