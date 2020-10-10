#pragma once

#include "ICommandList.h"

#define WANTS_DRAW_EVENTS 1

#if WANTS_DRAW_EVENTS

namespace platform::Render
{
	struct DrawEvent
	{
		CommandList* CmdList;

		DrawEvent()
			:CmdList(nullptr)
		{}

		~DrawEvent()
		{
			if (CmdList)
			{
				Stop();
			}
		}

		void Start(CommandList& CmdList, FColor Color, const char16_t* Name);
		void Stop();
	};
}

#define SCOPED_GPU_EVENT(CmdList,Name) platform::Render::DrawEvent LPP_Concat(Event_##Name,__LINE__);LPP_Concat(Event_##Name,__LINE__).Start(CmdList,platform::FColor(),LPP_Concat(u,#Name));

#else

namespace platform::Render
{
	struct DrawEvent
	{
	};
}

#define SCOPED_GPU_EVENT(...)

#endif