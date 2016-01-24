#include "D3D11Texture.hpp"
#include "D3D11RenderSystem.hpp"
#include "..\..\DeviceMgr.h"
#include <exception.hpp>

namespace {
	auto device = [] {return leo::DeviceMgr().GetDevice(); };
	auto context = [] {return leo::DeviceMgr().GetDeviceContext(); };
}

leo::D3D11Texture1D::D3D11Texture1D(uint16 width, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access, ElementInitData init_data)
	: D3D11Texture(DT_1D, access)
{
	if (0 == numMipMaps)
	{
		numMipMaps = 1;
		uint32_t w = width;
		while (w != 1)
		{
			++numMipMaps;

			w = std::max<uint32_t>(1U, w / 2);
		}
	}
	mNumMipMaps = numMipMaps;

	auto & re = dynamic_cast<leo::D3D11Engine&>(leo::GetRenderEngine());
	if (re.GetCoreFeatureLevel() <= D3D_FEATURE_LEVEL_9_3)
	{
		if (!re.DeviceCaps().full_npot_texture_support
			&& (mNumMipMaps > 1) && ((width & (width - 1)) != 0))
		{
			// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
			mNumMipMaps = 1;
		}
	}

	mArraySize = array_size;
	mFormat = format;
	mDesc.Format = D3D11Mapping::MappingFormat(mFormat);
	mWidth = width;

	mDesc.Width = width;
	mDesc.MipLevels = NumMipMaps();
	mDesc.ArraySize = ArraySize();
	mDesc.Format = D3D11Mapping::MappingFormat(Format());

	D3DFlags(mDesc.Usage, mDesc.BindFlags, mDesc.CPUAccessFlags, mDesc.MiscFlags);
	ReclaimHWResource(init_data.data == nullptr ? nullptr : &init_data);
}

using leo::D3D11Texture1D;
ImplDeDtor(D3D11Texture1D)

leo::uint16 leo::D3D11Texture1D::Width(uint8 level) const
{
	LAssert(level < NumMipMaps(), "level out of NumMipMaps range");
	return std::max<uint16>(1U, mWidth >> level);
}

void leo::D3D11Texture1D::Map1D(uint8 array_index, uint8 level, MapAccess tma,
	uint16 x_offset,
	uint16 width,
	void*& data)
{
	D3D11_MAPPED_SUBRESOURCE MapSubRes;
	dxcall(context()->Map(mTex.get(), D3D11CalcSubresource(level, array_index, NumMipMaps()), D3D11Mapping::Mapping(tma, Type(), Access(), NumMipMaps()), 0, &MapSubRes));
	uint8_t* p = static_cast<uint8_t*>(MapSubRes.pData);
	data = static_cast<uint8_t*>(MapSubRes.pData) + x_offset * NumFormatBytes(mFormat);
}

void leo::D3D11Texture1D::Unmap1D(uint8 array_index, uint8 level)
{
	context()->Unmap(mTex.get(), D3D11CalcSubresource(level, array_index, NumMipMaps()));
}

ID3D11Resource * leo::D3D11Texture1D::Resource() const
{
	return mTex.get();
}

ID3D11ShaderResourceView * leo::D3D11Texture1D::ResouceView()
{
	LAssert(Access() & EA_G_R, "ShaderResourceView means GPU_Read");
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

	if (mArraySize > 1)
	{
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MostDetailedMip = 0;
		desc.Texture1DArray.MipLevels = NumMipMaps();
		desc.Texture1DArray.FirstArraySlice = 0;
		desc.Texture1DArray.ArraySize = ArraySize();
	}
	else
	{
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MostDetailedMip = 0;
		desc.Texture1D.MipLevels = NumMipMaps();
	}

	return CreateD3DSRV(desc);
}

ID3D11UnorderedAccessView * leo::D3D11Texture1D::AccessView()
{
	LAssert(Access() & EA_G_U, "UnorderedAccessView means GPU_Unordered");
	//LAssert(first_array_index < ArraySize());
	//LAssert(first_array_index + array_size <= ArraySize());

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = mDesc.Format;
	if (ArraySize() > 1)
	{
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MipSlice = 0;
		desc.Texture1DArray.FirstArraySlice = 0;
		desc.Texture1DArray.ArraySize = ArraySize();
	}
	else
	{
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
	}

	return CreateD3DUAV(desc);
}


ID3D11Texture1D * leo::D3D11Texture1D::D3DTexture() const
{
	return mTex.get();
}

void leo::D3D11Texture1D::ReclaimHWResource(ElementInitData const * init_data)
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

	auto d3d_tex = win::make_scope_com<ID3D11Texture1D>();
	dxcall(device()->CreateTexture1D(&mDesc, (init_data != nullptr) ? &subres_data[0] : nullptr, &d3d_tex));
	mTex.swap(d3d_tex);

	if ((Access() & (EA_G_R | EA_M)) && (NumMipMaps() > 1))
	{
		ResourceView();
	}
}