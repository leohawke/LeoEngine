#include "ICommandList.h"

#include "IContext.h"
#include <utility>
#include <mutex>

using namespace platform::Render;

CommandList& platform::Render::GetCommandList()
{
	static std::once_flag flag;
	static CommandList Instance;
	std::call_once(flag, [&]()
		{
		}
	);

	return Instance;
}

platform::Render::CommandListBase::CommandListBase()
	:Context(nullptr)
{
	Reset();
}

void platform::Render::CommandListBase::Reset()
{
	Context = platform::Render::Context::Instance().GetDefaultCommandContext();
}

void platform::Render::CommandList::BeginFrame()
{
	GetContext().BeginFrame();
}

void platform::Render::CommandList::EndFrame()
{
	GetContext().EndFrame();
	platform::Render::Context::Instance().AdvanceFrameFence();
}
