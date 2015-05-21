////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   ShaderSystem/DeferredRender.h
//  Version:     v1.00
//  Created:     05/16/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: —”≥Ÿ‰÷»æ
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef ShaderSystem_DeferredRender_Hpp
#define ShaderSystem_DeferredRender_Hpp

#include <memory>
#include <leoint.hpp>
#include "DepthStencil.hpp"
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
namespace leo {
	class DeferredRender {
	private:
		class DeferredResImpl;
		class DeferredStateImpl;
		std::unique_ptr<DeferredResImpl> pResImpl;
		std::unique_ptr<DeferredStateImpl> pStateImpl;

	public:
		using size_type = std::pair<uint16, uint16>;

		DeferredRender(ID3D11Device* device, size_type size);

		~DeferredRender();

		void OMSet(ID3D11DeviceContext* context,DepthStencil& depthstencil) noexcept;

		void UnBind(ID3D11DeviceContext* context,DepthStencil& depthstencil) noexcept;

		void ReSize(ID3D11Device* device,size_type size) noexcept;

		void LinearizeDepth(ID3D11DeviceContext* context, DepthStencil& depthstencil,float near_z,float far_z) noexcept;
		
		void SetSSAOParams(bool enable, uint8 level) noexcept;

		ID3D11ShaderResourceView* GetLinearDepthSRV() const noexcept;

		ID3D11RenderTargetView* GetLightRTV() const	noexcept;
		ID3D11ShaderResourceView* GetNormalAlphaSRV() const noexcept;

	public:
		class SSAO {
			class DeferredSSAOImpl;
			std::unique_ptr<DeferredSSAOImpl> pTecImpl;
			~SSAO();
		};
	};
}

#endif
