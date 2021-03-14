#pragma once

#include "ICommandList.h"
#include <string>

#define WANTS_DRAW_EVENTS 1

#if WANTS_DRAW_EVENTS

namespace platform::Render
{
	struct DrawEventName
	{
		DrawEventName() = default;

		explicit DrawEventName(const char* EventFormat, ...);

		const char16_t* c_str() const
		{
			return FormatedEventName.c_str();
		}
	private:
		std::basic_string<char16_t> FormatedEventName;
	};

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

#define SCOPED_GPU_EVENTF(CmdList, Format, ...) platform::Render::DrawEvent LPP_Concat(Event_##Name,__LINE__);LPP_Concat(Event_##Name,__LINE__).Start(CmdList,platform::FColor(),platform::Render::DrawEventName(Format, ##__VA_ARGS__).c_str());

#else

namespace platform::Render
{
	struct DrawEvent
	{
	};
}

#define SCOPED_GPU_EVENT(...)

#endif