#include "..\d3dx11.hpp"
#include "Terrain.Tessation.Effect.hpp"
#include "..\IndePlatform\Singleton.hpp"
#include "..\IndePlatform\memory.hpp"
#include "..\RenderStates.hpp"
#include "..\ShaderMgr.h"
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
			:mCBParams(device), mCBPerHardWare(device),
			mVSCBSizeOnset(device),
			mHSCBPerMatrix(device)
		{
			leo::RenderStates sss;
			CD3D11_SAMPLER_DESC LinearClamp;
			LinearClamp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			LinearClamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			LinearClamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			mSamplerClampLinear = sss.CreateSamplerState(L"LinearClamp", LinearClamp);

			leo::ShaderMgr sm;
			ID3D11InputLayout* mLayout;
			mHwTessellationVS = sm.CreateVertexShader(L"Shader\\TerrainTessation.VS.cso", nullptr, InputLayoutDesc::Terrain, 5, &mLayout);

			mHwTessellationHS = sm.CreateHullShader(L"Shader\\TerrainTessation.HS.cso", nullptr);
		}
		~TerrainTessationEffectDelegate()
		{}
	public:
		void Apply(ID3D11DeviceContext* context);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;
		//Common
		void SetCoarseHeightMap(ID3D11ShaderResourceView* srv);
		void SetTexureOffset(const float3& offset, ID3D11DeviceContext* context);
		void SetDetailNoiseScale(const float& scale, ID3D11DeviceContext* context);
		void SetDetailUVScale(const float2& scale, ID3D11DeviceContext* context);
		void SetCoarseSampleSpacing(const float& space, ID3D11DeviceContext* context);
		void SetDisplacementHeight(const float& height, ID3D11DeviceContext* context);
		//EndCommon

		//VertextShader
		void SetTileSize(const float& size, ID3D11DeviceContext* context);
#ifdef DEBUG
		void SetShowTiles(bool enable, ID3D11DeviceContext* context);
#endif
		//EndVertexShader

		//HullShader
		void SetScreenSize(const float2& size, ID3D11DeviceContext* context);
#ifdef DEBUG
		//uint pixel
		void SetTriWidth(const int& width, ID3D11DeviceContext* context);
#endif
		void SetWolrdViewProj(const float4x4& matrix, ID3D11DeviceContext* context);
		void SetLodWorldView(const float4x4& matrix, ID3D11DeviceContext* context);
		void SetProj(const float4x4& matrix, ID3D11DeviceContext* context);
		void SetEyePos(const float3& pos, ID3D11DeviceContext* context);
		void SetEyeDir(const float3& pos, ID3D11DeviceContext* context);
		//EndHullShader
		
		void SetDetailNoiseTexure(ID3D11ShaderResourceView* srv);

		

#ifdef DEBUG
#endif
	private:
		//Common
		//slot 0
		ID3D11SamplerState* mSamplerClampLinear = nullptr;
		//slot 0
		ID3D11ShaderResourceView* mCoarseHeightMap = nullptr;
		struct CommonParamsOnSet
		{
			XMFLOAT3 gTextureWorldOffset;	// Offset of fractal terrain in texture space.
			float     gDetailNoiseScale = 0.2;
			XMFLOAT2    gDetailUVScale = XMFLOAT2(1.f,1.f);				// x is scale; y is 1/scale
			float  gCoarseSampleSpacing;
			float gfDisplacementHeight;
			const static uint8 slot = 0;
		};
		ShaderConstantBuffer<CommonParamsOnSet> mCBParams;
		//EndCommon

		//VertexShader
		ID3D11VertexShader* mHwTessellationVS = nullptr;
		struct VsSizeOnSet
		{
			float gTileSize = 1;
#ifdef DEBUG
			bool gDebugShowTiles = false;
#else
			float pad;
#endif
			XMFLOAT2 pad2;
			const static uint32 slot = 1;
		};
		ShaderConstantBuffer<VsSizeOnSet> mVSCBSizeOnset;
		//EndVertexShader

		//HullShader
		ID3D11HullShader* mHwTessellationHS = nullptr;
		struct PerHardWare
		{
			//Render target size for screen-space calculations
			XMFLOAT2 gScreenSize;
			//Pixels per tri edge
			int gTessellatedTriWidth = 10;
			int pad;
			const static uint8 slot = 1;
		};
		ShaderConstantBuffer<PerHardWare> mCBPerHardWare;
		struct HsPerMatrix
		{
			XMMATRIX gWorldViewProj;

			XMMATRIX gWorldViewLOD;
			XMMATRIX gWorldViewProjLOD;
			//The proj matrix does not vary between the LOD and view-centre vsets.  
			//Only the view matrix varies
			XMMATRIX gProj;

			pack_type<float3> gEyePos;
			pack_type<float3> gViewDir;
			const static uint8 slot = 2;
		};
		ShaderConstantBuffer<HsPerMatrix> mHSCBPerMatrix;
		//EndHullShader

		//DomainShader
		struct DSPerCamera
		{
			XMMATRIX gWorldViewProj;
			const static uint8 slot = 0;
		};

#ifdef DEBUG
		struct DSDebugOnSet 
		{
			int32 gDebugShowPatches[4];
			const static uint8 slot = 1;
		};
#endif
		//EndDomainShader
		//in vs,s0,common
		ID3D11SamplerState* mSamplerRepeatLinera = nullptr;
		//in vs,t1
		
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