/*!	\file LException.h
\ingroup FrameWork
\brief �쳣����ģ�顣
*/

#ifndef FrameWork__LException_h
#define FrameWork__LException_h 1

#include <exception>
#include <stdexcept>
#include <string>
#include <LFramework/Core/LShellDefinition.h>
#include <LBase/functional.hpp>

namespace leo
{
	//! \ingroup exceptions
	//@{
	//! \brief һ������ʱ�쳣�¼��ࡣ
	using GeneralEvent = std::runtime_error;


	//! \brief ��¼��־���쳣�¼��ࡣ
	class LB_API LoggedEvent : public GeneralEvent
	{
	private:
		RecordLevel level;

	public:
		/*!
		\brief ���죺ʹ���쳣�ַ����ͼ�¼�ȼ���
		*/
		//@{
		//! \pre ��Ӷ��ԣ���һ�����ǿա�
		LB_NONNULL(2)
			LoggedEvent(const char* = "", RecordLevel = Emergent);
		LoggedEvent(const string_view, RecordLevel = Emergent);
		//@}
		/*!
		\brief ���죺ʹ��һ���쳣�¼�����ͼ�¼�ȼ���
		*/
		LoggedEvent(const GeneralEvent&, RecordLevel = Emergent);
		DefDeCopyCtor(LoggedEvent)
			/*!
			\brief ���������ඨ����Ĭ��ʵ�֡�
			*/
		virtual	~LoggedEvent() override;

		RecordLevel GetLevel() const lnothrow;
	};


	/*!
	\brief ��������
	\note �ⲻֱ�Ӳ�����������������쳣��
	*/
	class LB_API FatalError : public GeneralEvent
	{
	private:
		/*!
		\invariant \c content ��
		\todo ʹ�����ü���ʵ�ֵ��ַ�����
		*/
		shared_ptr<string> content;

	public:
		/*!
		\brief ���죺ʹ�ñ�������ݡ�
		\pre ��Ӷ��ԣ���һ�����͵ڶ�����������ָ��ǿա�
		\note �����ַ���������
		*/
		LB_NONNULL(2)
			FatalError(const char*, string_view);
		DefDeCopyCtor(FatalError)
			/*!
			\brief ���������ඨ����Ĭ��ʵ�֡�
			*/
			~FatalError() override;

		string_view GetContent() const lnothrow;
		const char* GetTitle() const lnothrow;
	};
	//@}


	//@{
	/*!
	\brief ��ӡ���в����Ϣ�ĺ������͡�
	\note Լ����һ�������ǿա�
	*/
	using ExtractedLevelPrinter
		= std::function<void(const char*, RecordLevel, size_t)>;
	template<typename _type>
	using GLevelTracer = std::function<void(_type, RecordLevel)>;
	using ExceptionTracer = GLevelTracer<const std::exception&>;


	/*!
	\brief ͨ�� YCL_TraceRaw ���ٴ��ո�������ε��쳣��Ϣ�ĺ������͡�
	\pre ���ԣ���һ�����ǿա�
	*/
	LB_API LB_NONNULL(1) void
		TraceException(const char*, RecordLevel = Err, size_t level = 0) lnothrow;

	/*!
	\brief ͨ�� YCL_TraceRaw ���ټ�¼�쳣���͡�
	\since build 658
	\todo �����������ơ�
	*/
	LB_API void
		TraceExceptionType(const std::exception&, RecordLevel = Err) lnothrow;

	/*!
	\brief ʹ�� TraceException չ���͸����쳣���ͺ���Ϣ��
	\sa ExtraceException
	\sa TraceException
	\sa TraceExceptionType
	*/
	LB_API void
		ExtractAndTrace(const std::exception&, RecordLevel = Err) lnothrow;

	//! \brief չ��ָ����ε��쳣��ʹ��ָ��������¼��
	LB_API void
		ExtractException(const ExtractedLevelPrinter&,
			const std::exception&, RecordLevel = Err, size_t = 0) lnothrow;
	//@}

	//! \return �Ƿ����������쳣��
	//@{
	/*!
	\brief ִ�в���ͼ��¼�쳣��

	�Բ���ָ���ĺ�����ֵ����ʹ�����һ���������ټ�¼�쳣��
	*/
	LB_API bool
		TryExecute(std::function<void()>, const char* = {}, RecordLevel = Alert,
			ExceptionTracer = ExtractAndTrace);

	/*!
	\brief ���ú�������ͼ���ء�
	*/
	template<typename _fCallable, typename... _tParams>
	nonvoid_result_t<result_of_t<add_rvalue_reference_t<_fCallable>(_tParams&&...)>>
		TryInvoke(_fCallable&& f, _tParams&&... args) lnothrow
	{
		TryRet(leo::invoke_nonvoid(lforward(f), lforward(args)...))
			CatchExpr(std::exception& e, TraceExceptionType(e, Emergent))
			CatchExpr(..., LFL_TraceRaw(Emergent, "Unknown exception found."))
			return{};
	}

	/*!
	\brief ���ú��������������쳣��
	\note ʹ�� ADL \c TryInvoke �� \c TryExecute ��

	�Բ���ָ���ĺ�����ֵ��������͸��ټ�¼�����쳣��
	*/
	template<typename _func>
	bool
		FilterExceptions(_func f, const char* desc = {}, RecordLevel lv = Alert,
			ExceptionTracer trace = ExtractAndTrace) lnothrow
	{
		return !TryInvoke([=] {
			return !TryExecute(f, desc, lv, trace);
		});
	}
	//@}

} // namespace leo;



#endif
