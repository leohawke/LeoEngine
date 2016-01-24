#include "D3D11Texture.hpp"
#include "D3D11RenderSystem.hpp"
#include "..\..\DeviceMgr.h"
#include <exception.hpp>

namespace {
	auto device = [] {return leo::DeviceMgr().GetDevice(); };
	auto context = [] {return leo::DeviceMgr().GetDeviceContext(); };
}

leo::D3D11Texture2D::D3D11Texture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access, SampleDesc sample_info, ElementInitData init_data)
	:D3D11Texture(DT_2D,access,sample_info)
{
	if (0 == numMipMaps)
	{
		numMipMaps = 1;
		uint32_t w = width;
		uint32_t h = height;
		while ((w != 1) || (h != 1))
		{
			++numMipMaps;

			w = std::max<uint32_t>(1U, w / 2);
			h = std::max<uint32_t>(1U, h / 2);
		}
	}
	mNumMipMaps = numMipMaps;

	auto & re = dynamic_cast<leo::D3D11Engine&>(leo::GetRenderEngine());
	if (re.GetCoreFeatureLevel() <= D3D_FEATURE_LEVEL_9_3)
	{
		if (!re.DeviceCaps().full_npot_texture_support
			&& (NumMipMaps() > 1) && (((width & (width - 1)) != 0) || ((height & (height - 1)) != 0)))
		{
			// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
			mNumMipMaps = 1;
		}

		if ((NumMipMaps() > 1) && IsCompressedFormat(format))
		{
			// height or width is not a multiply of 4 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
			uint32_t clamped_num_mip_maps;
			for (clamped_num_mip_maps = 0; clamped_num_mip_maps < NumMipMaps(); ++clamped_num_mip_maps)
			{
				uint32_t w = std::max<uint32_t>(1U, width >> clamped_num_mip_maps);
				uint32_t h = std::max<uint32_t>(1U, height >> clamped_num_mip_maps);
				if (((w & 0x3) != 0) || ((h & 0x3) != 0))
				{
					break;
				}
			}
			mNumMipMaps = clamped_num_mip_maps;
		}
	}

	mArraySize = array_size;
	mFormat = format;

	mWidth = width;
	mHeight = height;

	mDesc.Width = width;
	mDesc.Height = height;
	mDesc.MipLevels = NumMipMaps();
	mDesc.ArraySize = ArraySize();

	mDesc.Format = D3D11Mapping::MappingFormat(Format());
	mDesc.SampleDesc.Count = sample_info.Count;
	mDesc.SampleDesc.Quality = sample_info.Quality;

	D3DFlags(mDesc.Usage, mDesc.BindFlags, mDesc.CPUAccessFlags, mDesc.MiscFlags);
	ReclaimHWResource(init_data.data == nullptr? nullptr:&init_data);
}

using leo::D3D11Texture2D;
ImplDeDtor(D3D11Texture2D)

leo::uint16 leo::D3D11Texture2D::Width(uint8 level) const
{
	LAssert(level < NumMipMaps(), "level out of NumMipMaps range");
	return std::max<uint16>(1U,mWidth>>level);
}

leo::uint16 leo::D3D11Texture2D::Height(uint8 level) const
{
	LAssert(level < NumMipMaps(), "level out of NumMipMaps range");
	return std::max<uint16>(1U, mHeight >> level);
}

void leo::D3D11Texture2D::Map2D(uint8 array_index, uint8 level, MapAccess tma, uint16 x_offset, uint16 y_offset,
	uint16 width, uint16 height, void *& data, uint32_t & row_pitch)
{
	D3D11_MAPPED_SUBRESOURCE MapSubRes;
	dxcall(context()->Map(mTex.get(), D3D11CalcSubresource(level, array_index, NumMipMaps()), D3D11Mapping::Mapping(tma, Type(), Access(), NumMipMaps()), 0, &MapSubRes));
	uint8_t* p = static_cast<uint8_t*>(MapSubRes.pData);
	data = p + y_offset * MapSubRes.RowPitch + x_offset * NumFormatBytes(Format());
	row_pitch = MapSubRes.RowPitch;
}

void leo::D3D11Texture2D::Unmap2D(uint8 array_index, uint8 level)
{
	context()->Unmap(mTex.get(), D3D11CalcSubresource(level, array_index, NumMipMaps()));
}

ID3D11Resource * leo::D3D11Texture2D::Resource() const
{
	return mTex.get();
}

