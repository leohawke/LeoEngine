////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   LBase/ldef.hpp
//  Version:     v1.03
//  Created:     02/06/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 
// -------------------------------------------------------------------------
//  History:
//		2014-9-27 11:02: 强制要求编译器支持alignas以及noexcept(以VS14 CTP1支持特性为主)
//		2015-3-9  11:22  新增要求编译器支持noexcept(以VS14 CTP5支持特性为主),并格式化注释
//		2015-8-8  10:48  新增两个宏LEO_BEGIN和LEO_END
//
////////////////////////////////////////////////////////////////////////////
#ifndef LBase_ldef_h
#define LBase_ldef_h 1

/*!	\defgroup lang_impl_versions Language Implementation Versions
\brief 语言实现的版本。
\since build 1.01
*/
//@{

/*!
\def LB_IMPL_CPP
\brief C++实现支持版本
\since build 1.02

定义为 __cplusplus
*/
#ifdef __cplusplus
#define LB_IMPL_CPP __cplusplus
#else
# error "This header is only for C++."
#endif



/*!
\def LB_IMPL_MSCPP
\brief Microsof C++ 实现支持版本
\since build 1.02

定义为 _MSC_VER 描述的版本号
*/

/*!
\def LB_IMPL_GNUCPP
\brief GNU C++ 实现支持版本
\since build 1.02

定义为 100进制的三重版本编号和
*/

/*!
\def LB_IMPL_CLANGCPP
\brief LLVM/Clang++ C++ 实现支持版本
\since build 1.02

定义为 100进制的三重版本编号和
*/
#ifdef _MSC_VER
#	undef LB_IMPL_MSCPP
#	define LB_IMPL_MSCPP _MSC_VER
#define VC_STL 1
#elif __clang__
#	undef LB_IMPL_CLANGPP
#	define LB_IMPL_CLANGPP (__clang_major__ * 10000 + __clang_minor__ * 100 \
			+ __clang_patchlevel__)
#	elif defined(__GNUC__)
#		undef LB_IMPL_GNUCPP
#		define LB_IMPL_GNUCPP (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 \
			+ __GNUC_PATCHLEVEL__)
#endif

#ifdef __clang__
#ifdef LB_IMPL_MSCPP
#undef LB_IMPL_MSCPP
#define VC_STL 1
#endif
#	undef LB_IMPL_CLANGPP
#	define LB_IMPL_CLANGPP (__clang_major__ * 10000 + __clang_minor__ * 100 \
			+ __clang_patchlevel__)
#endif

//禁止CL编译器的安全警告
#if VC_STL
//将指针用作迭代器引发的error C4996
//See doucumentation on how to use Visual C++ 'Checked Iterators'
#undef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS 1

//使用不安全的缓存区函数引发的error C4996
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

//TODO 更改相关代码
#undef _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING 1

#undef _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING 1

#undef _ENABLE_EXTENDED_ALIGNED_STORAGE
#define _ENABLE_EXTENDED_ALIGNED_STORAGE 1
//@}

/*!
\brief 特性检测宏补充定义：若不可用则替换为预处理记号 0 。
\since build 1.02
*/
//@{
//! \since build 1.4
#ifndef __has_attribute
#	define __has_attribute(...) 0
#endif

//! \since build 1.3
#ifndef __has_builtin
#	define __has_builtin(...) 0
#endif


#ifndef __has_cpp_attribute
#	define __has_cpp_attribute(...) 0
#endif

#ifndef __has_feature
#	define __has_feature(...) 0
#endif

#ifndef __has_extension
#	define __has_extension(...) 0
#endif
//@}


#include <cstddef> //std::nullptr_t,std::size_t,std::ptrdiff_t,offsetof;
#include <climits> //CHAR_BIT;
#include <cassert> //assert;
#include <cstdint>
#include <cwchar>  //std::wint_t;
#include <utility> //std::foward;
#include <type_traits> //std:is_class,std::is_standard_layout;
#include <functional> //std::function


