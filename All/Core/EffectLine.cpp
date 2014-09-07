#include "EffectLine.hpp"
#include "Vertex.hpp"
#include "..\ShaderMgr.h"
namespace leo
{
	class EffectLineDelegate : CONCRETE(EffectLine), public leo::Singleton < EffectLineDelegate >
	{
	public:
		EffectLineDelegate(ID3D11Device* device)
			:mVertexConstantBufferPerCamera(device), mPixelConstantBufferPerColor(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(L"Shader\\LineVS.cso", nullptr, InputLayoutDesc::Sky, 1, &layout);
			mPixelShader = sm.CreatePixelShader(L"Shader\\LinePS.cso");
		}
		~EffectLineDelegate()
		{}
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"line");
			pContext.VSSetShader(mVertexShader, nullptr, 0);
			mVertexConstantBufferPerCamera.Update(context);
			pContext.VSSetConstantBuffers(mVertexConstantBufferPerCamera.slot, 1, &mVertexConstantBufferPerCamera.mBuffer);
			
			pContext.PSSetShader(mPixelShader, nullptr, 0);
			mPixelConstantBufferPerColor.Update(context);
			pContext.PSSetConstantBuffers(0, 1, &mPixelConstantBufferPerColor.mBuffer);
		}

		void ViewProj(CXMMATRIX matrix, ID3D11DeviceContext* context)
		{
			mVertexConstantBufferPerCamera.gViewProj = XMMatrixTranspose(matrix);
			if (context)
				mVertexConstantBufferPerCamera.Update(context);
		}

		void Color(const float4& color, ID3D11DeviceContext* context = nullptr)
		{
			memcpy(mPixelConstantBufferPerColor.gColor, color);
			if (context)
				mPixelConstantBufferPerColor.Update(context);
		}
	public:
		struct VScbPerCamera
		{
			XMMATRIX gViewProj;
		public:
			static const std::uint8_t slot = 0;
		};
		struct PScbPerColor
		{
			float4 gColor;
		};
		ShaderConstantBuffer<VScbPerCamera> mVertexConstantBufferPerCamera;
		ShaderConstantBuffer<PScbPerColor> mPixelConstantBufferPerColor;

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;
	};


	void EffectLine::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectLineDelegate*>(this));

		return ((EffectLineDelegate*)this)->Apply(
			context
			);
	}

	void EffectLine::ViewProj(CXMMATRIX matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectLineDelegate*>(this));

		return ((EffectLineDelegate*)this)->ViewProj(
			matrix,context
			);
	}

	void EffectLine::Color(const float4& color, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectLineDelegate*>(this));

		return ((EffectLineDelegate*)this)->Color(
			color, context
			);
	}

	const std::unique_ptr<EffectLine>& EffectLine::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = unique_raw<EffectLine>(new EffectLineDelegate(device));
		return mInstance;
	}
}