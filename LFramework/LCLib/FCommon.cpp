#include "Debug.h"
#include "NativeAPI.h"
#include <cstring>
#include <cerrno>
#include <cstdarg>
#if LFL_Win32
#	include "../Win32/LCLib/NLS.h"
#endif

namespace platform
{

	void
		terminate() lnothrow
	{
		std::abort();
	}

	int
		usystem(const char* cmd)
	{
#if LFL_Win32
		return ::_wsystem(platform_ex::Windows::MBCSToWCS(cmd, CP_UTF8).c_str());
#else
		return std::system(cmd);
#endif
	}

	size_t
		FetchLimit(SystemOption opt) lnothrow
	{
		// NOTE: For constant expression value that is always less than max value
		//	of %size_t, there is no check needed.
		// XXX: Values greater than maximum value of %size_t would be truncated.
		const auto conf_conv([](long n) {
			using res_t = leo::common_type_t<long, size_t>;
			return n >= 0 && res_t(n) < res_t(std::numeric_limits<size_t>::max())
				? size_t(n) : size_t(-1);
		});

		// NOTE: For POSIX-defined minimum values, if the macro is undefined,
		//	use the value specified in POSIX.1-2013 instead. See http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/limits.h.html.
		switch (opt)
		{
		case SystemOption::PageSize:
			// NOTE: Value of 0 indicates the target system does not support paging.
			//	Though POSIX requires 1 as minimum acceptable value, it is intended
			//	here. For platforms without an appropriate page size, it should be
			//	specified here explicitly, rather than relying on the system library
			//	with POSIX compliance. For example, RTEMS: https://devel.rtems.org/ticket/1216.
#if LFL_Win32
			::SYSTEM_INFO info;
			::GetSystemInfo(&info);

			return size_t(info.dwPageSize);
#elif defined(_SC_PAGESIZE)
			return conf_conv(::sysconf(_SC_PAGESIZE));
#else
			// XXX: %::getpagesize is not used. Fallback to POSIX allowed minimum
			//	value.
			return 1;
#endif
		case SystemOption::MaxSemaphoreNumber:
#if LFL_Win32
			return size_t(-1);
#elif defined(_SC_SEM_NSEMS_MAX)
			return conf_conv(::sysconf(_SC_SEM_NSEMS_MAX));
#elif defined(_POSIX_SEM_NSEMS_MAX)
			static_assert(size_t(_POSIX_SEM_NSEMS_MAX) == _POSIX_SEM_NSEMS_MAX,
				"Invalid value found.");

			return _POSIX_SEM_NSEMS_MAX;
#else
			return 256;
#endif
		case SystemOption::MaxSemaphoreValue:
#if LFL_Win32
			static_assert(size_t(std::numeric_limits<long>::max())
				== std::numeric_limits<long>::max(), "Invalid value found.");

			return size_t(std::numeric_limits<long>::max());
#elif defined(_SC_SEM_VALUE_MAX)
			return conf_conv(::sysconf(_SC_SEM_VALUE_MAX));
#elif defined(_POSIX_SEM_VALUE_MAX)
			static_assert(size_t(_POSIX_SEM_VALUE_MAX) == _POSIX_SEM_VALUE_MAX,
				"Invalid value found.");

			return _POSIX_SEM_VALUE_MAX;
#else
			return 256;
#endif
		case SystemOption::MaxSymlinkLoop:
#if defined(_SC_SYMLOOP_MAX)
			return conf_conv(::sysconf(_SC_SYMLOOP_MAX));
#elif defined(_POSIX_SYMLOOP_MAX)
			static_assert(size_t(_POSIX_SYMLOOP_MAX) == _POSIX_SYMLOOP_MAX,
				"Invalid value found.");

			return _POSIX_SYMLOOP_MAX;
#else
			// NOTE: Windows has nothing about this concept. Win32 error
			//	%ERROR_CANT_RESOLVE_FILENAME maps to Windows NT error
			//	%STATUS_REPARSE_POINT_NOT_RESOLVED, which seems to be not
			//	necessarily related to a fixed loop limit. To keep POSIX compliant
			//	and better performance of loop detection, minimum allowed value is
			//	used. See also https://cygwin.com/ml/cygwin/2009-03/msg00335.html.
			// TODO: Make it configurable for Windows?
			return 8;
#endif
		default:
			break;
		}
#if !LFL_DS
		TraceDe(Descriptions::Warning,
			"Configuration option '%zu' is not handled.", size_t(opt));
		errno = EINVAL;
#endif
		return size_t(-1);
	}

} // namespace platform;