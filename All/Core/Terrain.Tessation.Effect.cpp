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
			mHSCBPerMatrix(device),
			mDSCBPerCamera(device), 
			mPSCBPerSet(device),
#ifdef DEBUG
			mDSCBPerDebug(device)
#endif
		{
			leo::RenderStates sss;
			CD3D11_SAMPLER_DESC LinearClamp;
			LinearClamp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			LinearClamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			LinearClamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			mSamplerClampLinear = sss.CreateSamplerState(L"LinearClamp", LinearClamp);

			CD3D11_SAMPLER_DESC LinearRepeat;
			LinearRepeat.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			LinearClamp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			LinearClamp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			mSamplerRepeatLinear = sss.CreateSamplerState(L"LinearRepeat", LinearRepeat);

			leo::ShaderMgr sm;
			ID3D11InputLayout* mLayout;
			mHwTessellationVS = sm.CreateVertexShader(L"Shader\\TerrainTessation.VS.cso", nullptr, InputLayoutDesc::Terrain, 5, &mLayout);

			mHwTessellationHS = sm.CreateHullShader(L"Shader\\TerrainTessation.HS.cso", nullptr);
			mHwTessellationDS = sm.CreateDomainShader(L"Shader\\TerrainTessation.DS.cso", nullptr);
			mHwTessellationPS = sm.CreatePixelShader(L"Shader\\TerrainTessation.PS.cso", nullptr);

			CD3D11_SAMPLER_DESC MaxAnisoRepeat;
			MaxAnisoRepeat.Filter = D3D11_FILTER_ANISOTROPIC;
			MaxAnisoRepeat.MaxAnisotropy = 16;
			MaxAnisoRepeat.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			MaxAnisoRepeat.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			mSamplerRepeatMaxAniso = sss.CreateSamplerState(L"MaxAnisoRepeat",MaxAnisoRepeat);
			MaxAnisoRepeat.MaxAnisotropy = 4;
			mSamplerRepeatMedAniso = sss.CreateSamplerState(L"MedAnisoRepeat", MaxAnisoRepeat);

			LinearRepeat.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			//Point => Nearest
			mSamplerRepeatPoint = sss.CreateSamplerState(L"NearestRepeat", LinearRepeat);
		}
		~TerrainTessationEffectDelegate()
		{}
	public:
		void Apply(ID3D11DeviceContext* context);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;
		//Common
		void SetCoarseHeightMap(ID3D11ShaderResourceView* srv);
		void SetDetailNoiseTexture(ID3D11ShaderResourceView* srv);

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
		void SetEyeDir(const float3& dir, ID3D11DeviceContext* context);
		//EndHullShader
#ifdef DEBUG
		void SetDebugShowPatches(bool enable, ID3D11DeviceContext* context);
#endif

		void SetTerrainColorTextures(ID3D11ShaderResourceView* srv0, ID3D11ShaderResourceView* srv1);
		void SetDetailNoiseGradTexture(ID3D11ShaderResourceView* srv);
		void SetCoarseGradientMap(ID3D11ShaderResourceView* srv);
		void SetNoiseTexture(ID3D11ShaderResourceView* srv);

		void SetFractalOctaves(const float3& octs, ID3D11DeviceContext* context);
	private:
		//Common
		//slot 0
		ID3D11SamplerState* mSamplerClampLinear = nullptr;
		//slot 0
		ID3D11ShaderResourceView* mCoarseHeightMap = nullptr;
		ID3D11SamplerState* mSamplerRepeatLinear = nullptr;
		ID3D11ShaderResourceView* mDetailNoiseTexture = nullptr;
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
			float gDebugShowTiles;
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
		ID3D11DomainShader* mHwTessellationDS = nullptr;
		struct DSPerCamera
		{
			XMMATRIX gWorldViewProj;
			const static uint8 slot = 0;
		};
		ShaderConstantBuffer<DSPerCamera> mDSCBPerCamera;
#ifdef DEBUG
		struct DSDebugOnSet 
		{
			int32 gDebugShowPatches[4];
			const static uint8 slot = 1;
		};
		ShaderConstantBuffer<DSDebugOnSet> mDSCBPerDebug;