/*!	\defgroup preprocessor_helpers Perprocessor Helpers
\brief 预处理器通用助手宏。
\since build 1.02
*/
//@{

//\brief 替换列表。
//\note 通过括号保护，可用于传递带逗号的参数。
#define LPP_Args(...) __VA_ARGS__

//! \brief 替换为空的预处理记号。
#define LPP_Empty

/*!
\brief 替换为逗号的预处理记号。
\note 可用于代替宏的实际参数中出现的逗号。
*/
#define LPP_Comma ,

/*
\brief 记号连接。
\sa LPP_Join
*/
#define LPP_Concat(x,y) x ## y

/*
\brief 带宏替换的记号连接。
\see ISO WG21/N4140 16.3.3[cpp.concat]/3 。
\see http://gcc.gnu.org/onlinedocs/cpp/Concatenation.html 。
\see https://www.securecoding.cert.org/confluence/display/cplusplus/PRE05-CPP.+Understand+macro+replacement+when+concatenating+tokens+or+performing+stringification 。

注意 ISO C++ 未确定宏定义内 # 和 ## 操作符求值顺序。
注意宏定义内 ## 操作符修饰的形式参数为宏时，此宏不会被展开。
*/
#define LPP_Join(x,y) LPP_Concat(x,y)
//@}


/*!
\brief 实现标签。
\since build 1.02
\todo 检查语言实现的必要支持：可变参数宏。
*/
#define limpl(...) __VA_ARGS__


/*!	\defgroup lang_impl_features Language Implementation Features
\brief 语言实现的特性。
\since build 1.02
*/
//@{

/*!
\def LB_HAS_ALIGNAS
\brief 内建 alignas 支持。
\since build 1.02
*/
#undef  LB_HAS_ALIGNAS
#define LB_HAS_ALIGNAS \
	(__has_feature(cxx_alignas) || __has_extension(cxx_alignas) || \
		LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1900)

/*!
\def LB_HAS_ALIGNOF
\brief 内建 alignof 支持。
\since build 1.02
*/
#undef LB_HAS_ALIGNOF
#define LB_HAS_ALIGNOF (LB_IMPL_CPP >= 201103L || LB_IMPL_GNUCPP >= 40500 || LB_IMPL_MSCPP >= 1900)

/*!
\def LB_HAS_BUILTIN_NULLPTR
\brief 内建 nullptr 支持。
\since build 1.02
*/
#undef LB_HAS_BUILTIN_NULLPTR
#define LB_HAS_BUILTIN_NULLPTR \
	(__has_feature(cxx_nullptr) || __has_extension(cxx_nullptr) || \
	 LB_IMPL_CPP >= 201103L ||  LB_IMPL_GNUCPP >= 40600 || \
	 LB_IMPL_MSCPP >= 1600)

/*!
\def LB_HAS_CONSTEXPR
\brief constexpr 支持。
\since build 1.02
*/
#undef LB_HAS_CONSTEXPR
#define LB_HAS_CONSTEXPR \
	(__has_feature(cxx_constexpr) || __has_extension(cxx_constexpr) || \
	LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1900)

/*!
\def LB_HAS_NOEXCPT
\brief noexcept 支持。
\since build 1.02
*/
#undef LB_HAS_NOEXCEPT
#define LB_HAS_NOEXCEPT \
	(__has_feature(cxx_noexcept) || __has_extension(cxx_noexcept) || \
		LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1900)

/*!
\def LB_HAS_THREAD_LOCAL
\brief thread_local 支持。
\see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=1773 。
\since build 425
*/
#undef LB_HAS_THREAD_LOCAL
#define LB_HAS_THREAD_LOCAL \
	(__has_feature(cxx_thread_local) || (LB_IMPL_CPP >= 201103L \
		&& !LB_IMPL_GNUCPP) || (LB_IMPL_GNUCPP >= 40800 && _GLIBCXX_HAVE_TLS) ||\
		LB_IMPL_MSCPP >= 1900)
