#////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   RenderSystem/D3D11/D3D11RenderSystem.hpp
//  Version:     v1.00
//  Created:     12/13/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: D3D11 µœ÷≤„
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef ShaderSystem_D3D11_ShaderSystem_Hpp
#define ShaderSystem_D3D11_ShaderSystem_Hpp

#include "..\RenderSystem.hpp"

#include <d3d11.h>

namespace leo {
	struct D3D11Caps {
		bool full_npot_texture_support = true;
	};


	class D3D11RenderFactory :public RenderFactory {
	public:
		std::string const & Name() const override
		{
			static std::string const name("D3D11 Render Factory");
			return name;
		}

		TexturePtr MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size,
			EFormat format, SampleDesc sample_info, uint32 access, uint8 const * init_data) override;
	};

	class D3D11Engine :public RenderEngine {
	public:
		std::string const & Name() const override
		{
			static std::string const name("D3D11 Render Engine");
			return name;
		}

		RenderFactory& GetFactory() override {
			static D3D11RenderFactory mD3DFactory;
			return mD3DFactory;
		}

		D3D_FEATURE_LEVEL GetCoreFeatureLevel();

		const D3D11Caps& DeviceCaps() const {
			return mCaps;
		}
	private:
			D3D11Caps mCaps;
	};

	namespace D3D11Mapping {
		DXGI_FORMAT MappingFormat(EFormat format);
	}
}

#endif