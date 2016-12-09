#ifndef LScheme_LSchemeREPL_H
#define LScheme_LSchemeREPL_H 1

#include "LScheme.h"

namespace scheme {
	namespace v1 {
		/*
		\brief REPL 上下文。
		\warning 非虚析构。

		REPL 表示读取-求值-输出循环。
		每个循环包括一次翻译。
		这只是一个基本的可扩展实现。功能通过操作数据成员控制。
		*/
		class LS_API REPLContext
		{
		public:
			//! \brief 上下文根节点。
			ContextNode Root{};
			//! \brief 预处理节点：每次翻译时预先处理调用的公共例程。
			TermPasses Preprocess{};
			//! \brief 表项处理例程：每次翻译中规约回调处理调用的公共例程。
			EvaluationPasses ListTermPreprocess{};

			/*!
			\brief 构造：使用默认的解释。
			\note 参数指定是否启用对规约深度进行跟踪。
			\sa ListTermPreprocess
			\sa SetupDefaultInterpretation
			\sa SetupTraceDepth
			*/
			REPLContext(bool = {});

			/*!
			\brief 处理：分析输入并预处理后进行规约。
			\sa SContext::Analyze
			\sa Preprocess
			\sa Reduce
			*/
			//@{
			void
				Process(TermNode&);
			TermNode
				Process(const TokenList&);
			TermNode
				Process(const Session&);
			//@}

			/*!
			\brief 执行循环：对非空输入进行翻译。
			\pre 断言：字符串的数据指针非空。
			\throw LoggedEvent 输入为空串。
			\sa Process
			*/
			TermNode
				Perform(string_view);
		};

		namespace Forms
		{
			/*!
			\brief 创建 REPL 并对翻译单元规约以求值。
			*/
			LS_API void
				Eval(TermNode&, const REPLContext&);
		}
	}
}

#endif
