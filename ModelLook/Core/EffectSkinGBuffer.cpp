#include "Core/EffectGBuffer.hpp"
#include "Core/Vertex.hpp"
#include "Core/FileSearch.h"
#include "Core/EngineConfig.h"

#include "RenderSystem/ShaderMgr.h"
#include "RenderSystem/RenderStates.hpp"

#include <leomathutility.hpp>

namespace leo {
#pragma region EffectSkinGBuffer
	using vector = __m128;
	using matrix = std::array < __m128, 4 >;

	class EffectSkinGBufferDelegate :CONCRETE(EffectSkinGBuffer), public Singleton<EffectSkinGBufferDelegate>
	{
	public:
		EffectSkinGBufferDelegate(ID3D11Device* device)
			:mVSCBPerFrame(device), mVertexShaderConstantBufferPerSkin(device), mPSCBPerMatrial(device),mPSCBPerCamera(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(
				FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"skingbuffer", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::Skinned,arrlen(InputLayoutDesc::Skinned), &layout);

			auto blob = sm.CreateBlob(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"skingbuffer", D3D11_PIXEL_SHADER)));
			mPixelShader = sm.CreatePixelShader(blob);

			RenderStates rss;
			aniso_sampler = rss.GetSamplerState(L"aniso_sampler");
			bilinear_sampler = rss.GetSamplerState(L"bilinear_sampler");
		}

		~EffectSkinGBufferDelegate()
		{

		}
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"skingbuffer");
			mVSCBPerFrame.Update(context);
			mPSCBPerMatrial.Update(context);
			mPSCBPerCamera.Update(context);
			mVertexShaderConstantBufferPerSkin.Update(context);

			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.VSSetConstantBuffers(0, 1, &mVSCBPerFrame.mBuffer);
			pContext.VSSetConstantBuffers(1, 1, &mVertexShaderConstantBufferPerSkin.mBuffer);

			pContext.PSSetShader(mPixelShader, nullptr, 0);

			std::invoke(dx::SetSampleState<D3D11_PIXEL_SHADER>(context), 0, aniso_sampler, bilinear_sampler);
			std::invoke(dx::SetShaderResourceView<D3D11_PIXEL_SHADER>(context),0,mPixelShaderDiffuseSRV, mPixelShaderNormalSRV);
			std::invoke(dx::SetConstantBuffer<D3D11_PIXEL_SHADER>(context), 0, mPSCBPerMatrial.mBuffer, mPSCBPerCamera.mBuffer);
		}


		void  LM_VECTOR_CALL WorldViewProjMatrix(matrix  Matrix, ID3D11DeviceContext* context) {
			mVSCBPerFrame.WorldViewProj = Transpose(Matrix);
			if (context)
				mVSCBPerFrame.Update(context);
		}
		void LM_VECTOR_CALL WorldMatrix(matrix  Matrix, ID3D11DeviceContext* context) {
			vector unused;
			mVSCBPerFrame.InvTransposeWorld = Inverse(unused,Matrix);
			mVSCBPerFrame.World =Transpose(Matrix);
			if (context)
				mVSCBPerFrame.Update(context);
		}
		void LM_VECTOR_CALL ViewMatrix(matrix  Matrix, ID3D11DeviceContext* context) {
			mPSCBPerCamera.View = Transpose(Matrix);
			if (context)
				mPSCBPerCamera.Update(context);
		}

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context) {
			mPixelShaderDiffuseSRV = diff;
			if (context)
				context->PSSetShaderResources(0, 1, &mPixelShaderDiffuseSRV);
		}
		void NormalSRV(ID3D11ShaderResourceView * const normal, ID3D11DeviceContext* context) {
			mPixelShaderNormalSRV = normal;
			if (context)
				context->PSSetShaderResources(1, 1, &mPixelShaderNormalSRV);
		}


		void Specular(const float4& specular_power, ID3D11DeviceContext* context) {
			mPSCBPerMatrial.specular_pow = specular_power;
			if (context)
				mPSCBPerMatrial.Update(context);
		}

		void SkinMatrix(float4x4Object * globalmatrix, std::uint32_t numJoint) {
			mNumJoint = numJoint;
			for (auto i = 0u; i != numJoint; ++i) {
				mVertexShaderConstantBufferPerSkin.SkinMatrix[i] = Transpose(load(static_cast<const float4x4&>(globalmatrix[i])));
			}
		}

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
	private:
		struct VScbPerFrame
		{
			matrix World;
			matrix InvTransposeWorld;
			matrix WorldViewProj;
		public:
			const static std::uint8_t slot = 0;
		};

		ShaderConstantBuffer<VScbPerFrame> mVSCBPerFrame;

		struct VScbPerSkin
		{
			matrix SkinMatrix[96];
		public:
			const static std::uint8_t slot = 1;
		};
		ShaderConstantBuffer<VScbPerSkin> mVertexShaderConstantBufferPerSkin;
		std::uint32_t mNumJoint = 0;


		struct PScbPerMaterial {
			float4 specular_pow;
		public:
			const static std::uint8_t slot = 0;
		};
		struct PScbPerCamera {
			matrix View;
		public:
			const static std::uint8_t slot = 1;
		};
		ShaderConstantBuffer<PScbPerMaterial> mPSCBPerMatrial;
		ShaderConstantBuffer<PScbPerCamera> mPSCBPerCamera;


		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11PixelShader*	mPixelShader = nullptr;

		ID3D11ShaderResourceView *mPixelShaderDiffuseSRV = nullptr;
		ID3D11ShaderResourceView *mPixelShaderNormalSRV = nullptr;


		ID3D11SamplerState* aniso_sampler = nullptr;
		ID3D11SamplerState* bilinear_sampler = nullptr;
	};

	

	std::unique_ptr<EffectSkinGBuffer>& EffectSkinGBuffer::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = std::unique_ptr<EffectSkinGBuffer>(new EffectSkinGBufferDelegate(device));
		return mInstance;
	}

	void EffectSkinGBuffer::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->Apply(
			context
			);
	}


	void LM_VECTOR_CALL EffectSkinGBuffer::WorldViewProjMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->WorldViewProjMatrix(
			matrix, context
			);
	}



	void LM_VECTOR_CALL  EffectSkinGBuffer::WorldMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context) {
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->WorldMatrix(
			matrix, context
			);
	}

	void LM_VECTOR_CALL EffectSkinGBuffer::ViewMatrix(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->ViewMatrix(
			Matrix, context
			);
	}


	void EffectSkinGBuffer::DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->DiffuseSRV(
			diff, context
			);
	}


	void EffectSkinGBuffer::NormalSRV(ID3D11ShaderResourceView * const normal, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->NormalSRV(
			normal, context
			);
	}

	void leo::EffectSkinGBuffer::Specular(const float4 & specular_power, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->Specular(
			specular_power, context
			);
	}

	void EffectSkinGBuffer::SkinMatrix(float4x4Object* globalmatrix, std::uint32_t numJoint) {
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->SkinMatrix(globalmatrix, numJoint
			);
	}

	bool EffectSkinGBuffer::SetLevel(EffectConfig::EffectLevel l)  lnothrow
	{
		lassume(dynamic_cast<EffectSkinGBufferDelegate *>(this));

		return ((EffectSkinGBufferDelegate *)this)->SetLevel(
			l
			);
	}

	
#pragma endregion
}