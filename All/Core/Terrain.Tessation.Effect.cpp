#include "..\d3dx11.hpp"
#include "Terrain.Tessation.Effect.hpp"
#include "..\IndePlatform\Singleton.hpp"
#include "..\IndePlatform\memory.hpp"
namespace leo
{
	namespace InputLayoutDesc
	{
		extern const D3D11_INPUT_ELEMENT_DESC Terrain[5] =
		{
			{ "POSITION_2D", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "ADJACENCY_SIZES", 0, DXGI_FORMAT_R32_FLOAT, 0, sizeof(float2), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "ADJACENCY_SIZES", 1, DXGI_FORMAT_R32_FLOAT, 0, sizeof(float2)+4, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "ADJACENCY_SIZES", 2, DXGI_FORMAT_R32_FLOAT, 0, sizeof(float2)+8, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "ADJACENCY_SIZES", 3, DXGI_FORMAT_R32_FLOAT, 0, sizeof(float2)+12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}
}

namespace leo
{
	class TerrainTessationEffectDelegate : CONCRETE(TerrainTessationEffect), public Singleton < TerrainTessationEffectDelegate >
	{
	public:
		TerrainTessationEffectDelegate(ID3D11Device* device)
		{

		}
		~TerrainTessationEffectDelegate()
		{}
	public:
		void Apply(ID3D11DeviceContext* context);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;

		void SetCoarseHeightMap(ID3D11ShaderResourceView* srv);
		void SetDetailNoiseTexure(ID3D11ShaderResourceView* srv);

		void SetTexureOffset(const float3& offset, ID3D11DeviceContext* context);
		void SetDetailNoiseScale(const float& scale, ID3D11DeviceContext* context);
		void SetDetailUVScale(const float2& scale, ID3D11DeviceContext* context);
		void SetTileSize(const float& size, ID3D11DeviceContext* context);

#ifdef DEBUG
#endif
	private:
		//in vs,s1
		ID3D11SamplerState* mSamplerClampLinear = nullptr;
		//in vs,s0,common
		ID3D11SamplerState* mSamplerRepeatLinera = nullptr;
		//in vs,t1
		ID3D11ShaderResourceView* mCoarseHeightMap = nullptr;
		//in vs,t0,common
		ID3D11ShaderResourceView* mDetailNoiseTexture = nullptr;

		struct VscbScalePerSet
		{
			XMFLOAT3 gTexureWolrdOffset;
			float gDetailNoiseScale = 0.2;
			XMFLOAT2 gDetailUVScale = XMFLOAT2(1.f, 1.f);
			float pod;
			const static uint32 slot = 0;
		};

		struct VscbSizeAndTexOffsetPerSet
		{
			float gTileSize = 1;
			const static uint32 slot = 1;
		};

#ifdef DEBUG
		struct VscbDebugPerSet
		{
			int32 gDebugShowTiles[4];
			const static uint32 slot = 2;
		};
#endif
	};

	const std::unique_ptr<TerrainTessationEffect>& TerrainTessationEffect::GetInstance(ID3D11Device* device)
	{
		auto mInstance = unique_raw<TerrainTessationEffect>(new TerrainTessationEffectDelegate(device));
		return mInstance;
	}

	void TerrainTessationEffect::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->Apply(
			context
			);
	}

	bool TerrainTessationEffect::SetLevel(EffectConfig::EffectLevel l) lnothrow
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetLevel(
			l
			);
	}
}