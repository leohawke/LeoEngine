////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/ldef.hpp
//  Version:     v1.01
//  Created:     02/06/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 
// -------------------------------------------------------------------------
//  History:
//		2014-9-27 11:02: 强制要求编译器支持alignas以及noexcept(以VS14 CTP1支持特性为主)
//
////////////////////////////////////////////////////////////////////////////
#ifndef IndePlatform_ldef_h
#define IndePlatform_ldef_h


//def LB_IMPL_CPP
//brief C++实现支持版本
//定义为 __cplusplus
#ifdef __cplusplus
#define LB_IMPL_CPP __cplusplus
#else
# error "This header is only for C++."
#endif

//def LB_IMPL_MSCPP
//brief Microsof C++ 实现支持版本
//定义为 _MSC_VER 描述的版本号
#ifdef _MSC_VER
#	undef LB_IMPL_MSCPP
#	define LB_IMPL_MSCPP _MSC_VER
#elif __clang__
#	undef LB_IMPL_CLANGPP
#	define LB_IMPL_CLANGPP (__clang__ * 10000 + __clang_minor__ * 100 \
			+ __clang_patchlevel__)
#	elif defined(__GNUC__)
#		undef LB_IMPL_GNUCPP
#		define LB_IMPL_GNUCPP (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 \
			+ __GNUC_PATCHLEVEL__)
#endif

//禁止CL编译器的安全警告
#if LB_IMPL_MSCPP >= 1400
//将指针用作迭代器引发的error C4996
//See doucumentation on how to use Visual C++ 'Checked Iterators'
#undef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

//使用不安全的缓存区函数引发的error C4996
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstddef> //std::nullptr_t,std::size_t,std::ptrdiff_t,offsetof;
#include <climits> //CHAR_BIT;
#include <cassert> //assert;
#include <cstdint>
#include <cwchar>  //std::wint_t;
#include <utility> //std::foward;
#include <type_traits> //std:is_class,std::is_standard_layout;

#ifndef __has_feature
#define __has_feature(...) 0
#endif

#ifndef __has_extension
#define __has_extension(...) 0
#endif

//预处理修正助手宏
#define LPP_Empty
#define LPP_Comma ,
//标准未定义##求值顺序,GCC中,##参数为宏不会被展开,故多一层包装
#define LPP_Join(x,y) LPP_Concat(x,y) 
#define LPP_Concat(x,y) x ## y

//语言实现的必要支持:可变参数宏.
#define limpl(...) __VA_ARGS__

//语言特性检测

//LB_HAS_ALIGNAS
//内建 alignas 支持
#undef  LB_HAS_ALIGNAS
#define LB_HAS_ALIGNAS \
	(__has_feature(cxx_alignas) || __has_extension(cxx_alignas) || \
		LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1900)
#if !LB_HAS_ALIGNAS && LB_IMPL_MSCPP != 1800
#error "compiler must support alignas"
#endif


#undef LB_HAS_ALIGNOF
#define LB_HAS_ALIGNOF (LB_IMPL_CPP >= 201103L)


#undef LB_HAS_BUILTIN_NULLPTR
#define LB_HAS_BUILTIN_NULLPTR \
	(__has_feature(cxx_nullptr) || __has_extension(cxx_nullptr) || \
	 LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1600)
#if !LB_HAS_BUILTIN_NULLPTR
#error "compiler must support builtin_nullptr"
#endif
#undef LB_HAS_CONSTEXPR
#define LB_HAS_CONSTEXPR \
	(__has_feature(cxx_constexpr) || __has_extension(cxx_constexpr) || \
	LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1900)
#if !LB_HAS_CONSTEXPR && LB_IMPL_MSCPP != 1800
#error "compiler must support constexpr"
#endif
#undef LB_HAS_NOEXCEPT
#define LB_HAS_NOEXCEPT \
	(__has_feature(cxx_noexcept) || __has_extension(cxx_noexcept) || \
		LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 2000)
#if !defined(LB_HAS_NOEXCEPT) && !defined(LB_IMPL_MSCPP)
#error "compiler must support noexcept"
#endif
#undef LB_HAS_THREAD_LOCAL
#define LB_HAS_THREAD_LOCAL \
	(__has_feature(cxx_thread_local) || (LB_IMPL_CPP >= 201103L \
		&& !LB_IMPL_GNUCPP))


//附加提示
//保证忽略时不导致运行时语义差异的提示,主要用于便于实现可能的优化

//属性
#if LB_IMPL_GNUCPP >= 20500
#define LB_ATTR(...) __attribute__((__VA_ARGS__))
#else
#define LB_ATTR(...)
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

//指定无返回值函数
#if LB_IMPL_GNUCPP >= 40800
#	define LB_NORETURN [[noreturn]]
#elif LB_IMPL_GNUCPP >= 20296
#	define LB_NORETURN LB_ATTR(__noreturn__)
#else
#	define LB_NORETURN
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

//库选项
//LB_DLL 使用动态链接库
//LB_BUILD_DLL 构建动态链接库
#if defined(LB_DLL) && defined(LB_BUILD_DLL)
#	error "DLL could not be built and used at the same time."
#endif

#ifdef LB_DLL
#	define LB_API __declspec(dllimport)
#elif defined(LB_BUILD_DLL)
#	define LB_API __declspec(dllexport)
#else
#	define LB_API
#endif

//指定类型的对齐
#if LB_HAS_ALIGNOF
#	define lalignof alignof
#else
#	define lalignof(_type) std::alignment_of<_type>::value
#endif

#if LB_HAS_ALIGNAS
#	define lalignas alignas
#else
#	define lalignas(_n) _declspec(align(_n))
#endif

//编译器常量
#if LB_HAS_CONSTEXPR && !LB_IMPL_MSCPP
#	define lconstexpr constexpr
#	define lconstfn constexpr
#else
#	define lconstexpr const
#	define lconstfn inline
#endif

//搞不懂的异常规范,跳过
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

#if LB_HAS_NOEXCEPT
#	define lnothrow lnoexcept
#elif LB_IMPL_GNUCPP >= 30300
#	define lnothrow __attribute__ ((nothrow))
#else
#	define lnothrow lthrow()
#endif

#if LB_HAS_NOEXCEPT
#	define lnoexcept noexcept
#else
#	define lnoexcept(...)
#endif

#if LB_HAS_THREAD_LOCAL && defined(_MT)
#	define lthread thread_local
#else
#	define lthread static
#endif

namespace leo
{
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

#if LB_HAS_NOEXCEPT
#define loffsetof(type,member) \
	(decltype(sizeof(leo::stdex::offsetof_check<std::is_member_object_pointer< \
	decltype(&type::member)>::value,lnoexcept(offsetof(type,member)), \
	type>))(offsetof(type,member)))
#else
#define loffsetof(type,member) \
	(decltype(sizeof(leo::stdex::offsetof_check<std::is_member_object_pointer< \
	decltype(&type::member)>::value,true, \
	type>))(offsetof(type,member)))
#endif
#define lforward(expr) std::forward<decltype(expr)>(expr)

		template<typename type, typename ...tParams>
		lconstfn auto
			unsequenced(type && arg, tParams&&...)->decltype(lforward(arg))
		{
			return lforward(arg);
		}

		//无序求值
#define lunseq leo::stdex::unsequenced
	}
}

#endif