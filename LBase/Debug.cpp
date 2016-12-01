#include <LBase/Debug.h>
#include <LBase/Host.h>
#if LFL_Win32
#include <csignal>
#include <LBase/NativeAPI.h>
#include <LBase/Win32/Consoles.h>
#include <LBase/Win32/NLS.h>
#endif
#if LB_Multithread == 1
#include <LBase/concurrency.h>
#endif
#include <iostream>
#include <cstdarg>

namespace platform
{
	namespace
	{
		inline const char*
			chk_null(const char* s)
		{
			return s && *s != char() ? s : "<unknown>";
		}

#if LB_Multithread == 1
		std::string
			FetchCurrentThreadID() lnothrow
		{
			TryRet(leo::get_this_thread_id())
				// XXX: Nothing more can be done.
				CatchIgnore(...)
				return{};
		}
#endif

#if LFL_Win32
		LB_NONNULL(3) size_t
			WConsoleOutput(wstring& wstr, unsigned long h, const char* str)
		{
			using namespace platform_ex;

			wstr = platform_ex::Windows::UTF8ToWCS(str) + L'\n';
			return platform_ex::Windows::WConsole(h).WriteString(wstr);
		}
#endif

		using namespace Concurrency;

	} // unnamed namespace;

	bool
		Echo(string_view sv) lnoexcept(false)
	{
#if LFL_Win32
		wstring wstr;
		size_t n(0);

		TryExpr(n = WConsoleOutput(wstr, STD_OUTPUT_HANDLE, sv.data()))
			CatchIgnore(platform_ex::Windows::Win32Exception&)
			if (n < wstr.length())
				std::cout << &wstr[n];
		return bool(std::cout.flush());
#elif LB_Hosted
		return bool(std::cout << platform_ex::EncodeArg(sv) << std::endl);
#else
		return bool(std::cout << sv << std::endl);
#endif
	}

	void
		Logger::SetFilter(Filter f)
	{
		if (f)
			filter = std::move(f);
	}
	void
		Logger::SetSender(Sender s)
	{
		if (s)
			sender = std::move(s);
	}

	bool
		Logger::DefaultFilter(Level lv, Logger& logger) lnothrow
	{
		return lv <= logger.FilterLevel;
	}

	void
		Logger::DefaultSendLog(Level lv, Logger& logger, const char* str) lnothrowv
	{
		SendLog(std::cerr, lv, logger, str);
	}

	void
		Logger::DefaultSendLogToFile(Level lv, Logger& logger, const char* str)
		lnothrowv
	{
		SendLogToFile(stderr, lv, logger, str);
	}

	void
		Logger::DoLog(Level level, const char* str)
	{
		if (str)
		{
			lock_guard<recursive_mutex> lck(record_mutex);

			DoLogRaw(level, str);
		}
	}

	void
		Logger::DoLogRaw(Level level, const char* str)
	{
		sender(level, *this, Nonnull(str));
	}

	void
		Logger::DoLogException(Level lv, const std::exception& e) lnothrow
	{
		const auto do_log_excetpion_raw([this](const char* msg) LB_NONNULL(1)
			lnothrow {
			try
			{
				DoLogRaw(Descriptions::Emergent,
					"Another exception thrown when handling exception.");
				DoLogRaw(Descriptions::Emergent, msg);
			}
			CatchExpr(...,
				leo::ltrace(stderr, Descriptions::Emergent, Descriptions::Notice,
					__FILE__, __LINE__, "Logging error: unhandled exception."))
		});
		const auto& msg(e.what());
		lock_guard<recursive_mutex> lck(record_mutex);

		try
		{
			// XXX: Provide no throw guarantee and put it out of the critical
			//	section?
			// XXX: Log demangled type name.
			DoLogRaw(lv, sfmt("<%s>: %s", typeid(e).name(), msg));
		}
		CatchExpr(std::exception& ex, do_log_excetpion_raw(ex.what()))
			CatchExpr(..., do_log_excetpion_raw({}))
	}

	Logger::Sender
		Logger::FetchDefaultSender(string_view tag)
	{
		LAssertNonnull(tag.data());
#if LFL_Win32
		return [](Level lv, Logger& logger, const char* str) {
			// TODO: Avoid throwing of %WriteString here for better performance?
			// FIXME: Output may be partially updated?
			try
			{
				wstring wstr;

				WConsoleOutput(wstr, STD_ERROR_HANDLE, str);
			}
			CatchExpr(platform_ex::Windows::Win32Exception&, DefaultSendLog(lv, logger, str))
		};
#endif
		return DefaultSendLog;
	}

