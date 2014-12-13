//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Terraineffect.h
//  Version:     v1.00
//  Created:     12/13/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供阴影渲染接口
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_ShadowMap_Effect_hpp
#define Core_ShadowMap_Effect_hpp

#include "effect.h"

namespace leo {
	class EffectShadowMap :public Effect, ABSTRACT {
	public:
		void Apply(ID3D11DeviceContext* context);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;

		void WorldMatrix(const float4x4& matrix,ID3D11DeviceContext* context = nullptr);
		void ViewProjTexMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);
		static const std::unique_ptr<EffectShadowMap>& GetInstance(ID3D11Device* device = nullptr);
	};
}

#endif