//@}

/*!
\def LB_ABORT
\brief 非正常终止程序。
\note 可能使用体系结构相关实现或标准库 std::abort 函数等。
\since build 1.02
*/
#if __has_builtin(__builtin_trap) || LB_IMPL_GNUCPP >= 40203
#	define LB_ABORT __builtin_trap()
#else
#	define LB_ABORT std::abort()
#endif

/*!
\def LB_ASSUME(expr)
\brief 假定表达式总是成立。
\note 若假定成立有利于优化。
\warning 若假定不成立则行为未定义。
\since build 1.02
*/
#if LB_IMPL_MSCPP >= 1200
#	define LB_ASSUME(_expr) __assume(_expr)
#elif __has_builtin(__builtin_unreachable) || LB_IMPL_GNUCPP >= 40500
#	define LB_ASSUME(_expr) (_expr) ? void(0) : __builtin_unreachable()
#else
#	define LB_ASSUME(_expr) (_expr) ? void(0) : LB_ABORT
#endif


//附加提示
//保证忽略时不导致运行时语义差异的提示,主要用于便于实现可能的优化

//属性
#if LB_IMPL_GNUCPP >= 20500
#define LB_ATTR(...) __attribute__((__VA_ARGS__))
#else
#define LB_ATTR(...)
#endif

/*!
\def LB_ATTR_STD
\brief C++ 标准属性。
\note 注意和 GNU 风格不同，在使用时受限，如不能修饰 lambda 表达式非类型的声明。
\warning 不检查指令。用户应验证可能使用的指令中的标识符在宏替换后能保持正确。
*/
#if __cpp_attributes >= 200809 || __has_feature(cxx_attributes)
#	define LB_ATTR_STD(...) [[__VA_ARGS__]]
#else
#	define LB_ATTR_STD(...)
#endif

#if __has_cpp_attribute(fallthrough)
#	define LB_ATTR_fallthrough LB_ATTR_STD(fallthrough)
#elif __has_cpp_attribute(clang::fallthrough)
#	define LB_ATTR_fallthrough LB_ATTR_STD(clang::fallthrough)
#elif __has_cpp_attribute(gnu::fallthrough)
#	define LB_ATTR_fallthrough LB_ATTR_STD(gnu::fallthrough)
#elif __has_attribute(fallthrough)
#	define LB_ATTR_fallthrough LB_ATTR(fallthrough)
#else
#	define LB_ATTR_fallthrough
#endif


/*!
\def LB_ATTR_returns_nonnull
\brief 指示返回非空属性。
\since build 1.4
\see http://reviews.llvm.org/rL199626 。
\see http://reviews.llvm.org/rL199790 。
\todo 确认 Clang++ 最低可用的版本。
*/
#if __has_attribute(returns_nonnull) || LB_IMPL_GNUC >= 40900 \
	|| LB_IMPL_CLANGPP >= 30500
#	define LB_ATTR_returns_nonnull LB_ATTR(returns_nonnull)
#else
#	define LB_ATTR_returns_nonnull
#endif

//修饰的是分配器,或返回分配器调用的函数或函数模板
//行为类似 std::malloc 或 std::calloc
//函数若返回非空指针,返回的指针不是其他有效指针的别人,且指针指向内容不由其他存储决定
#if LB_IMPL_GNUCPP >= 20296
#	define LB_ALLOCATOR LB_ATTR(__malloc__)
#else
#	define LB_ALLOCATOR
#endif

//分支预测提示
#if LB_IMPL_GNUCPP >= 29600
#	define LB_EXPECT(expr, constant) (__builtin_expect(expr, constant))
#	define LB_LIKELY(expr) (__builtin_expect(bool(expr), 1))
#	define LB_UNLIKELY(expr) (__builtin_expect(bool(expr), 0))
#else
#	define LB_EXPECT(expr, constant) (expr)
#	define LB_LIKELY(expr) (expr)
#	define LB_UNLIKELY(expr) (expr)
#endif

