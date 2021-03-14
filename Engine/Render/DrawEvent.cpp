#include "DrawEvent.h"
#include <cstdarg>
#include <LFramework/Core/LString.h>

using namespace platform::Render;

platform::Render::DrawEventName::DrawEventName(const char* EventFormat, ...)
{
	va_list valist;
	va_start(valist, EventFormat);
	FormatedEventName = leo::Text::String(leo::vsfmt(EventFormat, valist));
}

void DrawEvent::Start(CommandList& InCmdList, FColor Color, const char16_t* Name)
{
	CmdList = &InCmdList;
	CmdList->PushEvent(Name, Color);
}

void DrawEvent::Stop()
{
	CmdList->PopEvent();
}