#include <string>
#include <assert.h>
#include "IndePlatform\platform.h"
#include "exception.hpp"

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
			try
			{
				return s + ": " + formatmessage(ec);
			}
			catch (...)
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
			try
			{
				LPVOID msgbuff;
				::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					nullptr,
					ec,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPSTR)&msgbuff,
					0,
					nullptr);
				return std::string((char*)msgbuff);
			}
			catch (...)
			{
			}
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
}