/*!
\def YB_NONNULL
\brief 指定假定保证不为空指针的函数参数。
\warning 当指定的函数实际为空时行为未定义。
*/
#if LB_IMPL_GNUCPP >= 30300
#	define LB_NONNULL(...) __attribute__ ((__nonnull__ (__VA_ARGS__)))
#else
#	define LB_NONNULL(...)
#endif

//指定无返回值函数
#if LB_IMPL_GNUCPP >= 40800
#	define LB_NORETURN [[noreturn]]
#elif LB_IMPL_GNUCPP >= 20296
#	define LB_NORETURN LB_ATTR(__noreturn__)
#else
#	define LB_NORETURN [[noreturn]]
#endif

//指定函数或函数模板实例为纯函数
//假定函数可返回,假定函数无外部可见的副作用
#if LB_IMPL_GNUCPP >= 20296
#	define LB_PURE LB_ATTR(__pure__)
#else
#	define LB_PURE
#endif

//指定函数为无状态函数
#if LB_IMPL_GNUCPP >= 20500
#	define LB_STATELESS LB_ATTR(__const__)
#else
#	define LB_STATELESS
#endif

/*!	\defgroup lib_options Library Options
\brief 库选项。
\since build 1.02
*/
//@{

/*!
\def LB_DLL
\brief 使用 LBase 动态链接库。
\since build 1.02
*/
/*!
\def LB_BUILD_DLL
\brief 构建 LBase 动态链接库。
\since build 1.02
*/
/*!
\def LB_API
\brief LBase 应用程序编程接口：用于向库文件约定链接。
\since build 1.02
\todo 判断语言特性支持。
*/
#if defined(LB_DLL) && defined(LB_BUILD_DLL)
#	error "DLL could not be built and used at the same time."
#endif

#if LB_IMPL_MSCPP \
	|| (LB_IMPL_GNUCPP && (defined(__MINGW32__) || defined(__CYGWIN__)))
#	ifdef LB_DLL
#		define LB_API __declspec(dllimport)
#	elif defined(LB_BUILD_DLL)
#		define LB_API __declspec(dllexport)
#	else
#		define LB_API
#	endif
#elif defined(LB_BUILD_DLL) && (LB_IMPL_GNUCPP >= 40000 || LB_IMPL_CLANGPP)
#	define LB_API LB_ATTR(__visibility__("default"))
#else
#	define LB_API
#endif

/*!
\def LB_Use_LAssert
\brief 使用断言。
\since build 1.02
*/
/*!
\def LB_Use_LTrace
\brief 使用调试跟踪记录。
\since build 1.02
*/
#ifndef NDEBUG
#	ifndef LB_Use_LAssert
#		define LB_Use_LAssert 1
#	endif
#endif
#define LB_Use_LTrace 1


/*!
\def LB_USE_EXCEPTION_SPECIFICATION
\brief 使用 LBase 动态异常规范。
\since build 1.02
*/
#if 0 && !defined(NDEBUG)
#	define LB_USE_EXCEPTION_SPECIFICATION 1
#endif

/*!
\def LEO_BEGIN
\brief 定义为 namespace leo {
\since build 1.03
*/
#ifndef LEO_BEGIN
#define LEO_BEGIN namespace leo{
#endif

/*!
\def LEO_END
\brief 定义为 }
\since build 1.03
*/
#ifndef LEO_END
#define LEO_END	}
#endif

//@}

/*!	\defgrouppseudo_keyword Specified Pseudo-Keywords
\brief LBase 指定的替代关键字。
\since build 1.02
*/
//@{

