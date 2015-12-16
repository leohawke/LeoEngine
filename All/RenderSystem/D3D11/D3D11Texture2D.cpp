#include "D3D11Texture.hpp"
#include "D3D11RenderSystem.hpp"
#include "..\..\DeviceMgr.h"

auto device = [] {return leo::DeviceMgr().GetDevice(); };



namespace {

	template<unsigned N>
	leo::uint16& get(std::pair<leo::uint16, leo::uint16>& pair) {
		return N == 0 ? pair.first : pair.second;
	}

	enum select {
		width = 0,
		height = 1,
	};
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

	mSize.resize(NumMipMaps());
	get<select::width>(mSize[0]) = width;
	get<select::height>(mSize[0]) = height;
	for (uint32_t level = 1; level < NumMipMaps(); ++level)
	{
		get<select::width>(mSize[level]) = std::max<uint32_t>(1U, width >> level);
		get<select::height>(mSize[level]) = std::max<uint32_t>(1U, height >>level);
	}

	mDesc.Width = width;
	mDesc.Height = height;
	mDesc.MipLevels = NumMipMaps();
	mDesc.ArraySize = ArraySize();
	switch (Format())
	{
	case EF_D16:
		mDesc.Format = DXGI_FORMAT_R16_TYPELESS;
		break;

	case EF_D24S8:
		mDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		break;

	case EF_D32F:
		mDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		break;

	default:
		mDesc.Format = D3D11Mapping::MappingFormat(Format());
		break;
	}
	mDesc.SampleDesc.Count = sample_info.Count;
	mDesc.SampleDesc.Quality = sample_info.Quality;

	D3DFlags(mDesc.Usage, mDesc.BindFlags, mDesc.CPUAccessFlags, mDesc.MiscFlags);
	ReclaimHWResource(init_data.data == nullptr? nullptr:&init_data);
}

leo::D3D11Texture2D::~D3D11Texture2D()
{
}

leo::uint16 leo::D3D11Texture2D::Width(uint8) const
{
	return uint16();
}

leo::uint16 leo::D3D11Texture2D::Height(uint8) const
{
	return uint16();
}

ID3D11Resource * leo::D3D11Texture2D::Resource() const
{
	return nullptr;
}

ID3D11ShaderResourceView * leo::D3D11Texture2D::ResouceView()
{
	return nullptr;
}

ID3D11UnorderedAccessView * leo::D3D11Texture2D::AccessView()
{
	return nullptr;
}

ID3D11RenderTargetView * leo::D3D11Texture2D::TargetView()
{
	return nullptr;
}

ID3D11DepthStencilView * leo::D3D11Texture2D::DepthStencilView()
{
	return nullptr;
}

ID3D11Texture2D * leo::D3D11Texture2D::D3DTexture() const
{
	return nullptr;
}

void leo::D3D11Texture2D::ReclaimHWResource(ElementInitData const * init_data)
{
}
