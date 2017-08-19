/*!	\file LShell.h
\ingroup LFrameWork/Core
\brief Shell ����
*/

#ifndef LFrameWork_Core_LShell_H
#define LFrameWork_Core_LShell_H 1

#include <LFramework/Core/lmsgdef.h>

namespace leo
{

	namespace Shells
	{
		//! \brief ��ǳ���ʵ�������ڿ�����ӳ�����塣
		class LF_API Shell : private noncopyable, public enable_shared_from_this<Shell>
		{
		public:
			/*!
			\brief �޲������졣
			*/
			DefDeCtor(Shell)
				/*!
				\brief ������
				*/
				virtual
				~Shell();

			/*!
			\brief �ж� Shell �Ƿ��ڼ���״̬��
			*/
			bool
				IsActive() const;

			/*!
			\brief Ĭ�� Shell ��������
			\note ����Ĭ�� Shell ����ΪӦ�ó���û�д���� Shell ��Ϣ�ṩĬ�ϴ���
			ȷ��ÿһ����Ϣ�õ�����
			*/
			static void
				DefShlProc(const Message&);

			/*!
			\brief ������Ϣ����Ӧ�̵߳�ֱ�ӵ��á�
			*/
			virtual PDefH(void, OnGotMessage, const Message& msg)
				ImplExpr(DefShlProc(msg))
		};

	} // namespace Shells;

} // namespace leo;

#endif