/*!
\ingroup pseudo_keyword
\def lalignof
\brief 查询特定类型的对齐大小。
\note 同 C++11 alignof 作用于类型时的语义。
\since build 1.02
*/
#if LB_HAS_ALIGNOF
#	define lalignof alignof
#else
#	define lalignof(_type) std::alignment_of<_type>::value
#endif

/*!
\ingroup pseudo_keyword
\def lalignas
\brief 指定特定类型的对齐大小。
\note 同 C++11 alignas 作用于类型时的语义。
\since build 1.02
*/
#if LB_HAS_ALIGNAS
#	define lalignas(_n) alignas(_n)
#else
#	define lalignas(_n) _declspec(align(_n))
#endif

/*!
\ingroup pseudo_keyword
\def lconstexpr
\brief 指定编译时常量表达式。
\note 同 C++11 constepxr 作用于编译时常量的语义。
\since build 1.02
*/
/*!
\ingroup pseudo_keyword
\def lconstfn
\brief 指定编译时常量函数。
\note 同 C++11 constepxr 作用于编译时常量函数的语义。
\since build 1.02
*/
#if LB_HAS_CONSTEXPR
#	define lconstexpr constexpr
#	define lconstfn constexpr
#else
#	define lconstexpr const
#	define lconstfn inline
#endif

/*!
\def lconstfn_relaxed
\brief 指定编译时没有 C++11 限制和隐式成员 const 的常量函数。
\note 同 C++14 constepxr 作用于编译时常量函数的语义。
\since build 1.4
*/
#if __cpp_constexpr >= 201304
#	define lconstfn_relaxed constexpr
#else
#	define lconstfn_relaxed inline
#endif


/*!
\def lfname
\brief 展开为表示函数名的预定义变量的宏。
\todo 判断语言实现版本。
*/
#if LB_IMPL_MSCPP || __INTEL_COMPILER >= 600 || __IBMCPP__ >= 500
#	define lfname __FUNCTION__
#elif __BORLANDC__ >= 0x550
#	define lfname __FUNC__
#elif __cplusplus >= 201103 || __STDC_VERSION__ >= 199901
#	define lfname __func__
#else
#	define lfname "<unknown-fn>"
#endif

/*!
\def lfsig
\brief 展开为表示函数签名的预定义变量的宏。
\todo 判断语言实现版本。
*/
#if LB_IMPL_GNUCPP || LB_IMPL_CLANGPP
#	define lfsig __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#	define lfsig __FUNCSIG__
#elif __INTEL_COMPILER >= 600 || __IBMCPP__ >= 500
#	define lfsig __FUNCTION__
#elif __BORLANDC__ >= 0x550
#	define lfsig __FUNC__
#elif LB_IMPL_MSCPP || __cplusplus >= 201103 || __STDC_VERSION__ >= 199901
#	define lfsig __func__
#else
#	define lfsig "<unknown-fsig>"
#endif

/*!
\ingroup pseudo_keyword
\def lthrow
\brief LBase 动态异常规范：根据是否使用异常规范宏指定或忽略动态异常规范。
\note lthrow = "yielded throwing" 。
*/
#if LB_USE_EXCEPTION_SPECIFICATION
#	define lthrow throw
#else
#	define lthrow(...)
#endif

#ifdef LB_USE_EXCEPTION_VALIDATION
#	define lnothrowv lnothrow
#else
#	define lnothrowv
#endif

/*!
\ingroup pseudo_keyword
\def lnoexcept
\brief 无异常抛出保证：指定特定的异常规范。
\since build 1.02
*/
#if LB_HAS_NOEXCEPT
#	define lnoexcept noexcept
#	define lnoexcept_assert(_msg, ...) \
	static_assert(noexcept(__VA_ARGS__), _msg)
#else
#	define lnoexcept(...)
#	define lnoexcept_assert(_msg, ...)
#endif

#if LB_HAS_NOEXCEPT
#	define lnothrow lnoexcept
#elif LB_IMPL_GNUCPP >= 30300
#	define lnothrow __attribute__ ((nothrow))
#else
#	define lnothrow lthrow()
#endif