	void
		Logger::SendLog(std::ostream& os, Level lv, Logger&, const char* str)
		lnothrowv
	{
		try
		{
#if LB_Multithread == 1
			const auto& t_id(FetchCurrentThreadID());

			if (!t_id.empty())
				os << leo::sfmt("[%s:%#X]: %s\n", t_id.c_str(), unsigned(lv),
					Nonnull(str));
			else
#endif
				os << leo::sfmt("[%#X]: %s\n", unsigned(lv), Nonnull(str));
			os.flush();
		}
		CatchIgnore(...)
	}

	void
		Logger::SendLogToFile(std::FILE* stream, Level lv, Logger&, const char* str)
		lnothrowv
	{
		LAssertNonnull(stream);
#if LB_Multithread == 1
		const auto& t_id(FetchCurrentThreadID());

		if (!t_id.empty())
			std::fprintf(stream, "[%s:%#X]: %s\n", t_id.c_str(), unsigned(lv),
				Nonnull(str));
		else
#endif
			std::fprintf(stream, "[%#X]: %s\n", unsigned(lv), Nonnull(str));
		std::fflush(stream);
	}


	Logger&
		FetchCommonLogger()
	{
		static Logger logger;

		return logger;
	}


	string
		LogWithSource(const char* file, int line, const char* fmt, ...) lnothrow
	{
		try
		{
			std::va_list args;

			va_start(args, fmt);

			string str(vsfmt(fmt, args));

			va_end(args);
			return sfmt("\"%s\":%i:\n", chk_null(file), line) + std::move(str);
		}
		CatchExpr(...,
			leo::ltrace(stderr, Descriptions::Emergent, Descriptions::Notice,
				chk_null(file), line, "LogWithSource error: unhandled exception."))
			return{};
	}

} // namespace platform;

using namespace platform;

namespace platform_ex
{
#if LB_Multithread == 1
	void
		LogAssert(const char* expr_str, const char* file, int line,
			const char* msg) lnothrow
	{
//#	if LFL_Android
		//::__android_log_assert(expr_str, "YFramework",
			//"Assertion failed @ \"%s\":%i:\n %s .\nMessage: \n%s\n", file, line,
			//expr_str, msg);
//#	else
#		if LFL_Win32
		try
		{
			char prog[MAX_PATH]{ "<unknown>" };

			::GetModuleFileNameA({}, prog, MAX_PATH);

			const auto& errstr(sfmt("Assertion failed @ program %s: "
				"\"%s\":%i:\n %s .\nMessage: \n%s\n", prog, chk_null(file),
				line, chk_null(expr_str), chk_null(msg)));

			::OutputDebugStringA(errstr.c_str());
			// XXX: Not safe in windows procedure, but not used in YFramework.
			// TODO: Use custom windows creation?
			switch (::MessageBoxA({}, errstr.c_str(),
				"LeoEngine Runtime Assertion", MB_ABORTRETRYIGNORE | MB_ICONHAND
				| MB_SETFOREGROUND | MB_TASKMODAL))
			{
			case IDIGNORE:
				return;
			case IDABORT:
				std::raise(SIGABRT);
			default:
				break;
			}
			std::terminate();
		}
		catch (...)
		{
			LFL_TraceRaw(Descriptions::Emergent, "Unknown exception found.");
			leo::lassert(expr_str, file, line, msg);
		}
#		endif
		TryExpr(FetchCommonLogger().AccessRecord([=] {
			leo::lassert(expr_str, file, line, msg);
		}))
			catch (...)
		{
			std::fprintf(stderr, "Fetch logger failed.");
			std::fflush(stderr);
			leo::lassert(expr_str, file, line, msg);
		}
	}
#endif

#if LFL_Win32
	const char* sfmt(Logger::Level lv) {
		switch (lv) {
		case Logger::Level::Emergent:
			return "Emergent";
		case Logger::Level::Alert:
			return "Alert";
		case Logger::Level::Critical:
			return "Critical";
		case Logger::Level::Err:
			return "Err";
		case Logger::Level::Warning:
			return "Warning";
		case Logger::Level::Notice:
			return "Notice";
		case Logger::Level::Informative:
			return "Informative";
		case Logger::Level::Debug:
			return "Debug";
		}
		return "Unknown";
	}

	void
		SendDebugString(Logger::Level lv, Logger&, const char* str) lnothrowv
	{
		try
		{
			const auto& t_id(FetchCurrentThreadID());

			// TODO: Use %::WaitForDebugEventEx if possible. See https://msdn.microsoft.com/en-us/library/windows/desktop/mt171594(v=vs.85).aspx.
			::OutputDebugStringA((!t_id.empty() ? leo::sfmt("[%s:%s]: %s",
				t_id.c_str(), sfmt(lv), Nonnull(str)) : leo::sfmt(
					"[%s]: %s", sfmt(lv), Nonnull(str))).c_str());
		}
		CatchIgnore(...)
	}

#endif
}
