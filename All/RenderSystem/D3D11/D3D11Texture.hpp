////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   RenderSystem/D3D11/D3D11Texture.hpp
//  Version:     v1.00
//  Created:     12/13/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: D3D11纹理对象
// -------------------------------------------------------------------------
//  History:
////////////////////////////////////////////////////////////////////////////

#ifndef D3D11_Texture_Hpp
#define D3D11_Texture_Hpp

#include "../Texture.hpp"
#include "../d3dx11.hpp"


namespace leo {

	EFormat Format(DXGI_FORMAT);
	DXGI_FORMAT Format(EFormat);


	/*!
	\brief 一个D3D11Texture返回的D3D资源是被D3D11Texture独占的，故返回资源裸指针
	\warning 给D3D设置资源会增加其引用计数，在D3D使用这个资源的时段，必须保证其所属的D3D11Texture存在
	\warning 改类不持有D3D11Device与D3D11DeviceContext
	*/
	class D3D11Texture :public Texture
	{
	public:
		explicit D3D11Texture(Dis_Type type, uint32 access, SampleDesc sample_info = {});

		virtual ~D3D11Texture();

		std::string const & Name() const override;

		uint16 Width(uint8 /*level*/) const override;
		uint16 Height(uint8 /*level*/) const override;
		uint16 Depth(uint8 /*level*/) const override;

		virtual ID3D11Resource* Resource() const = 0;

		virtual ID3D11ShaderResourceView* ResourceView();
		virtual ID3D11UnorderedAccessView* AccessView();
		virtual ID3D11RenderTargetView* TargetView();
		virtual ID3D11DepthStencilView* DepthStencilView();
	protected:
		void D3DFlags(D3D11_USAGE& usage, UINT& bind_flags, UINT& cpu_access, UINT& misc_flags) {
			//D3D11_USAGE
			if (Access() & EA_I)
			{
				usage = D3D11_USAGE_IMMUTABLE;
			}
			else
			{
				if ((EA_C_W | EA_G_R) == Access())
				{
					usage = D3D11_USAGE_DYNAMIC;
				}
				else
				{
					if (EA_C_W == Access())
					{
						if ((NumMipMaps() != 1) || (DT_Cube == Type()))
						{
							usage = D3D11_USAGE_STAGING;
						}
						else
						{
							usage = D3D11_USAGE_DYNAMIC;
						}
					}
					else
					{
						if (!(Access() & EA_C_R) && !(Access() & EA_C_W))
						{
							usage = D3D11_USAGE_DEFAULT;
						}
						else
						{
							usage = D3D11_USAGE_STAGING;
						}
					}
				}
			}

			bind_flags = 0;
			if ((Access() & EA_G_R) || (D3D11_USAGE_DYNAMIC == usage))
			{
				bind_flags |= D3D11_BIND_SHADER_RESOURCE;
			}
			if (Access() & EA_G_W)
			{
				if (IsDepthFormat(Format()))
				{
					bind_flags |= D3D11_BIND_DEPTH_STENCIL;
				}
				else
				{
					bind_flags |= D3D11_BIND_RENDER_TARGET;
				}
			}

	
			if (Access() & EA_G_U)
			{
				bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
			}
		

			cpu_access = 0;
			if (Access() & EA_C_R)
			{
				cpu_access |= D3D11_CPU_ACCESS_READ;
			}
			if (Access() & EA_C_W)
			{
				cpu_access |= D3D11_CPU_ACCESS_WRITE;
			}

			misc_flags = 0;
			if (Access() & EA_M)
			{
				misc_flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			}
		}

		ID3D11ShaderResourceView*   CreateD3DSRV(D3D11_SHADER_RESOURCE_VIEW_DESC const & desc);
		ID3D11UnorderedAccessView* CreateD3DUAV(D3D11_UNORDERED_ACCESS_VIEW_DESC const & desc);
		ID3D11RenderTargetView*  CreateD3DRTV(D3D11_RENDER_TARGET_VIEW_DESC const & desc);
		ID3D11DepthStencilView*  CreateD3DDSV(D3D11_DEPTH_STENCIL_VIEW_DESC const & desc);
	private:
		std::unordered_map<std::size_t, win::unique_com<ID3D11ShaderResourceView>> d3d_srv_maps;
		std::unordered_map<std::size_t, win::unique_com<ID3D11UnorderedAccessView>> d3d_uav_maps;
		std::unordered_map<std::size_t, win::unique_com<ID3D11RenderTargetView>> d3d_rtv_maps;
		std::unordered_map<std::size_t, win::unique_com<ID3D11DepthStencilView>> d3d_dsv_maps;
	};

	using D3D11TexturePtr = std::shared_ptr<D3D11Texture>;

	class D3D11Texture2D : public D3D11Texture {
	public:
		D3D11Texture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access, SampleDesc sample_info = {}, ElementInitData = {});

		~D3D11Texture2D();

		uint16 Width(uint8 /*level*/) const override;
		uint16 Height(uint8 /*level*/) const override;

		ID3D11Resource* Resource() const;

		ID3D11ShaderResourceView* ResouceView();
		ID3D11UnorderedAccessView* AccessView();
		ID3D11RenderTargetView* TargetView();
		ID3D11DepthStencilView* DepthStencilView();

		ID3D11Texture2D* D3DTexture() const;
	private:
		D3D11_TEXTURE2D_DESC mDesc;
		win::unique_com<ID3D11Texture2D> mTex;

		std::vector<uint16> mWidths;
		std::vector<uint16> mHeights;
	};
}

#endif
