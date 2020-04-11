#pragma once

#include "Platform.h"
#include "LDescriptions.h"
#include <LBase/cassert.h>

namespace platform_ex {
#if LF_Multithread == 1
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
		SendDebugString(platform::Descriptions::RecordLevel, const char*)
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