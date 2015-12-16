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

			uint16 Width(uint8 /*level*/) const override
			{
				return 0;
			}
			uint16 Height(uint8 /*level*/) const override
			{
				return 0;
			}
			uint16 Depth(uint8 /*level*/) const override
			{
				return 0;
			}

			void ReclaimHWResource(ElementInitData const *) override{}
		};
	}

	TexturePtr Texture::NullTexture = std::make_shared<details::NullTexture>(Texture::DT_2D, 0,SampleDesc());

	
	Texture::Texture(Dis_Type type, uint32 access, SampleDesc sample_info)
	{
	}
	ImplDeDtor(Texture)

	uint8 Texture::NumMipMaps() const
	{
		return mNumMipMaps;
	}

	uint8 Texture::ArraySize() const
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