#include "ITexture.hpp"

using namespace platform::Render::IFormat;
using namespace leo::inttype;

platform::Render::Texture::Texture(Type type_, uint8 numMipMaps, uint8 array_size_,
	EFormat format_, uint32 access, SampleDesc sample_info_)
	:type(type_), access_mode(access), sample_info(sample_info_),
	mipmap_size(numMipMaps),array_size(array_size_),format(format_)
{
}

platform::Render::Texture::~Texture()
{
}

uint8 platform::Render::Texture::GetNumMipMaps() const
{
	return mipmap_size;
}

platform::Render::EFormat platform::Render::Texture::GetFormat() const
{
	return format;
}

uint8 platform::Render::Texture::GetArraySize() const
{
	return array_size;
}

platform::Render::TextureType platform::Render::Texture::GetType() const
{
	return type;
}

platform::Render::SampleDesc platform::Render::Texture::GetSampleInfo() const
{
	return sample_info;
}

uint32 platform::Render::Texture::GetSampleCount() const
{
	return sample_info.Count;
}

uint32 platform::Render::Texture::GetSampleQuality() const
{
	return sample_info.Quality;
}

uint32 platform::Render::Texture::GetAccessMode() const
{
	return access_mode;
}

platform::Render::Texture1D::Texture1D(uint8 numMipMaps, uint8 array_size_,
	EFormat format_, uint32 access, SampleDesc sample_info)
	:Texture(TextureType::T_1D,numMipMaps,array_size_,format_, access, sample_info)
{
}

platform::Render::Texture1D::~Texture1D()
{
}



void platform::Render::Texture1D::Resize(uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_width, uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_width, bool linear)
{
	Resize(*this, dst_array_index, dst_level, dst_x_offset, dst_width, src_array_index, src_level, src_x_offset, src_width, linear);
}

platform::Render::Texture2D::Texture2D(uint8 numMipMaps, uint8 array_size_,
	EFormat format_, uint32 access, SampleDesc sample_info)
	:Texture(TextureType::T_2D,numMipMaps,array_size_,format_, access, sample_info)
{
}

platform::Render::Texture2D::~Texture2D()
{
}



void platform::Render::Texture2D::Resize(uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_width, uint16 dst_height, 
	uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_width, uint16 src_height, bool linear)
{
	Resize(*this, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
		src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, linear);
}


platform::Render::Texture3D::Texture3D(uint8 numMipMaps, uint8 array_size_,
	EFormat format_, uint32 access, SampleDesc sample_info)
	:Texture(TextureType::T_3D,numMipMaps,array_size_,format_,access,sample_info)
{
}

platform::Render::Texture3D::~Texture3D()
{
}


void platform::Render::Texture3D::Resize(uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_z_offset, uint16 dst_width, uint16 dst_height, uint16 dst_depth, 
	uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_z_offset, uint16 src_width, uint16 src_height, uint16 src_depth, bool linear)
{
	Resize(*this, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_z_offset, dst_width, dst_height, dst_depth,
		src_array_index, src_level, src_x_offset, src_y_offset, src_z_offset, src_width, src_height, src_depth,
		linear);
}

platform::Render::TextureCube::TextureCube(uint8 numMipMaps, uint8 array_size_,
	EFormat format_, uint32 access, SampleDesc sample_info)
	:Texture(TextureType::T_Cube,numMipMaps,array_size_,format_,access,sample_info)
{
}

platform::Render::TextureCube::~TextureCube()
{
}

void platform::Render::TextureCube::Resize(uint8 dst_array_index, CubeFaces dst_face, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_width, uint16 dst_height, 
	uint8 src_array_index, CubeFaces src_face, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_width, uint16 src_height, bool linear)
{
	Resize(*this, dst_array_index, dst_face, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
		src_array_index, src_face, src_level, src_x_offset, src_y_offset, src_width, src_height,
		linear);
}
