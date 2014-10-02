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
		{
			leo::ShaderMgr SM;
			ID3D11InputLayout* mLayout = nullptr;
			mVS = SM.CreateVertexShader(L"Shader\\TerrainVS.cso", nullptr, InputLayoutDesc::Terrain, 1, &mLayout);
			mPS = SM.CreatePixelShader(L"Shader\\TerrainPS.cso");

			leo::RenderStates SS;
			mSS = SS.GetSamplerState(L"LinearClamp");
		}
		void Apply(ID3D11DeviceContext* con)
		{
			context_wrapper context(con);

			context.VSSetShader(mVS, nullptr, 0);
			mVSCBPerMatrix.Update(con);
			context.VSSetConstantBuffers(0, 1, &mVSCBPerMatrix.mBuffer);
			context.PSSetShader(mPS, nullptr, 0);
			context.PSSetShaderResources(0, 1, &mSRV);
			context.PSSetSamplers(0, 1, &mSS);
		}
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
		void ViewProjMatrix(CXMMATRIX matrix, ID3D11DeviceContext* context )
		{
			mVSCBPerMatrix.gViwProj = XMMatrixTranspose(matrix);
			if (context)
				mVSCBPerMatrix.Update(context);
		}
		void WorldOffset(const float3& offset, ID3D11DeviceContext* context)
		{
			mVSCBPerMatrix.gOffset = offset;
			if (context)
				mVSCBPerMatrix.Update(context);
		}

		void HeightMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context)
		{
			mSRV = srv;
			if (context)
				context->PSSetShaderResources(0, 1, &mSRV);
		}
	public:
	private:
		struct vsCBMatrix
		{
			XMMATRIX gViwProj;
			float3 gOffset;
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<vsCBMatrix> mVSCBPerMatrix;

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
	void EffectTerrain::WorldOffset(const float3 & offset, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->WorldOffset(
			offset, context
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

	const std::unique_ptr<EffectTerrain>& EffectTerrain::GetInstance(ID3D11Device * device)
	{
		static auto mInstance = std::unique_ptr<EffectTerrain>(new EffectTerrainDelegate(device));
		return mInstance;
	}
}