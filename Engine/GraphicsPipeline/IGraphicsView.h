#ifndef LE_GRAPHICSVIEW_H
#define LE_GRAPHICSVIEW_H 1

#pragma once

#include <LBase/linttype.hpp>
#include "../System/NinthTimer.h"
#include "../Core/Camera.h"

namespace LeoEngine::GraphicsPipeline {
	inline namespace Interface {
		class IGarphicsView {
			enum ViewType {
				Default_View,
				Recursive_View,
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

			const leo::uint16 ViewTypeCount = 3;

			virtual void SetFrameID(leo::uint32 frameId) = 0;
			virtual leo::uint32 GetFrameID() const = 0;

			using TimeValue = ::platform::chrono::TimeValue;

			virtual void SetFrameTime(TimeValue time) = 0;
			virtual TimeValue GetFrameTime() const = 0;

			virtual void SetCamera(const engine::Core::Camera& camera) = 0;
			virtual void SetPreviousFrameCamera(const engine::Core::Camera& camera) = 0;

			virtual void SwitchUsageMode(UsageMode mode) = 0;

		};
	}
}

#endif