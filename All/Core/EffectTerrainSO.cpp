#include "EffectTerrainSO.hpp"
#include "RenderSystem\ShaderMgr.h"
#include "RenderSystem\RenderStates.hpp"
#include "Vertex.hpp"
#include "FileSearch.h"
#include "EngineConfig.h"
namespace leo
{
	class EffectTerrainSODelegate :CONCRETE(EffectTerrainSO), public Singleton<EffectTerrainSODelegate>
	{
	public:
		EffectTerrainSODelegate(ID3D11Device* device)
			:mVSCBPerMatrix(device)
		{
			leo::ShaderMgr SM;
			ID3D11InputLayout* mLayout = nullptr;
			mVS = SM.CreateVertexShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"terrainso", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::Terrain, 1, &mLayout);
			D3D11_SO_DECLARATION_ENTRY pDecls[] = {
				{0,"HEIGHT",0,0,1,0},
				{0,"Id",0,0,1,0 }
			};
			UINT pBufferStides[] = { 8u };
			mGS = SM.CreateGeometryShaderWithSO(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"terrainso", D3D11_GEOMETRY_SHADER)),pDecls,2,pBufferStides,1,D3D11_SO_NO_RASTERIZED_STREAM);

			leo::RenderStates SS;
			mSS = SS.GetSamplerState(L"NearestRepeat");

		}
		void Apply(ID3D11DeviceContext* con)
		{
			context_wrapper context(con);

			context.VSSetShader(mVS, nullptr, 0);
			mVSCBPerMatrix.Update(con);
			context.VSSetConstantBuffers(0, 1, &mVSCBPerMatrix.mBuffer);
			context->VSSetShaderResources(0, 1, &mSRV);
			context->VSSetSamplers(0, 1, &mSS);

			context.GSSetShader(mGS, nullptr, 0);
			context.SOSetTargets(mNumBuffers, mppSOTargets, mpOffsets);
		}
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
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
				context->VSSetShaderResources(0, 1, &mSRV);
		}

		void SetSOTargets(UINT NumBuffers, ID3D11Buffer*const* ppSOTargets, UINT* pOffsets) {
			mNumBuffers = NumBuffers;
			mppSOTargets = ppSOTargets;
			mpOffsets = pOffsets;
		}
	public:
	private:
		struct vsCBMatrix
		{
			float4 gOffsetUVScale;
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<vsCBMatrix> mVSCBPerMatrix;


		ID3D11VertexShader* mVS = nullptr;
		ID3D11GeometryShader* mGS = nullptr;
		ID3D11SamplerState* mSS = nullptr;
		ID3D11ShaderResourceView* mSRV = nullptr;

		ID3D11ShaderResourceView* mWeightSRV = nullptr;

		UINT mNumBuffers = 0;
		ID3D11Buffer*const* mppSOTargets = nullptr;
		UINT* mpOffsets = nullptr;
	};
	void EffectTerrainSO::Apply(ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainSODelegate *>(this));

		return ((EffectTerrainSODelegate *)this)->Apply(
			context
			);
	}
	void EffectTerrainSO::WorldOffset(const float2 & offset, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainSODelegate *>(this));

		return ((EffectTerrainSODelegate *)this)->WorldOffset(
			offset, context
			);
	}

	void EffectTerrainSO::UVScale(const float2& scale, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectTerrainSODelegate *>(this));

		return ((EffectTerrainSODelegate *)this)->UVScale(
			scale, context
			);
	}
	void leo::EffectTerrainSO::HeightMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainSODelegate *>(this));

		return ((EffectTerrainSODelegate *)this)->HeightMap(
			srv, context
			);
	}

	bool EffectTerrainSO::SetLevel(EffectConfig::EffectLevel l) lnothrow
	{
		lassume(dynamic_cast<EffectTerrainSODelegate *>(this));

		return ((EffectTerrainSODelegate *)this)->SetLevel(
			l
			);
	}

	void EffectTerrainSO::SetSOTargets(UINT NumBuffers, ID3D11Buffer*const* ppSOTargets, UINT* pOffsets) {
		lassume(dynamic_cast<EffectTerrainSODelegate *>(this));

		return ((EffectTerrainSODelegate *)this)->SetSOTargets(
			NumBuffers,ppSOTargets,pOffsets
			);
	}

	const std::unique_ptr<EffectTerrainSO>& EffectTerrainSO::GetInstance(ID3D11Device * device)
	{
		static auto mInstance = std::unique_ptr<EffectTerrainSO>(new EffectTerrainSODelegate(device));
		return mInstance;
	}
}