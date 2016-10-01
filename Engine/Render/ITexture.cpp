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



void platform::Render::Texture1D::Resize(const Box1D& dst, const Box1D& src, bool linear)
{
	Resize(*this, dst, src, linear);
}

platform::Render::Texture2D::Texture2D(uint8 numMipMaps, uint8 array_size_,
	EFormat format_, uint32 access, SampleDesc sample_info)
	:Texture(TextureType::T_2D,numMipMaps,array_size_,format_, access, sample_info)
{
}

platform::Render::Texture2D::~Texture2D()
{
}



void platform::Render::Texture2D::Resize(const Box2D& dst, 
	const Box2D& src, bool linear)
{
	Resize(*this, dst,
		src, linear);
}


platform::Render::Texture3D::Texture3D(uint8 numMipMaps, uint8 array_size_,
	EFormat format_, uint32 access, SampleDesc sample_info)
	:Texture(TextureType::T_3D,numMipMaps,array_size_,format_,access,sample_info)
{
}

platform::Render::Texture3D::~Texture3D()
{
}


void platform::Render::Texture3D::Resize(const Box3D& dst, 
	const Box3D& src,bool linear)
{
	Resize(*this, dst,
		src,
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

void platform::Render::TextureCube::Resize(const BoxCube& dst, 
	const BoxCube& src, bool linear)
{
	Resize(*this, dst,
		src,
		linear);
}
