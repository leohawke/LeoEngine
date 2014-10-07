#include "EffectTerrain.hpp"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"
#include "Terrain.hpp"
namespace leo
{
	class EffectTerrainDelegate :CONCRETE(EffectTerrain), public Singleton<EffectTerrainDelegate>
	{
	public:
		EffectTerrainDelegate(ID3D11Device* device)
			:mVSCBPerMatrix(device)
#ifdef DEBUG
			, mPSCBPerLodColor(device)
#endif
		{
			leo::ShaderMgr SM;
			ID3D11InputLayout* mLayout = nullptr;
			mVS = SM.CreateVertexShader(L"Shader\\TerrainVS.cso", nullptr, InputLayoutDesc::Terrain, 1, &mLayout);
			mPS = SM.CreatePixelShader(L"Shader\\TerrainPS.cso");

			leo::RenderStates SS;
			mSS = SS.GetSamplerState(L"NearestRepeat");
		}
		void Apply(ID3D11DeviceContext* con)
		{
			context_wrapper context(con);

			context.VSSetShader(mVS, nullptr, 0);
			mVSCBPerMatrix.Update(con);
			context.VSSetConstantBuffers(0, 1, &mVSCBPerMatrix.mBuffer);
			context.PSSetShader(mPS, nullptr, 0);
			context.PSSetConstantBuffers(0, 1, &mPSCBPerLodColor.mBuffer);
			context->VSSetShaderResources(0, 1, &mSRV);
			context->VSSetSamplers(0, 1, &mSS);
		}
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
		void ViewProjMatrix(CXMMATRIX matrix, ID3D11DeviceContext* context )
		{
			mVSCBPerMatrix.gViwProj =XMMatrixTranspose(matrix);
			if (context)
				mVSCBPerMatrix.Update(context);
		}
		void WorldOffset(const float2& offset, ID3D11DeviceContext* context)
		{
			mVSCBPerMatrix.gOffsetUVScale.x = offset.x;
			mVSCBPerMatrix.gOffsetUVScale.y = offset.y;

			if (context)
				mVSCBPerMatrix.Update(context);
		}
		void UVScale(const float2& scale, ID3D11DeviceContext* context)
		{
			mVSCBPerMatrix.gOffsetUVScale.z = scale.x;
			mVSCBPerMatrix.gOffsetUVScale.w = scale.y;

			if (context)
				mVSCBPerMatrix.Update(context);
		}
		void HeightMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context)
		{
			mSRV = srv;
			if (context)
				context->PSSetShaderResources(0, 1, &mSRV);
		}

#ifdef DEBUG
		void LodColor(const float4& color, ID3D11DeviceContext* context)
		{
			mPSCBPerLodColor.gColor = color;
			mPSCBPerLodColor.Update(context);
		}
#endif
	public:
	private:
		struct vsCBMatrix
		{
			XMMATRIX gViwProj;
			float4 gOffsetUVScale;
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<vsCBMatrix> mVSCBPerMatrix;

#ifdef DEBUG
		struct psLodColor
		{
			float4 gColor;
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<psLodColor> mPSCBPerLodColor;
#endif

		ID3D11VertexShader* mVS = nullptr;
		ID3D11PixelShader* mPS = nullptr;
		ID3D11SamplerState* mSS = nullptr;
		ID3D11ShaderResourceView* mSRV = nullptr;
	};
	void EffectTerrain::Apply(ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->Apply(
			context
			);
	}
	void EffectTerrain::ViewProjMatrix(CXMMATRIX matrix, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->ViewProjMatrix(
			matrix,context
			);
	}
	void EffectTerrain::WorldOffset(const float2 & offset, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->WorldOffset(
			offset, context
			);
	}

	void EffectTerrain::UVScale(const float2& scale, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->UVScale(
			scale, context
			);
	}
	void leo::EffectTerrain::HeightMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->HeightMap(
			srv, context
			);
	}
	
	bool EffectTerrain::SetLevel(EffectConfig::EffectLevel l) lnothrow
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->SetLevel(
			l
			);
	}
#ifdef DEBUG
	void EffectTerrain::LodColor(const float4& color, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->LodColor(
			color,context
			);
	}
#endif
	const std::unique_ptr<EffectTerrain>& EffectTerrain::GetInstance(ID3D11Device * device)
	{
		static auto mInstance = std::unique_ptr<EffectTerrain>(new EffectTerrainDelegate(device));
		return mInstance;
	}
}