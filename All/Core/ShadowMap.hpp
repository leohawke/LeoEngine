//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/ShadowMap.hpp
//  Version:     v1.00
//  Created:     12/08/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: ÒõÓ°Ô¤ÏÈ²Ù×÷
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#include "..\IndePlatform\utility.hpp"
#include "..\IndePlatform\Singleton.hpp"

struct ID3D11Device;
struct ID3D11ShaderResourceView;
struct ID3D11DeviceContext;

namespace leo {
	class CastShadowCamera;
	//!\store scene depth
	class LB_API ShadowMap : ABSTRACT {
	public:
		ID3D11ShaderResourceView* GetDepthSRV();
		void BeginShadowMap(ID3D11DeviceContext*,const CastShadowCamera&);
		void EndShadowMap(ID3D11DeviceContext*);

		static ShadowMap& GetInstance(ID3D11Device* device, std::pair<uint16, uint16> resolution);

	};
}
