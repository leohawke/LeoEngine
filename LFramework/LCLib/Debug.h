#pragma once

#include "Platform.h"
#include "LDescriptions.h"
#include <LBase/cassert.h>

namespace platform_ex {
#if LF_Multithread == 1
	/*!
	\brief ��־���Ժ�����
	\note Ĭ�϶��� leo::lassert �������
	\warning �������Ҽ���������Ϊδ���塣Ӧֻ���ڵ�����;��
	\todo ������ Win32 �Ͻ�����Ϣ��

	�� Win32 ������ʹ��ͼ�ν��棨��Ϣ����ʾ����ʧ�ܡ�ע�ⲻ��������Ϣѭ���ڲ���
	������Բ�������������ֹ���򡣵�ѡ����ֹʱ�����ȷ��� \c SIGABRT �źš�
	���Դ˹��̵����д��󣬰������б��׳����쳣���������쳣�����������Ϊ��
	����������־��¼������� leo::lassert �����յ��� std::terminate ��ֹ����
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
	\brief �����ַ�������������
	\pre ��Ӷ��ԣ��ַ��������ǿա�
	\note ��ǰֱ�ӵ��� ::OutputDebugStringA ��
	*/
	LB_API LB_NONNULL(3) void
		SendDebugString(platform::Descriptions::RecordLevel, const char*)
		lnothrowv;

#endif
}

namespace platform
{
	/*!
	\brief ���Բ����طǿղ�����
	\pre ���ԣ������ǿա�
	*/
	template<typename _type>
	inline _type&&
		Nonnull(_type&& p) lnothrowv
	{
		LAssertNonnull(p);
		return lforward(p);
	}

	/*!
	\brief ���Բ����ؿɽ����õĵ�������
	\pre ���ԣ���������ȷ�����ɽ����á�
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
	\brief ���Բ������÷ǿ�ָ�롣
	\pre ʹ�� ADL ָ���� FwdIter ���ñ��ʽ��ֵ�ȼ��ڵ��� platform::FwdIter ��
	\pre ��Ӷ��ԣ�ָ��ǿա�
	*/
	template<typename _type>
	lconstfn auto
		Deref(_type&& p) -> decltype(*p)
	{
		return *FwdIter(lforward(p));
	}

} // namespace platform;