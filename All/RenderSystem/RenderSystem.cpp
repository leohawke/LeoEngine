#include "RenderSystem.hpp"

namespace leo {

	ImplDeDtor(RenderFactory)
	ImplDeDtor(RenderEngine)

	class NullRenderFactory :public RenderFactory {
	public:
		std::string const & Name() const override
		{
			static std::string const name("Null Render Factory");
			return name;
		}

		TexturePtr MakeTexture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size,
			EFormat format, SampleDesc sample_info, uint32 access, uint8 const * init_data) override {
			return Texture::NullTexture;
		}
	};

	class NullEngine :public RenderEngine {
	public:
		std::string const & Name() const override
		{
			static std::string const name("Null Render Engine");
			return name;
		}

		RenderFactory& GetFactory() override {
			static NullRenderFactory mNullFactory;
			return mNullFactory;
		}
	};

}
