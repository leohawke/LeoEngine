//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Terraineffect.h
//  Version:     v1.00
//  Created:     11/27/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供地形渲染流输出接口,用于查询高度
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_TerrainSO_Effect_hpp
#define Core_TerrainSO_Effect_hpp

#include "effect.h"


namespace leo
{
	class EffectTerrainSO :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;
		void WorldOffset(const float2& offset, ID3D11DeviceContext* context = nullptr);
		void UVScale(const float2& offset, ID3D11DeviceContext* context = nullptr);
		void HeightMap(ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context = nullptr);
		void SetSOTargets(UINT NumBuffers,ID3D11Buffer*const* ppSOTargets,UINT* pOffsets);
	public:
		static const std::unique_ptr<EffectTerrainSO>& GetInstance(ID3D11Device* device = nullptr);
	};
}

#endif