ID3D11ShaderResourceView * leo::D3D11Texture2D::ResouceView()
{
	LAssert(Access() & EA_G_R,"ShaderResourceView means GPU_Read");
	//LAssert(first_array_index < ArraySize());
	//LAssert(first_array_index + array_size <= ArraySize());

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	switch (Format())
	{
	case EF_D16:
		desc.Format = DXGI_FORMAT_R16_UNORM;
		break;

	case EF_D24S8:
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;

	case EF_D32F:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;

	default:
		desc.Format = mDesc.Format;
		break;
	}

	if (ArraySize() > 1)
	{
		if (mSampleInfo.Count > 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		}
		desc.Texture2DArray.MostDetailedMip = 0;
		desc.Texture2DArray.MipLevels = NumMipMaps();
		desc.Texture2DArray.ArraySize = ArraySize();
		desc.Texture2DArray.FirstArraySlice = 0;
	}
	else
	{
		if (mSampleInfo.Count > 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		}
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.MipLevels = NumMipMaps();
	}

	return CreateD3DSRV(desc);
}

ID3D11UnorderedAccessView * leo::D3D11Texture2D::AccessView()
{
	LAssert(Access() & EA_G_U, "UnorderedAccessView means GPU_Unordered");
	//LAssert(first_array_index < ArraySize());
	//LAssert(first_array_index + array_size <= ArraySize());

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = mDesc.Format;
	if (ArraySize() > 1)
	{
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = 0;
		desc.Texture2DArray.ArraySize = ArraySize();
		desc.Texture2DArray.FirstArraySlice = 0;
	}
	else
	{
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
	}

	return CreateD3DUAV(desc);
}

ID3D11RenderTargetView * leo::D3D11Texture2D::TargetView()
{
	LAssert(Access() & EA_G_W, "UnorderedAccessView means GPU_Write");
	//LAssert(first_array_index < ArraySize());
	//LAssert(first_array_index + array_size <= ArraySize());

	D3D11_RENDER_TARGET_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = D3D11Mapping::MappingFormat(Format());
	if (mSampleInfo.Count > 1)
	{
		if (ArraySize() > 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		}
	}
	else
	{
		if (ArraySize() > 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		}
	}
	if (ArraySize() > 1)
	{
		desc.Texture2DArray.MipSlice = 0;
		desc.Texture2DArray.ArraySize = ArraySize();
		desc.Texture2DArray.FirstArraySlice = 0;
	}
	else
	{
		desc.Texture2D.MipSlice = 0;
	}

	return CreateD3DRTV(desc);
}

ID3D11DepthStencilView * leo::D3D11Texture2D::DepthStencilView()
{
	LAssert(Access() & EA_G_W,"DepthStencilView means GPU_Write");
	//LAssert(first_array_index < ArraySize());
	//LAssert(first_array_index + array_size <= ArraySize());

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = D3D11Mapping::MappingFormat(Format());
	desc.Flags = 0;
	if (mSampleInfo.Count > 1)
	{
		if (ArraySize() > 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}
	}
	else
	{
		if (ArraySize() > 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}
	}
	if (ArraySize() > 1)
	{
		desc.Texture2DArray.MipSlice = 0;
		desc.Texture2DArray.ArraySize = ArraySize();
		desc.Texture2DArray.FirstArraySlice = 0;
	}
	else
	{
		desc.Texture2D.MipSlice = 0;
	}

	return CreateD3DDSV(desc);
}

ID3D11Texture2D * leo::D3D11Texture2D::D3DTexture() const
{
	return mTex.get();
}

void leo::D3D11Texture2D::ReclaimHWResource(ElementInitData const * init_data)
{
	std::vector<D3D11_SUBRESOURCE_DATA> subres_data(ArraySize() * NumMipMaps());
	if (init_data != nullptr)
	{
		for (uint32_t j = 0; j < ArraySize(); ++j)
		{
			for (uint32_t i = 0; i < NumMipMaps(); ++i)
			{
				subres_data[j * NumMipMaps() + i].pSysMem = init_data[j * NumMipMaps() + i].data;
				subres_data[j * NumMipMaps() + i].SysMemPitch = init_data[j * NumMipMaps() + i].row_pitch;
				subres_data[j * NumMipMaps() + i].SysMemSlicePitch = init_data[j * NumMipMaps() + i].slice_pitch;
			}
		}
	}

	auto d3d_tex = win::make_scope_com<ID3D11Texture2D>();
	dxcall(device()->CreateTexture2D(&mDesc, (init_data != nullptr) ? &subres_data[0] : nullptr, &d3d_tex));
	mTex.swap(d3d_tex);

	if ((Access() & (EA_G_R | EA_M)) && (NumMipMaps() > 1))
	{
		ResourceView();
	}
}
