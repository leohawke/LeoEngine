/*!	\file Debug.h
\ingroup Framework
\brief 调试设施。
*/

#ifndef FrameWork_Debug_h
#define FrameWork_Debug_h 1

#include <LBase/FCommon.h>
#include <LBase/FContainer.h>
#include <LBase/Mutex.h>

\
/*!	\defgroup diagnostic Diagnostic
\brief 诊断设施。
*/

/*!	\defgroup debugging Debugging
\ingroup diagnostic
\brief 调试设施。

仅在宏 NDEBUG 未被定义时起诊断作用的调试接口和实现。
*/

/*!	\defgroup tracing Tracing
\ingroup diagnostic
\brief 跟踪设施。

独立配置的起诊断作用的调试接口和实现。
*/

/*!
\ingroup tracing
\def LFL_Use_TraceSrc
\brief 在跟踪日志中使用跟踪源码位置。
*/
#ifndef NDEBUG
#	ifndef LFL_Use_TraceSrc
#		define LFL_Use_TraceSrc 1
#	endif
#endif

namespace platform
{

	/*!
	\ingroup diagnostic
	*/
	//@{
	//@}

	/*!
	\brief 使用标准输出流打印字符串并刷新。
	\pre 间接断言：参数的数据指针非空。
	\return 打印和刷新是否成功。

	使用标准输出流以平台相关的方式打印字符串，然后刷新流。编码视为 UTF-8 。
	DS 平台：使用 std::puts 。
	Win32 平台：使用控制台接口，失败时使用 std::cout 。
	其它平台：使用 std::cout 。
	*/
	LB_API bool
		Echo(string_view) lnoexcept(false);

	/*!
	\ingroup tracing
	\brief 日志记录器。
	*/
	class LB_API Logger
	{
	public:
		using Level = Descriptions::RecordLevel;
		using Filter = std::function<bool(Level, Logger&)>;
		//! \note 传递的第三参数非空。
		using Sender = std::function<void(Level, Logger&, const char*)>;

#ifdef NDEBUG
		Level FilterLevel = Descriptions::Informative;
#else
		Level FilterLevel = Descriptions::Debug;
#endif

	private:
		//! \invariant <tt>bool(filter)</tt> 。
		Filter filter{ DefaultFilter };
		//! \invariant <tt>bool(Sender)</tt> 。
		Sender sender{ FetchDefaultSender() };
		/*!
		\brief 日志记录锁。

		仅 DoLog 和 DoLogException 在发送日志时使用的锁。
		使用递归锁以允许用户在发送器中间接递归调用 DoLog 和 DoLogException 。
		*/
		Concurrency::recursive_mutex record_mutex;

	public:
		DefGetter(const lnothrow, const Sender&, Sender, sender)

			/*!
			\brief 设置过滤器。
			\note 忽略空过滤器。
			*/
			void
			SetFilter(Filter);
		/*!
		\brief 设置发送器。
		\note 忽略空发送器。
		*/
		void
			SetSender(Sender);

		/*!
		\brief 访问日志记录执行指定操作。
		\note 线程安全：互斥访问。
		*/
		template<typename _func>
		auto
			AccessRecord(_func f) -> decltype(f())
		{
			Concurrency::lock_guard<Concurrency::recursive_mutex> lck(record_mutex);

			return f();
		}

		//! \brief 默认过滤：仅允许等级不大于阈值的日志被记录。
		static bool
			DefaultFilter(Level, Logger&) lnothrow;

		//! \pre 间接断言：第三参数非空。
		//@{
		/*!
		\brief 默认发送器：使用 std::cerr 输出。
		*/
		static LB_NONNULL(3) void
			DefaultSendLog(Level, Logger&, const char*) lnothrowv;

		/*!
		\brief 默认发送器：使用 stderr 输出。
		*/
		static LB_NONNULL(3) void
			DefaultSendLogToFile(Level, Logger&, const char*) lnothrowv;
		//@}

		/*!
		\brief 转发等级和日志至发送器。
		\note 忽略字符串参数对应的空数据指针参数。
		\note 保证串行发送。
		*/
		//@{
		void
			DoLog(Level, const char*);
		PDefH(void, DoLog, Level lv, string_view sv)
			ImplRet(DoLog(lv, sv.data()))
			//@}

	private:
		/*!
		\brief 转发等级和日志至发送器。
		\pre 间接断言：字符串参数对应的数据指针非空。
		*/
		//@{
		LB_NONNULL(1) void
			DoLogRaw(Level, const char*);
		PDefH(void, DoLogRaw, Level lv, string_view sv)
			ImplRet(DoLogRaw(lv, sv.data()))
			//@}

	public:
		/*!
		\brief 转发等级和异常对象至发送器。

		根据异常对象确定日志字符串并发送。若转发时抛出异常，则记录此异常。
		对并发操作保证串行发送，且整个过程持有锁 record_mutex 以保证连续性。
		*/
		void
			DoLogException(Level level, const std::exception&) lnothrow;

		/*!
		\brief 取新建的平台相关的默认发送：按指定的标签取平台相关实现。
		\pre 断言：参数的数据指针非空。
		*/
		static Sender
			FetchDefaultSender(string_view = "Leo");

		template<typename _fCaller, typename... _tParams>
		void
			Log(Level level, _fCaller&& f, _tParams&&... args)
		{
			if (filter(level, *this))
				TryExpr(DoLog(level, lforward(f)(lforward(args)...)))
				CatchExpr(std::exception& e, DoLogException(level, e), throw)
		}

