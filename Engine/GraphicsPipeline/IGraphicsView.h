#ifndef LE_IGRAPHICSVIEW_H
#define LE_IGRAPHICSVIEW_H 1

#pragma once

#include <LBase/linttype.hpp>
#include "../System/NinthTimer.h"
#include "../Core/Camera.h"
#include "../Render/ViewPort.h"
#include "../Render/DataStructures.h"
#include "../Render/ShadingElement.h"
#include "../Render/ShadingObject.h"
#include "../Render/Effect/Effect.hpp"

namespace LeoEngine::GraphicsPipeline {
	inline namespace Interface {
		using ViewPort = LeoEngine::Render::ViewPort;
		using LightIndex = LeoEngine::Render::LightIndex;
		using DirectLight = LeoEngine::Render::DirectLight;
		using Camera = LeoEngine::Core::Camera;
		using TimeValue = ::platform::chrono::TimeValue;

		using ShadingElement = LeoEngine::Render::ShadingElement;
		using ShadingObject = LeoEngine::Render::ShadingObject;

		using EffectItem = LeoEngine::RenderEffect::EffectItem;


		class IGraphicsView {
		public:
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


			virtual void SetFrameTime(TimeValue time) = 0;
			virtual TimeValue GetFrameTime() const = 0;

			virtual void SetCamera(const Camera& camera) = 0;
			virtual void SetPreviousFrameCamera(const Camera& camera) = 0;

			virtual void SwitchUsageMode(UsageMode mode) = 0;

			virtual void SetViewPort(const ViewPort& viewport) = 0;

			virtual const ViewPort& GetViewPort() const = 0;

			virtual LightIndex  AddLight(const  DirectLight& light) = 0;
			virtual DirectLight& GetLight(LightIndex light_id) = 0;

			virtual void AddShadingObject(leo::observer_ptr<ShadingElement> pShadingElement, leo::observer_ptr<ShadingObject> pShadingObject, EffectItem& effect_item) = 0;
		};
	}
}

#endif