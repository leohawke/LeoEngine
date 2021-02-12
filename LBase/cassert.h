/*! \file cassert.h
\ingroup LBase
\breif ISO C ����/���Ը�����չ��
\note Ĭ�����ʹ�ò���ϵͳ������Ϣ����ӿ�
\todo ���ڽӿ���չ����̨������ļ����
*/

#ifndef LBase_cassert_h
#define LBase_cassert_h 1

#include "LBase/ldef.h"
#include <cstdio>

namespace platform {
	/*!
	\brief LBase Ĭ��Debug�������
	\note �ú���ʹ�ò���ϵͳ������Ϣ����ӿ�
	\note ����û�еĽӿڣ����ᶨλ��wcerr/cerr/stderr
	\note �ú����������"����"�������Ϣ[�����������Ϣ����]
	*/
	LB_API void
		ldebug(const char*, ...) lnothrow;

	LB_API void
		ldebug(const wchar_t*, ...) lnothrow;
}


namespace leo
{
	/*!
	\brief LBase Ĭ�϶��Ժ�����
	\note ������� LB_Use_LAssert ������ 0 ʱ���� LAssert �����ɴ˺���ʵ�֡�
	\note ��������Ϊ�����ʽ���ļ������кź���Ϣ�ı���
	\note �����ָ���������Ϊδ֪��
	\note ���� std::terminate ��ֹ����
	*/
	LB_NORETURN LB_API void
		lassert(const char*, const char*, int, const char*) lnothrow;

#if LB_Use_LTrace
	/*!
	\brief LBase ���Ը��ٺ�����
	\note ������� LB_Use_YTrace ������ 0 ʱ���� LTrace �����ɴ˺���ʵ�֡�
	*/
	LB_API  void
		ltrace(std::FILE*, std::uint8_t, std::uint8_t, const char*, int, const char*,
			...) lnothrow;
#endif

} // namespace leo;

#include <cassert>

#undef lconstraint
#undef lassume

  /*!
  \ingroup LBase_pseudo_keyword
  \def lconstraint
  \brief Լ�����ӿ����塣
  \note ����ͨ�������ǿ���ӿ���Լ������ֲ�ض���ƽ̨ʵ��ʱӦ�����ر�ע�⡣
  \note ��֤���� ISO C++11 constexpr ģ�塣
  \see $2015-10 @ %Documentation::Workflow::Annual2015.

  ����ʱ���Ľӿ�����Լ�����ԡ�������˶��Ե���Ϊ�ǽӿ���ȷ��δ����ģ���Ϊ����Ԥ�⡣
  */
#ifdef NDEBUG
#define ldebug(format,...) (void)0
#else
#define ldebug(format,...) ::platform::ldebug(format,__VA_ARGS__)
#endif

#ifdef NDEBUG
#	define lconstraint(_expr) LB_ASSUME(_expr)
#else
#	define lconstraint(_expr) \
	((_expr) ? void(0) : leo::lassert(#_expr, __FILE__, __LINE__, \
		"Constraint violation."))
#endif

  /*!
  \ingroup LBase_pseudo_keyword
  \def lassume
  \brief �ٶ����������塣
  \note ����ͨ�������ǿ���ǽӿ���Լ������ֲ�ض���ƽ̨ʵ��ʱӦ�����ر�ע�⡣

  ����ʱ���Ļ�������Լ�����ԡ�������ȷ�ط� lconstraint ���õ����Ρ�
  */
#ifdef NDEBUG
#	define lassume(_expr) LB_ASSUME(_expr)
#else
#	define lassume(_expr) assert(_expr)
#endif


#ifndef LAssert
#	if LB_Use_LAssert
#		define LAssert(_expr, _msg) \
	((_expr) ? void(0) : leo::lassert(#_expr, __FILE__, __LINE__, _msg))
#	else
#		define LAssert(_expr, _msg) lassume(_expr)
#	endif
#endif
#pragma warning(disable:4800)
#ifndef LAssertNonnull
#	define LAssertNonnull(_expr) LAssert(bool(_expr), "Null reference found.")
#endif

#ifndef LTrace
  //@{
  /*!
  \brief LBase ��չ���Ը��١�
  \note ʹ���Զ���ĵ��Ը��ټ���
  \sa ltrace
  */
#	if LB_Use_LTrace
#		define LTrace(_stream, _lv, _t, _msg, ...) \
	leo::ltrace(_stream, _lv, _t, __FILE__, __LINE__, _msg, __VA_ARGS__)
#	else
#		define LTrace(...)
#	endif
#endif
#endif
