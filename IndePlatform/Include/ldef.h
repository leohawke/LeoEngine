////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/ldef.hpp
//  Version:     v1.02
//  Created:     02/06/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 
// -------------------------------------------------------------------------
//  History:
//		2014-9-27 11:02: ǿ��Ҫ�������֧��alignas�Լ�noexcept(��VS14 CTP1֧������Ϊ��)
//		2015-3-9  11:22  ����Ҫ�������֧��noexcept(��VS14 CTP5֧������Ϊ��),����ʽ��ע��
//
////////////////////////////////////////////////////////////////////////////
#ifndef IndePlatform_ldef_h
#define IndePlatform_ldef_h

/*!	\defgroup lang_impl_versions Language Implementation Versions
\brief ����ʵ�ֵİ汾��
\since build 373
*/
//@{

/*!
\def LB_IMPL_CPP
\brief C++ʵ��֧�ְ汾
\since build 1.02

����Ϊ __cplusplus
*/
#ifdef __cplusplus
#define LB_IMPL_CPP __cplusplus
#else
# error "This header is only for C++."
#endif



/*!
\def LB_IMPL_MSCPP
\brief Microsof C++ ʵ��֧�ְ汾
\since build 1.02

����Ϊ _MSC_VER �����İ汾��
*/

/*!
\def LB_IMPL_GNUCPP
\brief GNU C++ ʵ��֧�ְ汾
\since build 1.02

����Ϊ 100���Ƶ����ذ汾��ź�
*/

/*!
\def LB_IMPL_CLANGCPP
\brief LLVM/Clang++ C++ ʵ��֧�ְ汾
\since build 1.02

����Ϊ 100���Ƶ����ذ汾��ź�
*/
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

//��ֹCL�������İ�ȫ����
#if LB_IMPL_MSCPP >= 1400
//��ָ������������������error C4996
//See doucumentation on how to use Visual C++ 'Checked Iterators'
#undef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

//ʹ�ò���ȫ�Ļ���������������error C4996
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
//@}

/*!
\brief ���Լ��겹�䶨�壺�����������滻ΪԤ�����Ǻ� 0 ��
\since build 1.02
*/
//@{
#ifndef __has_feature
#	define __has_feature(...) 0
#endif

#ifndef __has_extension
#	define __has_extension(...) 0
#endif

//! \since build 535
#ifndef __has_builtin
#	define __has_builtin(...) 0
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
\brief Ԥ������ͨ�����ֺꡣ
\since build 1.02
*/
//@{
//! \brief �滻Ϊ�յ�Ԥ�����Ǻš�
#define LPP_Empty

/*!
\brief �滻Ϊ���ŵ�Ԥ�����Ǻš�
\note �����ڴ�����ʵ�ʲ����г��ֵĶ��š�
*/
#define LPP_Comma ,

/*
\brief �Ǻ����ӡ�
\sa YPP_Join
*/
#define LPP_Concat(x,y) x ## y

/*
\brief �����滻�ļǺ����ӡ�
\see ISO WG21/N4140 16.3.3[cpp.concat]/3 ��
\see http://gcc.gnu.org/onlinedocs/cpp/Concatenation.html ��
\see https://www.securecoding.cert.org/confluence/display/cplusplus/PRE05-CPP.+Understand+macro+replacement+when+concatenating+tokens+or+performing+stringification ��

ע�� ISO C++ δȷ���궨���� # �� ## ��������ֵ˳��
ע��궨���� ## ���������ε���ʽ����Ϊ��ʱ���˺겻�ᱻչ����
*/
#define LPP_Join(x,y) LPP_Concat(x,y)
//@}


/*!
\brief ʵ�ֱ�ǩ��
\since build 1.02
\todo �������ʵ�ֵı�Ҫ֧�֣��ɱ�����ꡣ
*/
#define limpl(...) __VA_ARGS__


/*!	\defgroup lang_impl_features Language Implementation Features
\brief ����ʵ�ֵ����ԡ�
\since build 1.02
*/
//@{

/*!
\def LB_HAS_ALIGNAS
\brief �ڽ� alignas ֧�֡�
\since build 1.02
*/
#undef  LB_HAS_ALIGNAS
#define LB_HAS_ALIGNAS \
	(__has_feature(cxx_alignas) || __has_extension(cxx_alignas) || \
		LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1900)