#define lnoexcept_spec(...) lnoexcept(noexcept(__VA_ARGS__))

/*!
\ingroup pseudo_keyword
\def lthread
\brief 线程局部存储：若实现支持，指定为 \c thread_local 。
\since build 1.02
*/
#if LB_HAS_THREAD_LOCAL && defined(_MT)
#	define lthread thread_local
#else
#ifdef LB_IMPL_MSCPP
#	define lthread __declspec(thread)
#else
#	define lthread static
#endif
#endif
//@}

#ifdef LB_IMPL_MSCPP
#define lselectany __declspec(selectany)
#else
#define lselectany __attribute__((weak))
#endif

namespace stdex
{

	//char无unsigned和signed指定
	using byte = unsigned char;
#if  CHAR_BIT == 8
	//一字节并不一定等于8位!
	using octet = byte;
#else
	using octet = void;
#endif
	using errno_t = int;
	using std::ptrdiff_t;
	using std::size_t;
	using std::wint_t;


#if LB_HAS_BUILTIN_NULLPTR
	using std::nullptr_t;
#else
	const class nullptr_t
	{
	public:
		template<typename _type>
		inline
			operator _type*() const
		{
			return 0;
		}

		template<class _tClass, typename _type>
		inline
			operator _type _tClass::*() const
		{
			return 0;
		}
		template<typename _type>
		bool
			equals(const _type& rhs) const
		{
			return rhs == 0;
		}

		void operator&() const = delete;
	} nullptr = {};

	template<typename _type>
	inline bool
		operator==(nullptr_t lhs, const _type& rhs)
	{
		return lhs.equals(rhs);
	}
	template<typename _type>
	inline bool
		operator==(const _type& lhs, nullptr_t rhs)
	{
		return rhs.equals(lhs);
	}

	template<typename _type>
	inline bool
		operator!=(nullptr_t lhs, const _type& rhs)
	{
		return !lhs.equals(rhs);
	}
	template<typename _type>
	inline bool
		operator!=(const _type& lhs, nullptr_t rhs)
	{
		return !rhs.equals(lhs);
	}
#endif

	template<typename...>
	struct empty_base
	{};

	//tuple,pair所需的构造重载
	using raw_tag = empty_base<>;

	//成员计算静态类型检查. 
	template<bool bMemObjPtr, bool bNoExcept, typename T>
	class offsetof_check
	{
		static_assert(std::is_class<T>::value, "Non class type found.");
		static_assert(std::is_standard_layout<T>::value,
			"Non standard layout type found.");
		static_assert(bMemObjPtr, "Non-static member object violation found.");
		static_assert(bNoExcept, "Exception guarantee violation found.");
	};

#define lunused(...) static_cast<void>(__VA_ARGS__)

#define loffsetof(type,member) \
	(decltype(sizeof(stdex::offsetof_check<std::is_member_object_pointer< \
	decltype(&type::member)>::value,lnoexcept(offsetof(type,member)), \
	type>))(offsetof(type,member)))

	/*!
	\ingroup pseudo_keyword
	\brief 根据参数类型使用 std::forward 传递对应参数。
	\since build 1.02

	传递参数：按类型保持值类别(value catory) 和 const 修饰符。
	当表达式类型为函数或函数引用类型时，结果为左值(lvalue) ，否则：
	当且仅当左值引用类型时结果为左值（此时类型不变）；
	否则结果为对应的右值引用类型的消亡值(xvalue) 。
	*/
#define lforward(expr) std::forward<decltype(expr)>(expr)

	template<typename type, typename ...tParams>
	lconstfn auto
		unsequenced(type && arg, tParams&&...)->decltype(lforward(arg))
	{
		return lforward(arg);
	}

	//无序求值
#define lunseq stdex::unsequenced

}

#endif