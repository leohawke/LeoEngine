/*!	\file MinGW32.h
\ingroup Framework
\ingroup Win32
\brief Framework MinGW32 平台公共扩展。
*/

#ifndef FrameWork_Win32_Mingw32_h
#define FrameWork_Win32_Mingw32_h 1

#include <LBase/Host.h>
#include <LBase/NativeAPI.h>
#if !LFL_Win32
#	error "This file is only for Win32."
#endif
#include <LBase/Debug.h>
#include <LBase/enum.hpp>
#include <LBase/FReference.h> //for unique_ptr todo FileIO.h
#include <chrono>

namespace platform_ex {

	namespace Windows {
		using ErrorCode = unsigned long;

		/*!
		\ingroup exceptions
		\brief Win32 错误引起的宿主异常。
		*/
		class LB_API Win32Exception : public Exception
		{
		public:
			/*!
			\pre 错误码不等于 0 。
			\warning 初始化参数时可能会改变 ::GetLastError() 的结果。
			*/
			//@{
			Win32Exception(ErrorCode, string_view = "Win32 exception",
				leo::RecordLevel = leo::Emergent);
			/*!
			\pre 第三参数非空。
			\note 第三参数表示函数名，可以使用 \c __func__ 。
			*/
			LB_NONNULL(4)
				Win32Exception(ErrorCode, string_view, const char*,
					leo::RecordLevel = leo::Emergent);
			//@}
			DefDeCopyCtor(Win32Exception)
				/*!
				\brief 虚析构：类定义外默认实现。
				*/
				~Win32Exception() override;

			DefGetter(const lnothrow, ErrorCode, ErrorCode, ErrorCode(code().value()))
				DefGetter(const lnothrow, std::string, Message,
					FormatMessage(GetErrorCode()))

				explicit DefCvt(const lnothrow, ErrorCode, GetErrorCode())

				/*!
				\brief 取错误类别。
				\return std::error_category 派生类的 const 引用。
				*/
				static const std::error_category&
				GetErrorCategory();

			/*!
			\brief 格式化错误消息字符串。
			\return 若发生异常则结果为空，否则为区域固定为 en-US 的系统消息字符串。
			*/
			static std::string
				FormatMessage(ErrorCode) lnothrow;
			//@}
		};


		//! \since build 592
		//@{
		//! \brief 按 ::GetLastError 的结果和指定参数抛出 Windows::Win32Exception 对象。
#	define LFL_Raise_Win32Exception(...) \
	{ \
		const auto err(::GetLastError()); \
	\
		throw platform_ex::Windows::Win32Exception(err, __VA_ARGS__); \
	}

		//! \brief 按表达式求值和指定参数抛出 Windows::Win32Exception 对象。
#	define LFL_Raise_Win32Exception_On_Failure(_expr, ...) \
	{ \
		const auto err(Windows::ErrorCode(_expr)); \
	\
		if(err != ERROR_SUCCESS) \
			throw platform_ex::Windows::Win32Exception(err, __VA_ARGS__); \
	}
		//@}

		/*!
		\brief 跟踪 ::GetLastError 取得的调用状态结果。
		*/
#	define LFL_Trace_Win32Error(_lv, _fn, _msg) \
	TraceDe(_lv, "Error %lu: failed calling " #_fn " @ %s.", \
		::GetLastError(), _msg)

		/*!
		\brief 调用 Win32 API 或其它可用 ::GetLastError 取得调用状态的例程。
		\note 调用时直接使用实际参数，可指定非标识符的表达式，不保证是全局名称。
		*/
		//@{
		/*!
		\note 若失败抛出 Windows::Win32Exception 对象。
		*/
		//@{
#	define LFL_WrapCallWin32(_fn, ...) \
	[&](const char* msg) LB_NONNULL(1){ \
		const auto res(_fn(__VA_ARGS__)); \
	\
		if(LB_UNLIKELY(!res)) \
			LFL_Raise_Win32Exception(#_fn, msg); \
		return res; \
	}

#	define LFL_CallWin32(_fn, _msg, ...) \
	LFL_WrapCallWin32(_fn, __VA_ARGS__)(_msg)

#	define LFL_CallWin32F(_fn, ...) LFL_CallWin32(_fn, lfsig, __VA_ARGS__)
		//@}

		/*!
		\note 若失败跟踪 ::GetLastError 的结果。
		\note 格式转换说明符置于最前以避免宏参数影响结果。
		\sa LFL_Trace_Win32Error
		*/
		//@{
#	define LFL_WrapCallWin32_Trace(_fn, ...) \
	[&](const char* msg) LB_NONNULL(1){ \
		const auto res(_fn(__VA_ARGS__)); \
	\
		if(LB_UNLIKELY(!res)) \
			LFL_Trace_Win32Error(platform::Descriptions::Warning, _fn, msg); \
		return res; \
	}

#	define LFL_CallWin32_Trace(_fn, _msg, ...) \
	LFL_WrapCallWin32_Trace(_fn, __VA_ARGS__)(_msg)

#	define LFL_CallWin32F_Trace(_fn, ...) \
	LFL_CallWin32_Trace(_fn, lfsig, __VA_ARGS__)
		//@}
		//@}

		//! \since for Load D3D12
		//@{
		//! \brief 加载过程地址得到的过程类型。
		using ModuleProc = std::remove_reference_t<decltype(*::GetProcAddress(::HMODULE(), {})) > ;

		/*!
		\brief 从模块加载指定过程的指针。
		\pre 参数非空。
		*/
		//@{
		LB_API LB_ATTR_returns_nonnull LB_NONNULL(2) ModuleProc*
			LoadProc(::HMODULE, const char*);
		template<typename _func>
		inline LB_NONNULL(2) _func&
			LoadProc(::HMODULE h_module, const char* proc)
		{
			return  *platform::FwdIter(reinterpret_cast<_func*>(LoadProc(h_module, proc)));
			//return platform::Deref(reinterpret_cast<_func*>(LoadProc(h_module, proc))); cl bug
		}
		template<typename _func>
		LB_NONNULL(1, 2) _func&
			LoadProc(const wchar_t* module, const char* proc)
		{
			return LoadProc<_func>(LFL_CallWin32F(GetModuleHandleW, module), proc);
		}

		/*!
		\brief 局部存储删除器。
		*/
		struct LB_API LocalDelete
		{
			using pointer = ::HLOCAL;

			void
				operator()(pointer) const lnothrow;
		};

	}
}

#endif

