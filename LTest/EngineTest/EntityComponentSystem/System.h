#ifndef FrameWork_ECS_SYSTEM_h
#define FrameWork_ECS_SYSTEM_h 1

#include <LBase/linttype.hpp>
#include "ECSCommon.h"

namespace ecs {
	struct UpdateParams {
		double timeStep;
	};

	class System {
	public:
		virtual ~System();

		virtual leo::uint32 Update(const UpdateParams&) = 0;
	};
}


#endif