		/*!
		\brief 默认发送器：使用第一参数指定的流输出。
		\pre 间接断言：字符串参数非空。
		*/
		//@{
		static LB_NONNULL(4) void
			SendLog(std::ostream&, Level, Logger&, const char*) lnothrowv;

		//! \pre 断言：流指针非空。
		static LB_NONNULL(1, 4) void
			SendLogToFile(std::FILE*, Level, Logger&, const char*) lnothrowv;
		//@}
	};


	/*!
	\brief 取公共日志记录器。
	*/
	LB_API Logger&
		FetchCommonLogger();


	/*!
	\brief 格式输出日志字符串前追加记录源文件名和行号。
	\pre 间接断言：第三参数非空。
	\note 允许第一参数为空指针，视为未知。
	\note 当失败时调用 leo::trace 记录，但只保留参数中的文件名和行号。
	*/
	LB_API LB_NONNULL(1, 3) string
		LogWithSource(const char*, int, const char*, ...) lnothrow;


	/*!
	\brief 使用公共日志记录器记录日志格式字符串。
	\note 支持格式同 std::fprintf 。
	\note 使用 FetchCommonLogger 保证串行输出。
	*/
#define LFL_Log(_lv, ...) \
	platform::FetchCommonLogger().Log( \
		platform::Descriptions::RecordLevel(_lv), __VA_ARGS__)

	/*!
	\brief 直接调试跟踪。
	\note 不带源代码信息。
	*/
#define LFL_TraceRaw(_lv, ...) \
	LFL_Log(_lv, [&]() -> platform::string { \
		using platform::sfmt; \
	\
		TryRet(sfmt(__VA_ARGS__)) \
		CatchRet(..., {}) \
	})

	/*!
	\def Trace
	\brief 默认调试跟踪。
	\note 无异常抛出。
	*/
#if LFL_Use_TraceSrc
#	define Trace(_lv, ...) \
	LFL_Log(_lv, [&]{ \
		return platform::LogWithSource(__FILE__, __LINE__, __VA_ARGS__); \
	})
#else
#	define Trace(_lv, ...) LFL_TraceRaw(_lv, __VA_ARGS__)
#endif


	/*!
	\def TraceDe
	\brief 默认调试跟踪。
	\note 使用默认的调试跟踪级别。
	\sa LFL_Trace
	*/
#if LB_Use_LTrace
#	define TraceDe(_lv, ...) Trace(_lv, __VA_ARGS__)
#else
#	define TraceDe(...)
#endif

} // namespace platform;

namespace platform_ex {
#if LB_Multithread == 1
	/*!
	\brief 日志断言函数。
	\note 默认断言 leo::lassert 的替代。
	\warning 若忽略且继续，则行为未定义。应只用于调试用途。
	\todo 允许在 Win32 上禁用消息框。

	在 Win32 上首先使用图形界面（消息框）提示断言失败。注意不适用于消息循环内部。
	允许忽略并继续，否则终止程序。当选择中止时候首先发送 \c SIGABRT 信号。
	忽略此过程的所有错误，包括所有被抛出的异常。若捕获异常则继续以下行为。
	锁定公共日志记录器后调用 leo::lassert ，最终调用 std::terminate 终止程序。
	*/
	LB_API void
		LogAssert(const char*, const char*, int, const char*) lnothrow;

#	if LB_Use_LAssert
#		undef LAssert
#		define LAssert(_expr, _msg) \
	((_expr) ? void(0) \
		: platform_ex::LogAssert(#_expr, __FILE__, __LINE__, _msg))
#	endif
#endif

#if LFL_Win32

	/*!
	\brief 发送字符串至调试器。
	\pre 间接断言：字符串参数非空。
	\note 当前直接调用 ::OutputDebugStringA 。
	*/
	LB_API LB_NONNULL(3) void
		SendDebugString(platform::Logger::Level, platform::Logger&, const char*)
		lnothrowv;


	/*!
	\brief 发送字符串至调试器以及使用Win32控制台API进行输出(包含颜色)。
	\pre 间接断言：字符串参数非空。
	\note 当前直接调用 ::OutputDebugStringA ::WriteConsoleA 。
	*/
	LB_API LB_NONNULL(3) void
		SendDebugStringEx(platform::Logger::Level, platform::Logger&, const char* str) 
		lnothrowv;
#endif
}

namespace platform
{
	/*!
	\brief 断言并返回非空参数。
	\pre 断言：参数非空。
	*/
	template<typename _type>
	inline _type&&
		Nonnull(_type&& p) lnothrowv
	{
		LAssertNonnull(p);
		return lforward(p);
	}

	/*!
	\brief 断言并返回可解引用的迭代器。
	\pre 断言：迭代器非确定不可解引用。
	*/
	template<typename _type>
	inline _type&&
		FwdIter(_type&& i) lnothrowv
	{
		using leo::is_undereferenceable;

		LAssert(!is_undereferenceable(i), "Invalid iterator found.");
		return lforward(i);
	}

	/*!
	\brief 断言并解引用非空指针。
	\pre 使用 ADL 指定的 FwdIter 调用表达式的值等价于调用 platform::FwdIter 。
	\pre 间接断言：指针非空。
	*/
	template<typename _type>
	lconstfn auto
		Deref(_type&& p) -> decltype(*p)
	{
		return *FwdIter(lforward(p));
	}

} // namespace platform;


#endif