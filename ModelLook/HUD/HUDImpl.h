#pragma once

#include "Widget.h"
#include "HUDGraphics.h"

namespace leo {
	namespace HUD {
		namespace details {
			class hud_tex_wrapper : implements IImage {
			public:
				hud_tex_wrapper(const Size& = {}) {}

				~hud_tex_wrapper() {}

				Graphics GetContext() const lnothrow override {
					return context;
				}
				void SetSize(const Size& s) override {

				}

				hud_tex_wrapper* clone() const override{
					return new hud_tex_wrapper();
				}
			private:
				Graphics context;
			};
		}
	}


	namespace HUD {
		namespace details {
			void DrawText(const std::string& Text, const PaintContext& pc, const Point& offset);
		}
	}
}
