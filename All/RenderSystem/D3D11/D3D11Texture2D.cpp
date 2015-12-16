#include "D3D11Texture.hpp"

leo::D3D11Texture2D::D3D11Texture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access, SampleDesc sample_info, ElementInitData)
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
	num_mip_maps_ = numMipMaps;

	D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	if (re.DeviceFeatureLevel() <= D3D_FEATURE_LEVEL_9_3)
	{
		if (!re.DeviceCaps().full_npot_texture_support
			&& (num_mip_maps_ > 1) && (((width & (width - 1)) != 0) || ((height & (height - 1)) != 0)))
		{
			// height or width is not a power of 2 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
			num_mip_maps_ = 1;
		}

		if ((num_mip_maps_ > 1) && IsCompressedFormat(format))
		{
			// height or width is not a multiply of 4 and multiple mip levels are specified. This is not supported at feature levels below 10.0.
			uint32_t clamped_num_mip_maps;
			for (clamped_num_mip_maps = 0; clamped_num_mip_maps < num_mip_maps_; ++clamped_num_mip_maps)
			{
				uint32_t w = std::max<uint32_t>(1U, width >> clamped_num_mip_maps);
				uint32_t h = std::max<uint32_t>(1U, height >> clamped_num_mip_maps);
				if (((w & 0x3) != 0) || ((h & 0x3) != 0))
				{
					break;
				}
			}
			num_mip_maps_ = clamped_num_mip_maps;
		}
	}

	array_size_ = array_size;
	format_ = format;

	widths_.resize(num_mip_maps_);
	heights_.resize(num_mip_maps_);
	widths_[0] = width;
	heights_[0] = height;
	for (uint32_t level = 1; level < num_mip_maps_; ++level)
	{
		widths_[level] = std::max<uint32_t>(1U, widths_[level - 1] / 2);
		heights_[level] = std::max<uint32_t>(1U, heights_[level - 1] / 2);
	}

	desc_.Width = width;
	desc_.Height = height;
	desc_.MipLevels = num_mip_maps_;
	desc_.ArraySize = array_size_;
	switch (format_)
	{
	case EF_D16:
		desc_.Format = DXGI_FORMAT_R16_TYPELESS;
		break;

	case EF_D24S8:
		desc_.Format = DXGI_FORMAT_R24G8_TYPELESS;
		break;

	case EF_D32F:
		desc_.Format = DXGI_FORMAT_R32_TYPELESS;
		break;

	default:
		desc_.Format = D3D11Mapping::MappingFormat(format_);
		break;
	}
	desc_.SampleDesc.Count = sample_count;
	desc_.SampleDesc.Quality = sample_quality;

	this->GetD3DFlags(desc_.Usage, desc_.BindFlags, desc_.CPUAccessFlags, desc_.MiscFlags);
	this->ReclaimHWResource(init_data);
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
