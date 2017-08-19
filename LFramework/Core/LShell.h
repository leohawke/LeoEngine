/*!	\file LShell.h
\ingroup LFrameWork/Core
\brief Shell 抽象。
*/

#ifndef LFrameWork_Core_LShell_H
#define LFrameWork_Core_LShell_H 1

#include <LFramework/Core/lmsgdef.h>

namespace leo
{

	namespace Shells
	{
		//! \brief 外壳程序：实现运行期控制流映像语义。
		class LF_API Shell : private noncopyable, public enable_shared_from_this<Shell>
		{
		public:
			/*!
			\brief 无参数构造。
			*/
			DefDeCtor(Shell)
				/*!
				\brief 析构。
				*/
				virtual
				~Shell();

			/*!
			\brief 判断 Shell 是否处于激活状态。
			*/
			bool
				IsActive() const;

			/*!
			\brief 默认 Shell 处理函数。
			\note 调用默认 Shell 函数为应用程序没有处理的 Shell 消息提供默认处理，
			确保每一个消息得到处理。
			*/
			static void
				DefShlProc(const Message&);

			/*!
			\brief 处理消息：响应线程的直接调用。
			*/
			virtual PDefH(void, OnGotMessage, const Message& msg)
				ImplExpr(DefShlProc(msg))
		};

	} // namespace Shells;

} // namespace leo;

#endif
