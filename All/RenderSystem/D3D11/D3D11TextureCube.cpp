#include "D3D11Texture.hpp"
#include "D3D11RenderSystem.hpp"
#include "..\..\DeviceMgr.h"
#include <exception.hpp>

namespace {
	auto device = [] {return leo::DeviceMgr().GetDevice(); };
	auto context = [] {return leo::DeviceMgr().GetDeviceContext(); };
}

leo::D3D11TextureCube::D3D11TextureCube(uint16 size, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access, ElementInitData const* init_data)
	: D3D11Texture(DT_1D, access)
{
	if (0 == numMipMaps)
	{
		numMipMaps = 1;
		uint32_t w = size;
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
			&& (mNumMipMaps > 1) && ((size & (size - 1)) != 0))
		{
			// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
			mNumMipMaps = 1;
		}

		if ((mNumMipMaps > 1) && IsCompressedFormat(format))
		{
			// height or width is not a multiply of 4 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
			uint32_t clamped_num_mip_maps;
			for (clamped_num_mip_maps = 0; clamped_num_mip_maps < mNumMipMaps; ++clamped_num_mip_maps)
			{
				uint32_t s = std::max<uint32_t>(1U, size >> clamped_num_mip_maps);
				if ((s & 0x3) != 0)
				{
					break;
				}
			}
			mNumMipMaps = clamped_num_mip_maps;
		}
	}

	mArraySize = array_size;
	mFormat = format;
	mWidth = size;

	mDesc.Width = size;
	mDesc.Height = size;
	mDesc.MipLevels = NumMipMaps();
	mDesc.ArraySize = ArraySize()*6;
	mDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

	mDesc.Format = D3D11Mapping::MappingFormat(Format());
	mDesc.SampleDesc.Count = 1;
	mDesc.SampleDesc.Quality = 0;

	D3DFlags(mDesc.Usage, mDesc.BindFlags, mDesc.CPUAccessFlags, mDesc.MiscFlags);
	ReclaimHWResource(init_data);
}

using leo::D3D11TextureCube;
ImplDeDtor(D3D11TextureCube)

leo::uint16 leo::D3D11TextureCube::Width(uint8 level) const
{
	LAssert(level < NumMipMaps(), "level out of NumMipMaps range");
	return std::max<uint16>(1U, mWidth >> level);
}

leo::uint16 leo::D3D11TextureCube::Height(uint8 level) const
{
	LAssert(level < NumMipMaps(), "level out of NumMipMaps range");
	return std::max<uint16>(1U, mWidth >> level);
}

void leo::D3D11TextureCube::MapCube(uint8 array_index, CubeFaces face, uint8 level,MapAccess tma,
	uint16 x_offset, uint16 y_offset, uint16 /*width*/, uint16 /*height*/,
	void*& data, uint32_t& row_pitch)
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	dxcall(context()->Map(mTex.get(), D3D11CalcSubresource(level, array_index * 6 + face - CF_P_X, NumMipMaps()),
		D3D11Mapping::Mapping(tma, Type(), Access(), NumMipMaps()), 0, &mapped));
	uint8_t* p = static_cast<uint8_t*>(mapped.pData);
	data = p + y_offset * mapped.RowPitch + x_offset * NumFormatBytes(Format());
	row_pitch = mapped.RowPitch;
}

void leo::D3D11TextureCube::UnmapCube(uint8 array_index, CubeFaces face, uint8 level)
{
	context()->Unmap(mTex.get(), D3D11CalcSubresource(level, array_index * 6 + face - CF_P_X, NumMipMaps()));
}

ID3D11Resource * leo::D3D11TextureCube::Resource() const
{
	return mTex.get();
}

ID3D11ShaderResourceView * leo::D3D11TextureCube::ResouceView()
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

	desc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURECUBE;
	desc.TextureCube.MostDetailedMip = 0;
	desc.TextureCube.MipLevels = NumMipMaps();

	return CreateD3DSRV(desc);
}

ID3D11UnorderedAccessView * leo::D3D11TextureCube::AccessView()
{
	LAssert(Access() & EA_G_U, "UnorderedAccessView means GPU_Unordered");
	//LAssert(first_array_index < ArraySize());
	//LAssert(first_array_index + array_size <= ArraySize());

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = mDesc.Format;

	desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.MipSlice = 0;
	desc.Texture2DArray.ArraySize = ArraySize()*6;
	desc.Texture2DArray.FirstArraySlice = 0;

	return CreateD3DUAV(desc);
}

ID3D11RenderTargetView * leo::D3D11TextureCube::TargetView()
{
	LAssert(Access() & EA_G_W, "UnorderedAccessView means GPU_Write");
	LAssert(mSampleInfo.Count == 1, "D3D11TextureCube means Count == 1");
	//LAssert(first_array_index < ArraySize());
	//LAssert(first_array_index + array_size <= ArraySize());

	D3D11_RENDER_TARGET_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = D3D11Mapping::MappingFormat(Format());
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.MipSlice = 0;
	desc.Texture2DArray.FirstArraySlice = 0;
	desc.Texture2DArray.ArraySize = ArraySize() * 6;

	return CreateD3DRTV(desc);
}

ID3D11DepthStencilView * leo::D3D11TextureCube::DepthStencilView()
{
	LAssert(Access() & EA_G_W, "DepthStencilView means GPU_Write");
	LAssert(mSampleInfo.Count == 1, "D3D11TextureCube means Count == 1");
	//LAssert(first_array_index < ArraySize());
	//LAssert(first_array_index + array_size <= ArraySize());
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = D3D11Mapping::MappingFormat(Format());
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.MipSlice = 0;
	desc.Texture2DArray.FirstArraySlice = 0;
	desc.Texture2DArray.ArraySize = ArraySize() * 6;

	return CreateD3DDSV(desc);
}


void leo::D3D11TextureCube::ReclaimHWResource(ElementInitData const * init_data)
{
	std::vector<D3D11_SUBRESOURCE_DATA> subres_data(6 * NumMipMaps());
	if (init_data != nullptr)
	{
		for (int face = 0; face < 6; ++face)
		{
			for (uint32_t i = 0; i < NumMipMaps(); ++i)
			{
				subres_data[face * NumMipMaps() + i].pSysMem = init_data[face * NumMipMaps() + i].data;
				subres_data[face * NumMipMaps() + i].SysMemPitch = init_data[face * NumMipMaps() + i].row_pitch;
				subres_data[face * NumMipMaps() + i].SysMemSlicePitch = init_data[face * NumMipMaps() + i].slice_pitch;
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