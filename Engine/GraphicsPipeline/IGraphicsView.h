#ifndef LE_GRAPHICSVIEW_H
#define LE_GRAPHICSVIEW_H 1

#pragma once

#include <LBase/linttype.hpp>
#include "../System/NinthTimer.h"

namespace platform::GraphicsPipeline {
	inline namespace Interface {
		class IGarphicsView {
			enum ViewType {
				Default_View,
				Shadow_View,
			};

			// View can be in either reading or writing modes.
			//[Multi Thread]
			enum UsageMode {
				Undefined,
				Reading,
				Readed,
				Writing,
				Writed
			};

			const leo::uint16 ViewTypeCount = 2;
		};
	}
}

#endif