#endif
		//EndDomainShader

		//PixelShader
		ID3D11PixelShader* mHwTessellationPS = nullptr;
		//s2
		ID3D11SamplerState* mSamplerRepeatMaxAniso = nullptr;
		//s3
		ID3D11SamplerState* mSamplerRepeatMedAniso = nullptr;
		//Point => Nearest s4
		ID3D11SamplerState* mSamplerRepeatPoint = nullptr;
		
		ID3D11ShaderResourceView* mTerrainColorTexture1 = nullptr;
		ID3D11ShaderResourceView* mTerrainColorTexture2 = nullptr;
		ID3D11ShaderResourceView* mDetailNoiseGradTexture = nullptr;
		ID3D11ShaderResourceView* mCoarseGradientMap = nullptr;
		ID3D11ShaderResourceView* mNoiseTexture = nullptr;

		struct PSParamOnSet
		{
			XMFLOAT3 gFractalOctaves;
#ifdef DEBUG
			bool gDebugShowPatches = false;
#else
			float gDebugShowPatches;
#endif
			const static uint32 slot = 1;
		};
		ShaderConstantBuffer<PSParamOnSet> mPSCBPerSet;
		//EndPixelShader
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

		//Common
	void TerrainTessationEffect::SetCoarseHeightMap(ID3D11ShaderResourceView* srv){
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetCoarseHeightMap(
			srv
			);
	}

	void TerrainTessationEffect::SetDetailNoiseTexture(ID3D11ShaderResourceView* srv)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetDetailNoiseTexture(
			srv
			);
	}

	void TerrainTessationEffect::SetTexureOffset(const float3& offset, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetTexureOffset(
			offset,context
			);
	}
	
	void TerrainTessationEffect::SetDetailNoiseScale(const float& scale, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetDetailNoiseScale(
			scale,context
			);
	}
	
	void TerrainTessationEffect::SetDetailUVScale(const float2& scale, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetDetailUVScale(
			scale,context
			);
	}

	void TerrainTessationEffect::SetCoarseSampleSpacing(const float& space, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetCoarseSampleSpacing(
			space,context
			);
	}

	void TerrainTessationEffect::SetDisplacementHeight(const float& height, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetDisplacementHeight(
			height,context
			);
	}
	//EndCommon

	//VertextShader
	void TerrainTessationEffect::SetTileSize(const float& size, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetTileSize(
			size,context
			);
	}

#ifdef DEBUG
	void TerrainTessationEffect::SetShowTiles(bool enable, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetShowTiles(
			enable,context
			);
	}
#endif
	//EndVertexShader

	//HullShader
	void TerrainTessationEffect::SetScreenSize(const float2& size, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetScreenSize(
			size,context
			);
	}
#ifdef DEBUG
	//uint pixel
	void TerrainTessationEffect::SetTriWidth(const int& width, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetTriWidth(
			width,context
			);
	}
#endif
	void TerrainTessationEffect::SetWolrdViewProj(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetWolrdViewProj(
			matrix,context
			);
	}
	void TerrainTessationEffect::SetLodWorldView(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetLodWorldView(
			matrix, context
			);
	}
	void TerrainTessationEffect::SetProj(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetProj(
			matrix, context
			);
	}
	void TerrainTessationEffect::SetEyePos(const float3& pos, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetEyePos(
			pos,context
			);
	}
	void TerrainTessationEffect::SetEyeDir(const float3& dir, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetEyeDir(
			dir,context
			);
	}
	//EndHullShader
#ifdef DEBUG
	void TerrainTessationEffect::SetDebugShowPatches(bool enable, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetDebugShowPatches(
			enable,context
			);
	}
#endif

	void TerrainTessationEffect::SetTerrainColorTextures(ID3D11ShaderResourceView* srv0, ID3D11ShaderResourceView* srv1)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetTerrainColorTextures(
			srv0,srv1
			);
	}
	void TerrainTessationEffect::SetDetailNoiseGradTexture(ID3D11ShaderResourceView* srv)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetDetailNoiseGradTexture(
			srv
			);
	}
	void TerrainTessationEffect::SetCoarseGradientMap(ID3D11ShaderResourceView* srv)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetCoarseGradientMap(
			srv
			);
	}
	void TerrainTessationEffect::SetNoiseTexture(ID3D11ShaderResourceView* srv)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetNoiseTexture(
			srv
			);
	}

	void TerrainTessationEffect::SetFractalOctaves(const float3& octs, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<TerrainTessationEffectDelegate*>(this));

		return ((TerrainTessationEffectDelegate*)this)->SetFractalOctaves(
			octs,context
			);
	}
}