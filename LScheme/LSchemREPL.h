#ifndef LScheme_LSchemeREPL_H
#define LScheme_LSchemeREPL_H 1

#include "LScheme.h"

namespace scheme {
	namespace v1 {
		/*
		\brief REPL �����ġ�
		\warning ����������

		REPL ��ʾ��ȡ-��ֵ-���ѭ����
		ÿ��ѭ������һ�η��롣
		��ֻ��һ�������Ŀ���չʵ�֡�����ͨ���������ݳ�Ա���ơ�
		*/
		class LS_API REPLContext
		{
		public:
			//! \brief �����ĸ��ڵ㡣
			ContextNode Root{};
			//! \brief Ԥ����ڵ㣺ÿ�η���ʱԤ�ȴ�����õĹ������̡�
			TermPasses Preprocess{};
			//! \brief ��������̣�ÿ�η����й�Լ�ص�������õĹ������̡�
			EvaluationPasses ListTermPreprocess{};

			/*!
			\brief ���죺ʹ��Ĭ�ϵĽ��͡�
			\note ����ָ���Ƿ����öԹ�Լ��Ƚ��и��١�
			\sa ListTermPreprocess
			\sa SetupDefaultInterpretation
			\sa SetupTraceDepth
			*/
			REPLContext(bool = {});

			/*!
			\brief ���أ���ָ������ָ������Դ��ȡ������Դ���롣
			\sa Process
			*/
			//@{
			//! \throw std::invalid_argument ��״̬����򻺳��������ڡ�
			void
				LoadFrom(std::istream&);
			void
				LoadFrom(std::streambuf&);
			//@}

			/*!
			\brief �����������벢Ԥ�������й�Լ��
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
			\brief ִ��ѭ�����Էǿ�������з��롣
			\pre ���ԣ��ַ���������ָ��ǿա�
			\throw LoggedEvent ����Ϊ�մ���
			\sa Process
			*/
			TermNode
				Perform(string_view);
		};

		/*!
		\brief ���Լ���Դ���롣
		\exception NPLException Ƕ���쳣������ʧ�ܡ�
		\note �ڶ���������ʾ��Դ�������������Ϣ��
		\relates REPLContext
		*/
		template<typename... _tParams>
		LB_NONNULL(2) void
			TryLoadSource(REPLContext& context, const char* name, _tParams&&... args)
		{
			TryExpr(context.LoadFrom(lforward(args)...))
				CatchExpr(..., std::throw_with_nested(LSLException(
					leo::sfmt("Failed loading external unit '%s'.", name))));
		}

		namespace Forms
		{
			//@{
			//! \brief ��������ָ���� REPL �ĸ����������жԷ��뵥Ԫ��Լ����ֵ��
			LS_API void
				EvaluateUnit(TermNode&, const REPLContext&);
			//@}
		}
	}
}

#endif
