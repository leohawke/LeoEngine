#include "RenderSystem/TextureX.hpp"
#include "HUDImpl.h"

namespace leo {
	namespace HUD {
		ImplDeDtor(IImage)
	}

	namespace X {
		std::unique_ptr<HUD::IImage>  MakeIImage(uint16 w, uint16 h) {
			return std::make_unique<HUD::details::hud_tex_wrapper>(HUD::Size(w, h));
		}
	}
}