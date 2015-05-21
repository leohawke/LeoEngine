//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/EffectQuad.hpp
//  Version:     v1.00
//  Created:     05/21/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: Quad的VB,Layout,VS存放在此!
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_EffectQuad_Hpp
#define Core_EffectQuad_Hpp

#include "effect.h"

//test operator= .copy ctor,move ctor(accept)

namespace leo{

	class CameraFrustum;

class EffectQuad :public Effect, ABSTRACT
{
public:
	void Apply(ID3D11DeviceContext* context);

	void Draw(ID3D11DeviceContext* context);

	void SetFrustum(ID3D11Device* device,const leo::CameraFrustum& frustum);

	bool SetLevel(EffectConfig::EffectLevel l) noexcept
	{
		return true;
	}
public:
	static EffectQuad& GetInstance(ID3D11Device* device = nullptr);
};
}
#endif
