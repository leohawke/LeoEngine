/*! \file Core\LSLEvaluator.h
\ingroup Engine
\brief LSL求值器(解释器内核对象)。
*/
#ifndef LE_Core_LSLEvaluator_H
#define LE_Core_LSLEvaluator_H 1

#include <LScheme/SContext.h>
#include <LScheme/LScheme.h>
#include <LScheme/LSchemREPL.h>

namespace platform {
	using REPLContext = scheme::v1::REPLContext;

	/*
	\brief 对REPL上下文的包装,并屏蔽一些函数
	\brief 主要目的是扩展Debug输出,并且可以继承扩展包含多个上下文
	*/
	class LSLEvaluator {
	private:
		//Terminal
		REPLContext context;

	public:
		LSLEvaluator(std::function<void(REPLContext&)>);

		virtual ~LSLEvaluator();

		/*!
		\brief 加载：从指定参数指定的来源读取并处理源代码。
		\sa REPLContext::LoadFrom
		*/
		template<class _type>
		void
			LoadFrom(_type& input)
		{
			context.LoadFrom(input);
		}
		

		/*
		\brief 执行求值循环:对非空输入进行翻译
		\sa REPLContext::Process
		*/
		//@{
		template<class _type>
		scheme::TermNode
			Eval(const _type& input)
		{
			return context.Process(input);
		}

		/*
		\sa REPLContext::Perform
		*/
		scheme::TermNode 
			Eval(scheme::string_view unit) {
			return context.Perform(unit);
		}

		//@}
	};
}

#endif