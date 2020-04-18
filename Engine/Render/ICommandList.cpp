#include "ICommandList.h"
#include <utility>
#include <mutex>

using namespace platform::Render;

CommandList& platform::Render::GetCommandList()
{
	static std::once_flag flag;
	static CommandList Instance;
	std::call_once(flag, [&]()
		{
			Instance.SetContext(nullptr);
		}
	);

	return Instance;
}