/*!
\def LB_HAS_ALIGNOF
\brief �ڽ� alignof ֧�֡�
\since build 1.02
*/
#undef LB_HAS_ALIGNOF
#define LB_HAS_ALIGNOF (LB_IMPL_CPP >= 201103L || LB_IMPL_GNUCPP >= 40500 || LB_IMPL_MSCPP >= 1900)

/*!
\def LB_HAS_BUILTIN_NULLPTR
\brief �ڽ� nullptr ֧�֡�
\since build 1.02
*/
#undef LB_HAS_BUILTIN_NULLPTR
#define LB_HAS_BUILTIN_NULLPTR \
	(__has_feature(cxx_nullptr) || __has_extension(cxx_nullptr) || \
	 LB_IMPL_CPP >= 201103L ||  LB_IMPL_GNUCPP >= 40600 || \
	 LB_IMPL_MSCPP >= 1600)

/*!
\def LB_HAS_CONSTEXPR
\brief constexpr ֧�֡�
\since build 1.02
*/
#undef LB_HAS_CONSTEXPR
#define LB_HAS_CONSTEXPR \
	(__has_feature(cxx_constexpr) || __has_extension(cxx_constexpr) || \
	LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1900)

/*!
\def LB_HAS_NOEXCPT
\brief noexcept ֧�֡�
\since build 1.02
*/
#undef LB_HAS_NOEXCEPT
#define LB_HAS_NOEXCEPT \
	(__has_feature(cxx_noexcept) || __has_extension(cxx_noexcept) || \
		LB_IMPL_CPP >= 201103L || LB_IMPL_MSCPP >= 1900)

/*!
\def LB_HAS_THREAD_LOCAL
\brief thread_local ֧�֡�
\see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=1773 ��
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
\brief ��������ֹ����
\note ����ʹ����ϵ�ṹ���ʵ�ֻ��׼�� std::abort �����ȡ�
\since build 1.02
*/
#if __has_builtin(__builtin_trap) || LB_IMPL_GNUCPP >= 40203
#	define LB_ABORT __builtin_trap()
#else
#	define LB_ABORT std::abort()
#endif

/*!
\def LB_ASSUME(expr)
\brief �ٶ�����ʽ���ǳ�����
\note ���ٶ������������Ż���
\warning ���ٶ�����������Ϊδ���塣
\since build 1.02
*/
#if LB_IMPL_MSCPP >= 1200
#	define LB_ASSUME(_expr) __assume(_expr)
#elif __has_builtin(__builtin_unreachable) || LB_IMPL_GNUCPP >= 40500
#	define LB_ASSUME(_expr) (_expr) ? void(0) : __builtin_unreachable()
#else
#	define LB_ASSUME(_expr) (_expr) ? void(0) : LB_ABORT
#endif

//������ʾ
//��֤����ʱ����������ʱ����������ʾ,��Ҫ���ڱ���ʵ�ֿ��ܵ��Ż�

//����
#if LB_IMPL_GNUCPP >= 20500
#define LB_ATTR(...) __attribute__((__VA_ARGS__))
#else
#define LB_ATTR(...)
#endif

//���ε��Ƿ�����,�򷵻ط��������õĺ�������ģ��
//��Ϊ���� std::malloc �� std::calloc
//���������طǿ�ָ��,���ص�ָ�벻��������Чָ��ı���,��ָ��ָ�����ݲ��������洢����
#if LB_IMPL_GNUCPP >= 20296
#	define LB_ALLOCATOR LB_ATTR(__malloc__)
#else
#	define LB_ALLOCATOR
#endif

//��֧Ԥ����ʾ
#if LB_IMPL_GNUCPP >= 29600
#	define LB_EXPECT(expr, constant) (__builtin_expect(expr, constant))
#	define LB_LIKELY(expr) (__builtin_expect(bool(expr), 1))
#	define LB_UNLIKELY(expr) (__builtin_expect(bool(expr), 0))
#else
#	define LB_EXPECT(expr, constant) (expr)
#	define LB_LIKELY(expr) (expr)
#	define LB_UNLIKELY(expr) (expr)
#endif

//ָ���޷���ֵ����
#if LB_IMPL_GNUCPP >= 40800
#	define LB_NORETURN [[noreturn]]
#elif LB_IMPL_GNUCPP >= 20296
#	define LB_NORETURN LB_ATTR(__noreturn__)
#else
#	define LB_NORETURN
#endif

