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


#ifndef ShaderSystem_DepthStencil_Hpp
#define ShaderSystem_DepthStencil_Hpp

#include "Core\COM.hpp"

struct ID3D11Device;

struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

struct DXGI_SAMPLE_DESC;

namespace leo {
	class DepthStencil {
	public:
		DepthStencil(std::pair<uint16,uint16> size,ID3D11Device* device);

		DepthStencil(std::pair<uint16, uint16> size,ID3D11Device* device,DXGI_SAMPLE_DESC sampleDesc);

		~DepthStencil();

		operator ID3D11DepthStencilView*() const noexcept;

		ID3D11ShaderResourceView* GetDepthSRV() const noexcept;

		void ReSize(std::pair<uint16, uint16> size, ID3D11Device* device);

	private:
		win::unique_com<ID3D11DepthStencilView> mDepthStencilView = nullptr;
		win::unique_com<ID3D11ShaderResourceView> mDepthSRV = nullptr;
	};
}

#endif
