/*! \file Core\LSLEvaluator.h
\ingroup Engine
\brief LSL��ֵ��(�������ں˶���)��
*/
#ifndef LE_Core_LSLEvaluator_H
#define LE_Core_LSLEvaluator_H 1

#include <LScheme/SContext.h>
#include <LScheme/LScheme.h>
#include <LScheme/LSchemREPL.h>

namespace platform {
	using REPLContext = scheme::v1::REPLContext;

	/*
	\brief ��REPL�����ĵİ�װ,������һЩ����
	\brief ��ҪĿ������չDebug���,���ҿ��Լ̳���չ�������������
	*/
	class LSLEvaluator {
	private:
		//Terminal
		REPLContext context;

	public:
		LSLEvaluator(std::function<void(REPLContext&)>);

		virtual ~LSLEvaluator();

		/*!
		\brief ���أ���ָ������ָ������Դ��ȡ������Դ���롣
		\sa REPLContext::LoadFrom
		*/
		template<class _type>
		void
			LoadFrom(_type& input)
		{
			context.LoadFrom(input);
		}
		

		/*
		\brief ִ����ֵѭ��:�Էǿ�������з���
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