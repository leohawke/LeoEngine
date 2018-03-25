#ifndef FrameWork_ECS_CameraComponent_h
#define FrameWork_ECS_CameraComponent_h 1

#include "../EntityComponentSystem/Component.h"
#include "../EntityComponentSystem/Entity.h"
#include <LFramework/Adaptor/LAdaptor.h>

namespace ecs {
	struct CameraComponent final :public Component {
		CameraComponent(const leo::type_info& type_info);
		
		/*!
		\brief 透视投影信息。
		*/
		//@{
		float near, far;
		float fov,aspect;
		//@}
	};
}

#endif