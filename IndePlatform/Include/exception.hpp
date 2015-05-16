#ifndef IndePlatform_exception_hpp
#define IndePlatform_exception_hpp

#include <exception>
#include <stdexcept>
#include <cstdint>
#include <assert.h>

#include "BaseMacro.h"
#include "DebugOutput.hpp" // record_level
namespace leo
{
	

	//using exception = std::exception;
	using general_event = std::runtime_error;

	class logged_event : public general_event
	{
	public:
		using level_type = record_level;
	private:
		level_type level;
	public:
		logged_event(const std::string&, level_type);
		logged_event(const general_event&, level_type);

		DefGetter(const lnothrow, level_type, Level, level);

		friend std::string format_logged_event(const logged_event& e);
	};
	
	std::string format_logged_event(const logged_event& e);
	namespace win
	{
		class host_exception : public logged_event
		{
		public:
			host_exception(const std::string& = "unknown host exception", level_type = {}) lnothrow;
		};

		class win32_exception : public host_exception
		{
		public:
			using error_code_type = DWORD;
		private:
			error_code_type errcode;
		public:
			win32_exception(error_code_type, const std::string& = "win32 exception", level_type = {}) lnothrow;

			DefGetter(const lnothrow, error_code_type, Error_Code, errcode);
			DefGetter(const lnothrow, std::string, Message, formatmessage(errcode));

			explicit DefCvt(const lnothrow, error_code_type, errcode);

			static std::string formatmessage(error_code_type) lnothrow;
		};

		class dx_exception : public win32_exception
		{
		public:
			dx_exception(error_code_type ec, const std::string& s = "COM(directx) exception", level_type l = {}) lnothrow
				: win32_exception(ec, s, l)
			{}
		};

#define Raise_Error_Exception(err,...)\
		{\
		throw leo::win::win32_exception(err,__VA_ARGS__);\
		}
#define Raise_Win32_Exception(...) \
		{\
		const auto err(::GetLastError()); \
		\
		throw leo::win::win32_exception(err,__VA_ARGS__);\
		}
#define Raise_DX_Exception(expr,...)\
		{\
		const auto err = expr;\
		\
		throw leo::win::dx_exception(err,__VA_ARGS__);\
		}
#define Catch_Win32_Exception \
		catch(leo::win::win32_exception & e) \
				{\
			RecordPrintf(e.GetLevel(),e.what());\
				}
#define Catch_DX_Exception \
		catch(leo::win::dx_exception & e) \
		{\
			RecordPrintf(e.GetLevel(),e.what());\
		}
	}


	void dxcall(win::HRESULT hr);
	void win32call(win::BOOL retval);
}

#endif