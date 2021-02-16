/*!	\file cassert.cpp
\ingroup LBase
\brief ISO C ¶ÏÑÔ/µ÷ÊÔ¸ú×ÙÀ©Õ¹¡£
*/

#include "LBase/cassert.h"
#include <exception> // for std::terminate;
#include <cstdarg>

//todo add platform.h
#if _WIN32
#include <Windows.h>
#endif

#undef ldebug

namespace platform {
	void native_debug(const char* msg) {
#if _WIN32
		OutputDebugStringA(msg);
#else
		std::fprintf(stderr, msg);
#endif
	}

	void ldebug(const char* msg, va_list args) {
		const size_t count = 4096 / sizeof(char);
		char strBuffer[count] = { 0 };
		vsnprintf(strBuffer, count - 1, msg, args);
		native_debug(strBuffer);
	}

	void ldebug(const char* msg, ...) lnothrow {
		const size_t count = 4096 / sizeof(char);
		char strBuffer[count] = { 0 };
		char *p = strBuffer;
		va_list vlArgs;
		va_start(vlArgs, msg);
		vsnprintf(strBuffer, count - 1, msg, vlArgs);
		va_end(vlArgs);
		native_debug(strBuffer);
	}
}

namespace leo
{
	void
		lassert(const char* expr_str, const char* file, int line, const char* msg)
		lnothrow
	{
		const auto chk_null([](const char* s) lnothrow{
			return s && *s != char() ? s : "<unknown>";
		});

		platform::ldebug("Assertion failed @ \"%s\":%i:\n"
			" %s .\nMessage: \n%s\n", chk_null(file), line, chk_null(expr_str),
			chk_null(msg));
		std::terminate();
	}

#if LB_Use_LTrace
	void
		ltrace(std::FILE* stream, std::uint8_t lv, std::uint8_t t, const char* file,
			int line, const char* msg, ...) lnothrow
	{
		if (lv < t)
		{
#if _WIN32
			if LB_LIKELY(stream == stderr) {
				platform::ldebug("Trace[%#X] @ \"%s\":%i:\n", unsigned(lv), file,
					line);

				std::va_list args;
				va_start(args, msg);
				platform::ldebug(msg, args);
				va_end(args);
				return;
			}
#endif
			std::fprintf(stream, "Trace[%#X] @ \"%s\":%i:\n", unsigned(lv), file,
				line);

			std::va_list args;
			va_start(args, msg);
			std::vfprintf(stream, msg, args);
			va_end(args);
		}
	}
#endif

} // namespace leo;

