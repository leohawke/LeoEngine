#include "DrawEvent.h"

using namespace platform::Render;

void DrawEvent::Start(CommandList& InCmdList, FColor Color, const char16_t* Name)
{
	CmdList = &InCmdList;
	CmdList->PushEvent(Name, Color);
}

void DrawEvent::Stop()
{
	CmdList->PopEvent();
}