#include <string>
#include <assert.h>
#include "platform.h"
#include "exception.hpp"

#ifdef PLATFORM_WIN32
static std::string formatmessage(const char* pre,DWORD errcode)
{
	LPVOID msgbuff;
	::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr,
		errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&msgbuff,
		0,
		nullptr);
	std::string rvalue(pre);
	rvalue += (LPSTR)(msgbuff);
	LocalFree(msgbuff);
	return rvalue;
}
#endif

namespace leo
{
	logged_event::logged_event(const std::string& s, level_type l)
		:general_event(s), level(l)
	{}

	logged_event::logged_event(const general_event& e, level_type l)
		: general_event(e), level(l)
	{}
	
	namespace win
	{
		host_exception::host_exception(const std::string& s, level_type l)
			lnothrow :logged_event(s,l)
		{}

		win32_exception::win32_exception(error_code_type ec, const std::string& s, level_type l)
			lnothrow
			: host_exception([&]{
#ifndef LB_IMPL_CLANGPP
			try
#endif
			{
				return s + ": " + formatmessage(ec);
			}
#ifndef LB_IMPL_CLANGPP
			catch (...)
#endif
			{
			}
			return std::string(s);
		}(), l),
			errcode(ec)
		{
			assert(ec != 0);
		}

		std::string
			win32_exception::formatmessage(error_code_type ec) lnothrow
		{
#ifdef PLATFORM_WIN32
#ifndef LB_IMPL_CLANGPP
			try
#endif
			{

				LPVOID msgbuff;
				if (!::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					nullptr,
					ec,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPSTR)&msgbuff,
					0,
					nullptr))
					return std::to_string(GetLastError());
				return std::string((char*)msgbuff);
			}
#ifndef LB_IMPL_CLANGPP
			catch (...)
#endif
			{
			}
#endif
			return {};
		}
	}

	void dxcall(win::HRESULT hr)
	{
		if (FAILED(hr))
		{
			Raise_DX_Exception(hr);
		}
	}
	void win32call(win::BOOL retval)
	{
		if (!retval)
		{
			Raise_Win32_Exception();
		}
	}

	/*
	Emergent = 0x00,
	Alert = 0x20,
	Critical = 0x40,
	Err = 0x60,
	Warning = 0x80,
	Notice = 0xA0,
	Informative = 0xC0,
	Debug = 0xE0
	*/

	std::string format_logged_event(const logged_event& e) {
		std::string str;
		
		str.reserve(24+strlen(e.what()));
		switch (e.level) {
		case record_level::Emergent:
			str.append("Emergent !!! ");
			break;
		case record_level::Alert:
			str.append("Alert @@@ ");
			break;
		case record_level::Critical:
			str.append("Critical ### ");
			break;
		case record_level::Err:
			str.append("Err $$$ ");
			break;
		case record_level::Warning:
			str.append("Warning %%% ");
			break;
		case record_level::Notice:
			str.append("Notice ^^^ ");
			break;
		case record_level::Informative:
			str.append("Informative &&& ");
			break;
		case record_level::Debug:
			str.append("Debug *** ");
			break;
		}
		str += e.what();
		switch (e.level) {
		case record_level::Emergent:
			str.append(" !!!");
			break;
		case record_level::Alert:
			str.append(" @@@");
			break;
		case record_level::Critical:
			str.append(" ###");
			break;
		case record_level::Err:
			str.append(" $$$");
			break;
		case record_level::Warning:
			str.append(" %%%");
			break;
		case record_level::Notice:
			str.append(" ^^^");
			break;
		case record_level::Informative:
			str.append(" &&&");
			break;
		case record_level::Debug:
			str.append(" ***");
			break;
		}
		
		return str;
	}
}