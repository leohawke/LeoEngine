#ifndef FrameWork_ECS_Component_h
#define FrameWork_ECS_Component_h 1

#include "ECSCommon.h"
#include <LBase/typeindex.h>

namespace ecs {
	
	struct Component {
		leo::type_index type_index;

		template<typename... _tParams>
		Component(const leo::type_info& type_info, _tParams&&...)
			:type_index(type_info) {
		}

		virtual ~Component();
	};

}


#endif
