/*!	\file Platform.h
\ingroup Framework
\brief 平台相关的公共组件无关函数与宏定义集合。
*/

#ifndef FrameWork_Common_h
#define FrameWork_Common_h 1

#include <LFramework/LCLib/Platform.h>
#include <LBase/functional.hpp>
#include <LBase/cassert.h>
#include <LBase/cwctype.h>
#include <LBase/cstring.h>
#include <LBase/lmacro.h>

namespace platform
{

	/*!
	\ingroup Platforms
	*/
	//@{
	using ID = std::uintmax_t;

	template<ID _vN>
	using MetaID = leo::integral_constant<ID, _vN>;

	//! \brief 平台标识的公共标记类型：指定任意平台。
	struct IDTagBase
	{};

#if LB_IMPL_CLANGPP
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

	//! \brief 宿主平台标识的公共标记类型：指定任意宿主平台。
	struct HostedIDTagBase : virtual IDTagBase
	{};

	//! \brief 平台标识的整数标记类型。
	template<ID _vN>
	struct IDTag : virtual IDTagBase, virtual MetaID<_vN>
	{};

	//! \brief 指定特定基类类型的 IDTag 特化。
#define LFL_IDTag_Spec(_n, _base) \
	template<> \
	struct IDTag<LF_Platform_##_n> : virtual _base, \
		virtual MetaID<LF_Platform_##_n> \
	{};

	//! \brief 指定宿主平台的 IDTag 特化。
#define LFL_IDTag_SpecHost(_n) \
	LFL_IDTag_Spec(_n, HostedIDTagBase)

	//! \brief 指定基于特定平台的 IDTag 特化。
#define LFL_IDTag_SpecOnBase(_n, _b) \
	LFL_IDTag_Spec(_n, IDTag<LF_Platform_##_b>)

	LFL_IDTag_SpecHost(Win32)
		LFL_IDTag_SpecOnBase(Win64, Win32)
		LFL_IDTag_SpecOnBase(MinGW32, Win32)
		LFL_IDTag_SpecOnBase(MinGW64, Win64)
		//LFL_IDTag_SpecHost(Linux)
		//LFL_IDTag_SpecOnBase(Linux_x86, Linux)
		//LFL_IDTag_SpecOnBase(Linux_x64, Linux)
		//LFL_IDTag_SpecOnBase(Android, Linux)
		//LFL_IDTag_SpecOnBase(Android_ARM, Android)

		//! \brief 取平台标识的整数标记集合类型。
		template<ID... _vN>
	struct IDTagSet : virtual IDTag<_vN>...
	{};
	//@}

#if LB_IMPL_CLANGPP
#	pragma GCC diagnostic pop
#endif

#define LCL_Tag_constfn inline


	/*!
	\ingroup PlatformEmulation
	\brief 定义用于平台模拟的传递模板。
	*/
#define LCL_DefPlatformFwdTmpl(_n, _fn) \
	DefFwdTmplAuto(_n, _fn(platform::IDTag<LF_Platform>(), lforward(args)...))


	/*!
	\brief 声明检查存在合式调用的辅助宏。
	\sa leo::is_detected
	*/
	//@{
#define LCL_CheckDecl_t(_fn) CheckDecl##_fn##_t
#define LCL_DeclCheck_t(_fn, _call) \
	template<typename... _tParams> \
	using LCL_CheckDecl_t(_fn) \
		= decltype(_call(std::declval<_tParams&&>()...));
	//@}


	/*!
	\brief 异常终止函数。
	*/
	LB_NORETURN LB_API void
		terminate() lnothrow;


	/*!
	\brief 平台描述空间。
	*/
	namespace Descriptions
	{

		/*!
		\brief 记录等级。
		*/
		enum RecordLevel : std::uint8_t
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

	} // namespace Descriptions;


	  /*!
	  \brief 检查默认区域下指定字符是否为可打印字符。
	  \note MSVCRT 的 isprint/iswprint 实现缺陷的变通。
	  \sa https://connect.microsoft.com/VisualStudio/feedback/details/799287/isprint-incorrectly-classifies-t-as-printable-in-c-locale
	  */
	  //@{
	inline PDefH(bool, IsPrint, char c)
		ImplRet(stdex::isprint(c))
		inline PDefH(bool, IsPrint, wchar_t c)
		ImplRet(stdex::iswprint(c))
		template<typename _tChar>
	bool
		IsPrint(_tChar c)
	{
		return platform::IsPrint(wchar_t(c));
	}
	//@}


	//@{
	inline PDefH(leo::uchar_t*, ucast, wchar_t* p) lnothrow
		ImplRet(leo::replace_cast<leo::uchar_t*>(p))
		inline PDefH(const leo::uchar_t*, ucast, const wchar_t* p) lnothrow
		ImplRet(leo::replace_cast<const leo::uchar_t*>(p))
		template<typename _tChar>
	_tChar*
		ucast(_tChar* p) lnothrow
	{
		return p;
	}

	inline PDefH(wchar_t*, wcast, leo::uchar_t* p) lnothrow
		ImplRet(leo::replace_cast<wchar_t*>(p))
		inline PDefH(const wchar_t*, wcast, const leo::uchar_t* p) lnothrow
		ImplRet(leo::replace_cast<const wchar_t*>(p))
		template<typename _tChar>
	_tChar*
		wcast(_tChar* p) lnothrow
	{
		return p;
	}
	//@}


	/*!
	\brief 循环重复操作。
	*/
	template<typename _func, typename _tErrorRef,
		typename _tError = leo::decay_t<_tErrorRef>,
		typename _type = leo::result_of_t<_func&()>>
		_type
		RetryOnError(_func f, _tErrorRef&& err, _tError e = _tError())
	{
		return leo::retry_on_cond([&](_type res) {
			return res < _type() && _tError(err) == e;
		}, f);
	}


	/*!
	\brief 执行 UTF-8 字符串的环境命令。
	\note 使用 std::system 实现；若参数为空则和 std::system 行为一致。
	*/
	LB_API int
		usystem(const char*);

} // namespace platform;

#endif