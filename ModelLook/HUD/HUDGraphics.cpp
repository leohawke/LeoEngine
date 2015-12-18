#include "RenderSystem/TextureX.hpp"
#include "HUDImpl.h"

namespace leo {
	namespace HUD {
		ImplDeDtor(IImage)
	}

	namespace X {
		std::shared_ptr<HUD::IImage>  MakeIImage(HUD::Size s) {
			return std::make_shared<HUD::details::hud_tex_wrapper>(s);
		}
	}
}