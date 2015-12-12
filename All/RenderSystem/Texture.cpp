#include "Texture.hpp"

namespace leo{

	namespace details {
		class NullTexture :public Texture
		{
		public:
			NullTexture(Dis_Type type, uint32_t access, SampleDesc sample_info)
				: Texture(type, access, sample_info)
			{
			}

			std::string const & Name() const override
			{
				static std::string const name("Null Texture");
				return name;
			}

			uint32_t Width(uint32_t /*level*/) const override
			{
				return 0;
			}
			uint32_t Height(uint32_t /*level*/) const override
			{
				return 0;
			}
			uint32_t Depth(uint32_t /*level*/) const override
			{
				return 0;
			}
		};
	}

	TexturePtr Texture::NullTexture = std::make_shared<details::NullTexture>(Texture::DT_2D, 0,SampleDesc());

	

	Texture::Texture(Dis_Type type, uint32 access, SampleDesc sample_info)
	{
	}
	ImplDeDtor(Texture)

	uint32_t Texture::NumMipMaps() const
	{
		return mNumMipMaps;
	}

	uint32_t Texture::ArraySize() const
	{
		return mArraySize;
	}

	EFormat Texture::Format() const
	{
		return mFormat;
	}

	Texture::Dis_Type Texture::Type() const
	{
		return mDimension;
	}

	SampleDesc Texture::SampleInfo() const
	{
		return mSampleInfo;
	}


	uint32_t Texture::Access() const
	{
		return mAccess;
	}
}