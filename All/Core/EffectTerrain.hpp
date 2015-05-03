//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Terraineffect.h
//  Version:     v1.00
//  Created:     10/01/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供地形渲染接口
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Terrain_Effect_hpp
#define Core_Terrain_Effect_hpp

#include "effect.h"


namespace leo
{
	class EffectTerrain :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;
		void ViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);
		void WorldOffset(const float2& offset, ID3D11DeviceContext* context = nullptr);
		void UVScale(const float2& offset, ID3D11DeviceContext* context = nullptr);
		void HeightMap(ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context = nullptr);
#ifdef DEBUG
		void LodColor(const float4& color, ID3D11DeviceContext* context);

#endif
		void MatArrayMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context = nullptr);
		void WeightMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context = nullptr);

		void NormalMap(ID3D11ShaderResourceView * srv, ID3D11DeviceContext * context = nullptr);

	public:
		static const std::unique_ptr<EffectTerrain>& GetInstance(ID3D11Device* device = nullptr);
	};
}

#endif