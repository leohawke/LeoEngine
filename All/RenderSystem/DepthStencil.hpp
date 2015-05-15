////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   ShaderSystem/DepthStencil.hpp
//  Version:     v1.00
//  Created:     05/15/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 深度模板类
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////


#ifndef ShaderSystem_Deferred_H
#define ShaderSystem_Deferred_H

#include "Core\COM.hpp"

struct ID3D11Device;

struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

struct DXGI_SAMPLE_DESC;

namespace leo {
	class DepthStencil {
	public:
		DepthStencil(ID3D11Device* device) noexcept;

		DepthStencil(ID3D11Device* device,DXGI_SAMPLE_DESC sampleDesc);

		~DepthStencil();

		operator ID3D11DepthStencilView*() const;

		ID3D11ShaderResourceView* GetDepthSRV() const;

	private:
		win::unique_com<ID3D11DepthStencilView> mDepthStencilView = nullptr;
		win::unique_com<ID3D11ShaderResourceView> mDepthSRV = nullptr;
	};
}

#endif
