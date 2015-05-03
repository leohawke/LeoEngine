#include "EffectTerrain.hpp"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"
#include "Terrain.hpp"
#include "FileSearch.h"
#include "EngineConfig.h"
namespace leo
{
	using vector = __m128;
	using matrix = std::array<vector, 4>;

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
			mVS = SM.CreateVertexShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"terrain", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::Terrain, 1, &mLayout);
			mPS = SM.CreatePixelShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"terrain", D3D11_PIXEL_SHADER)));

			leo::RenderStates SS;
			mLinearClamp = SS.GetSamplerState(L"LinearClamp");

			mLinearRepeat = SS.GetSamplerState(L"LinearRepeat");

			normalSampler = SS.GetSamplerState(L"trilinearSampler");

			leo::TextureMgr texmgr;
			texNormals = texmgr.LoadTextureSRV(FileSearch::Search(L"NormalsFitting.dds"));
		}
		void Apply(ID3D11DeviceContext* con)
		{
			context_wrapper context(con);

			mVSCBPerMatrix.Update(con);

			context.VSSetShader(mVS, nullptr, 0);

			context.VSSetConstantBuffers(0, 1, &mVSCBPerMatrix.mBuffer);
			context->VSSetShaderResources(0, 1, &mHeightSRV);
			context->VSSetSamplers(0, 1, &mLinearClamp);

			context.PSSetShader(mPS, nullptr, 0);
#ifdef DEBUG
			context.PSSetConstantBuffers(0, 1, &mPSCBPerLodColor.mBuffer);
#endif
			

			ID3D11SamplerState* mPSSSs[] = { mLinearRepeat,normalSampler };

			context.PSSetSamplers(0, 2, mPSSSs);
			ID3D11ShaderResourceView* mArray[] = { mWeightSRV, mPSSRVArray,texNormals };
			context.PSSetShaderResources(0,arrlen(mArray), mArray);
		}
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
		void ViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context )
		{
			mVSCBPerMatrix.gViwProj =Transpose(load(matrix));
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
			mHeightSRV = srv;
			if (context)
				context->VSSetShaderResources(0, 1, &mHeightSRV);
		}

		void WeightMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context) {
			mWeightSRV = srv;
			if (context)
				context->PSSetShaderResources(0, 1, &mWeightSRV);
		}

		void MatArrayMap(ID3D11ShaderResourceView* srv, ID3D11DeviceContext * context)
		{
			mPSSRVArray = srv;
			if (context)
				context->PSSetShaderResources(1, 1, &mPSSRVArray);
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
			matrix gViwProj;
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
		ID3D11SamplerState* mLinearClamp = nullptr;

		ID3D11ShaderResourceView* mHeightSRV = nullptr;


		ID3D11ShaderResourceView* mWeightSRV = nullptr;
		ID3D11ShaderResourceView* mPSSRVArray = nullptr;

		ID3D11SamplerState* mLinearRepeat = nullptr;

		ID3D11ShaderResourceView *texNormals = nullptr;

		ID3D11SamplerState* normalSampler = nullptr;
	};
	void EffectTerrain::Apply(ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->Apply(
			context
			);
	}
	void EffectTerrain::ViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext * context)
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
	
	void leo::EffectTerrain::MatArrayMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->MatArrayMap(
			srv, context
			);
	}

	void leo::EffectTerrain::WeightMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context)
	{
		lassume(dynamic_cast<EffectTerrainDelegate *>(this));

		return ((EffectTerrainDelegate *)this)->WeightMap(
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