//ָ����������ģ��ʵ��Ϊ������
//�ٶ������ɷ���,�ٶ��������ⲿ�ɼ��ĸ�����
#if LB_IMPL_GNUCPP >= 20296
#	define LB_PURE LB_ATTR(__pure__)
#else
#	define LB_PURE
#endif

//ָ������Ϊ��״̬����
#if LB_IMPL_GNUCPP >= 20500
#	define LB_STATELESS LB_ATTR(__const__)
#else
#	define LB_STATELESS
#endif

/*!	\defgroup lib_options Library Options
\brief ��ѡ�
\since build 1.02
*/
//@{

/*!
\def LB_DLL
\brief ʹ�� IndePlatform ��̬���ӿ⡣
\since build 1.02
*/
/*!
\def LB_BUILD_DLL
\brief ���� IndePlatform ��̬���ӿ⡣
\since build 1.02
*/
/*!
\def LB_API
\brief IndePlatform Ӧ�ó����̽ӿڣ���������ļ�Լ�����ӡ�
\since build 1.02
\todo �ж���������֧�֡�
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
\brief ʹ�ö��ԡ�
\since build 1.02
*/
/*!
\def LB_Use_LTrace
\brief ʹ�õ��Ը��ټ�¼��
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
\brief ʹ�� IndePlatform ��̬�쳣�淶��
\since build 1.02
*/
#if 0 && !defined(NDEBUG)
#	define LB_USE_EXCEPTION_SPECIFICATION 1
#endif


//@}

/*!	\defgrouppseudo_keyword Specified Pseudo-Keywords
\brief IndePlatform ָ��������ؼ��֡�
\since build 1.02
*/
//@{

/*!
\ingroup pseudo_keyword
\def lalignof
\brief ��ѯ�ض����͵Ķ����С��
\note ͬ C++11 alignof ����������ʱ�����塣
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
\brief ָ���ض����͵Ķ����С��
\note ͬ C++11 alignas ����������ʱ�����塣
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
\brief ָ������ʱ��������ʽ��
\note ͬ C++11 constepxr �����ڱ���ʱ���������塣
\since build 1.02
*/
/*!
\ingroup pseudo_keyword
\def lconstfn
\brief ָ������ʱ����������
\note ͬ C++11 constepxr �����ڱ���ʱ�������������塣
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
\ingroup pseudo_keyword
\def lthrow
\brief IndePlatform ��̬�쳣�淶�������Ƿ�ʹ���쳣�淶��ָ������Զ�̬�쳣�淶��
\note lthrow = "yielded throwing" ��
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
\brief ���쳣�׳���֤��ָ���ض����쳣�淶��
\since build 1.02
*/
#if LB_HAS_NOEXCEPT
#	define lnoexcept noexcept
#else
#	define lnoexcept(...)
#endif

#if LB_HAS_NOEXCEPT
#	define lnothrow lnoexcept
#elif LB_IMPL_GNUCPP >= 30300
#	define lnothrow __attribute__ ((nothrow))
#else
#	define lnothrow lthrow()
#endif


/*!
\ingroup pseudo_keyword
\def lthread
\brief �ֲ߳̾��洢����ʵ��֧�֣�ָ��Ϊ \c thread_local ��
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

namespace stdex
{

	//char��unsigned��signedָ��
	using byte = unsigned char;
#if  CHAR_BIT == 8
	//һ�ֽڲ���һ������8λ!
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

	//tuple,pair����Ĺ�������
	using raw_tag = empty_base<>;

	//��Ա���㾲̬���ͼ��. 
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
	\brief ���ݲ�������ʹ�� std::forward ���ݶ�Ӧ������
	\since build 1.02

	���ݲ����������ͱ���ֵ���(value catory) �� const ���η���
	������ʽ����Ϊ����������������ʱ�����Ϊ��ֵ(lvalue) ������
	���ҽ�����ֵ��������ʱ���Ϊ��ֵ����ʱ���Ͳ��䣩��
	������Ϊ��Ӧ����ֵ�������͵�����ֵ(xvalue) ��
	*/
#define lforward(expr) std::forward<decltype(expr)>(expr)

	template<typename type, typename ...tParams>
	lconstfn auto
		unsequenced(type && arg, tParams&&...)->decltype(lforward(arg))
	{
		return lforward(arg);
	}

	//������ֵ
#define lunseq leo::stdex::unsequenced

}

#endif