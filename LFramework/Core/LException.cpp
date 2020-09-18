#include <LBase/ldef.h>
#include "LException.h"
#include <LBase/exception.h>

namespace leo {

	LoggedEvent::LoggedEvent(const char* str, RecordLevel lv)
		: GeneralEvent(Nonnull(str)),
		level(lv)
	{}
	LoggedEvent::LoggedEvent(string_view sv, RecordLevel lv)
		: GeneralEvent(std::string(sv)),
		level(lv)
	{}
	LoggedEvent::LoggedEvent(const GeneralEvent& e, RecordLevel lv)
		: GeneralEvent(e),
		level(lv)
	{}
	ImplDeDtor(LoggedEvent)

		RecordLevel LoggedEvent::GetLevel() const lnothrow
	{
		return level;
	}

		FatalError::FatalError(const char* t, string_view c)
		: GeneralEvent(Nonnull(t)),
		content((Nonnull(c.data()), make_shared<string>(string(c))))
	{}
	ImplDeDtor(FatalError)

		string_view	FatalError::GetContent() const lnothrow
	{
		return Deref(content);
	}

	const char* FatalError::GetTitle() const lnothrow
	{
		return what();
	}


		void
		TraceException(const char* str, RecordLevel lv, size_t level) lnothrow
	{
		TryExpr(
			LF_TraceRaw(lv, "%s%s", std::string(level, ' ').c_str(), Nonnull(str)))
			CatchExpr(..., LF_TraceRaw(Critical, "Failure @ TraceException."))
	}

	void
		TraceExceptionType(const std::exception& e, RecordLevel lv) lnothrow
	{
		LFL_Log(lv, [](const std::exception& e) {return leo::sfmt("Caught std::exception[%s].", typeid(e).name()); },e);
	}

	void
		ExtractAndTrace(const std::exception& e, RecordLevel lv) lnothrow
	{
		TraceExceptionType(e, lv);
		ExtractException(TraceException, e, lv);
	}

	void
		ExtractException(const ExtractedLevelPrinter& print, const std::exception& e,
			RecordLevel lv, size_t level) lnothrow
	{
		TryExpr(print(e.what(), lv, level))
			CatchExpr(..., print("Exception occurred when printing @ ExtractException.",
				Critical, level))
			// FIXME: Following code only tested OK for %YCL_Win32.
			TryExpr(leo::handle_nested(e,
				[&, lv, level](std::exception& ex) lnothrow{
			ExtractException(print, ex, lv, level + 1);
		}))
			CatchExpr(..., print("Unknown nested exception found nested on calling"
				" leo::handle_nested @ ExtractException.", Critical, level))
	}

	bool
		TryExecute(std::function<void()> f, const char* desc, RecordLevel lv,
			ExceptionTracer trace)
	{
		try
		{
			TryExpr(f())
				catch (...)
			{
				if (desc)
					LF_TraceRaw(Notice, "Exception filtered: %s.", desc);
				throw;
			}
			return{};
		}
		CatchExpr(std::exception& e, trace(e, lv))
			return true;
	}
}