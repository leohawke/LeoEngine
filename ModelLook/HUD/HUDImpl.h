#pragma once

#include "Widget.h"

#include "..\UI\TextBase.h"
#include "..\UI\Graphics.h"

namespace leo {
	namespace HUD {
		namespace details {

		}
	}


	namespace HUD {
		namespace details {
			void DrawText(const Drawing::PaintContext& pc, const Drawing::TextState& state, const std::string& Text);
		}
	}
}
