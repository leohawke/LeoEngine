#ifndef IndePlatform_exception_hpp
#define IndePlatform_exception_hpp

//Todo :fix debug printf

#include <exception>
#include <stdexcept>
#include <cstdint>
#include <assert.h>

#include "BaseMacro.h"
#include "leoint.hpp"
namespace leo
{
	enum class record_level : std::uint8_t
	{
		Emergent = 0x00,
		Alert = 0x20,
		Critical = 0x40,
		Err = 0x60,
		Warning = 0x80,
		Notice = 0xA0,
		Informative = 0xC0,
		Debug = 0xE0
	};

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
			OutputDebugStringA(e.what());\
				}
#define Catch_DX_Exception \
		catch(leo::win::dx_exception & e) \
		{\
			OutputDebugStringA(e.what());\
		}
	}


	void dxcall(win::HRESULT hr);
	void win32call(win::BOOL retval);
}

#endif