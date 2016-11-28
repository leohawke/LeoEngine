#include "LApplication.h"

namespace leo {
	namespace Shells {
		Shell::~Shell() = default;
	}

	Application::Application()
		: Shell()
	{}
	Application::~Application()
	{
	}
}