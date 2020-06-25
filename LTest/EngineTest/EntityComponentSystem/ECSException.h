#ifndef FrameWork_ECS_Exception_h
#define FrameWork_ECS_Exception_h 1

#include <stdexcept>

namespace ecs {
	class ECSException : public std::runtime_error {
	public:
		using base = std::runtime_error;
		using base::base;
	};
}

#endif