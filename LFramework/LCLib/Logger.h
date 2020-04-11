/*!	\file Debug.h
\ingroup Framework
\brief ������ʩ��
*/

#ifndef FrameWork_Debug_h
#define FrameWork_Debug_h 1

#include <LFramework/LCLib/LDescriptions.h>
#include <LFramework/LCLib/FCommon.h>
#include <LFramework/LCLib/FContainer.h>
#include <LFramework/LCLib/Mutex.h>


/*!	\defgroup diagnostic Diagnostic
\brief �����ʩ��
*/

/*!	\defgroup debugging Debugging
\ingroup diagnostic
\brief ������ʩ��

���ں� NDEBUG δ������ʱ��������õĵ��Խӿں�ʵ�֡�
*/

/*!	\defgroup tracing Tracing
\ingroup diagnostic
\brief ������ʩ��

�������õ���������õĵ��Խӿں�ʵ�֡�
*/

/*!
\ingroup tracing
\def LF_Use_TraceSrc
\brief �ڸ�����־��ʹ�ø���Դ��λ�á�
*/
#ifndef NDEBUG
#	ifndef LF_Use_TraceSrc
#		define LF_Use_TraceSrc 1
#	endif
#endif

namespace platform
{

	/*!
	\ingroup diagnostic
	*/
	//@{
	//@}

	/*!
	\brief ʹ�ñ�׼�������ӡ�ַ�����ˢ�¡�
	\pre ��Ӷ��ԣ�����������ָ��ǿա�
	\return ��ӡ��ˢ���Ƿ�ɹ���

	ʹ�ñ�׼�������ƽ̨��صķ�ʽ��ӡ�ַ�����Ȼ��ˢ������������Ϊ UTF-8 ��
	DS ƽ̨��ʹ�� std::puts ��
	Win32 ƽ̨��ʹ�ÿ���̨�ӿڣ�ʧ��ʱʹ�� std::cout ��
	����ƽ̨��ʹ�� std::cout ��
	*/
	LB_API bool
		Echo(string_view) lnoexcept(false);

	/*!
	\ingroup tracing
	\brief ��־��¼����
	*/
	class LB_API Logger
	{
	public:
		using Level = Descriptions::RecordLevel;
		using Filter = std::function<bool(Level, Logger&)>;
		//! \note ���ݵĵ��������ǿա�
		using Sender = std::function<void(Level, Logger&, const char*)>;

#ifdef NDEBUG
		Level FilterLevel = Descriptions::Informative;
#else
		Level FilterLevel = Descriptions::Debug;
#endif

	private:
		//! \invariant <tt>bool(filter)</tt> ��
		Filter filter{ DefaultFilter };
		//! \invariant <tt>bool(Sender)</tt> ��
		Sender sender{ FetchDefaultSender() };
		/*!
		\brief ��־��¼����

		�� DoLog �� DoLogException �ڷ�����־ʱʹ�õ�����
		ʹ�õݹ����������û��ڷ������м�ӵݹ���� DoLog �� DoLogException ��
		*/
		Concurrency::recursive_mutex record_mutex;

	public:
		//@{
		/*!
		\brief �޲������죺ʹ�ù������ͷ�������
		\note �쳣������ͬ FetchDefaultSender ��
		\note �� std::function �Ĺ���ģ���ṩ�ı�֤ȷ���������쳣�׳���
		*/
		DefDeCtor(Logger)
			//! \brief ���ƹ��죺���ƹ��˵ȼ����������ͷ�������ʹ���´����Ļ�������
		Logger(const Logger&);
		/*!
		\brief ת�ƹ��죺ת�ƹ��˵ȼ���ת�ƹ������ͷ�������ʹ���´����Ļ�������
		\post ��ת�Ƶ���־�������Ĭ�ϵĹ������ͷ�������
		\see LWG 2062 ��
		*/
		Logger(Logger&&) lnothrow;

		DefDeCopyMoveAssignment(Logger)
		//@}

		DefGetter(const lnothrow, const Sender&, Sender, sender)

		/*!
		\brief ���ù�������
		\note ���Կչ�������
		*/
		void
		SetFilter(Filter);
		/*!
		\brief ���÷�������
		\note ���Կշ�������
		*/
		void
			SetSender(Sender);

		/*!
		\brief ������־��¼ִ��ָ��������
		\note �̰߳�ȫ��������ʡ�
		*/
		template<typename _func>
		auto
			AccessRecord(_func f) -> decltype(f())
		{
			Concurrency::lock_guard<Concurrency::recursive_mutex> lck(record_mutex);

			return f();
		}

		//! \brief Ĭ�Ϲ��ˣ�������ȼ���������ֵ����־����¼��
		static bool
			DefaultFilter(Level, Logger&) lnothrow;

		//! \pre ��Ӷ��ԣ����������ǿա�
		//@{
		/*!
		\brief Ĭ�Ϸ�������ʹ�� std::cerr �����
		*/
		static LB_NONNULL(3) void
			DefaultSendLog(Level, Logger&, const char*) lnothrowv;

		/*!
		\brief Ĭ�Ϸ�������ʹ�� stderr �����
		*/
		static LB_NONNULL(3) void
			DefaultSendLogToFile(Level, Logger&, const char*) lnothrowv;
		//@}

		/*!
		\brief ת���ȼ�����־����������
		\note �����ַ���������Ӧ�Ŀ�����ָ�������
		\note ��֤���з��͡�
		*/
		//@{
		void
			DoLog(Level, const char*);
		PDefH(void, DoLog, Level lv, string_view sv)
			ImplRet(DoLog(lv, sv.data()))
			//@}

	private:
		/*!
		\brief ת���ȼ�����־����������
		\pre ��Ӷ��ԣ��ַ���������Ӧ������ָ��ǿա�
		*/
		//@{
		LB_NONNULL(1) void
			DoLogRaw(Level, const char*);
		PDefH(void, DoLogRaw, Level lv, string_view sv)
			ImplRet(DoLogRaw(lv, sv.data()))
			//@}

	public:
		/*!
		\brief ת���ȼ����쳣��������������

		�����쳣����ȷ����־�ַ��������͡���ת��ʱ�׳��쳣�����¼���쳣��
		�Բ���������֤���з��ͣ����������̳����� record_mutex �Ա�֤�����ԡ�
		*/
		void
			DoLogException(Level level, const std::exception&) lnothrow;

		/*!
		\brief ȡ�½���ƽ̨��ص�Ĭ�Ϸ��ͣ���ָ���ı�ǩȡƽ̨���ʵ�֡�
		\pre ���ԣ�����������ָ��ǿա�
		*/
		static Sender
			FetchDefaultSender(string_view = "Leo");

		template<typename _fCaller, typename... _tParams>
		void
			Log(Level level, _fCaller&& f, _tParams&&... args)
		{
			if (filter(level, *this))
				TryExpr(DoLog(level, lforward(f)(lforward(args)...)))
				CatchExpr(std::exception& e, DoLogException(level, e), throw)
		}

		/*!
		\brief Ĭ�Ϸ�������ʹ�õ�һ����ָ�����������
		\pre ��Ӷ��ԣ��ַ��������ǿա�
		*/
		//@{
		static LB_NONNULL(4) void
			SendLog(std::ostream&, Level, Logger&, const char*) lnothrowv;

		//! \pre ���ԣ���ָ��ǿա�
		static LB_NONNULL(1, 4) void
			SendLogToFile(std::FILE*, Level, Logger&, const char*) lnothrowv;
		//@}

		/*!
		\brief �������������л�������������ݳ�Ա��
		*/
		LF_API friend void
			swap(Logger&, Logger&) lnothrow;
	};


	/*!
	\brief ȡ������־��¼����
	*/
	LB_API Logger&
		FetchCommonLogger();


	/*!
	\brief ��ʽ�����־�ַ���ǰ׷�Ӽ�¼Դ�ļ������кš�
	\pre ��Ӷ��ԣ����������ǿա�
	\note �����һ����Ϊ��ָ�룬��Ϊδ֪��
	\note ��ʧ��ʱ���� leo::trace ��¼����ֻ���������е��ļ������кš�
	*/
	LB_API LB_NONNULL(1, 3) string
		LogWithSource(const char*, int, const char*, ...) lnothrow;


	/*!
	\brief ʹ�ù�����־��¼����¼��־��ʽ�ַ�����
	\note ֧�ָ�ʽͬ std::fprintf ��
	\note ʹ�� FetchCommonLogger ��֤���������
	*/
#define LFL_Log(_lv, ...) \
	platform::FetchCommonLogger().Log( \
		platform::Descriptions::RecordLevel(_lv), __VA_ARGS__)

	/*!
	\brief ֱ�ӵ��Ը��١�
	\note ����Դ������Ϣ��
	*/
#define LF_TraceRaw(_lv, ...) \
	LFL_Log(_lv, [&]() -> platform::string { \
		using platform::sfmt; \
	\
		TryRet(sfmt(__VA_ARGS__)) \
		CatchRet(..., {}) \
	})

	/*!
	\def LF_Trace
	\brief Ĭ�ϵ��Ը��١�
	\note ���쳣�׳���
	*/
#if LF_Use_TraceSrc
#	define LF_Trace(_lv, ...) \
	LFL_Log(_lv, [&]{ \
		return platform::LogWithSource(__FILE__, __LINE__, __VA_ARGS__); \
	})
#else
#	define LF_Trace(_lv, ...) LF_TraceRaw(_lv, __VA_ARGS__)
#endif


	/*!
	\def TraceDe
	\brief Ĭ�ϵ��Ը��١�
	\note ʹ��Ĭ�ϵĵ��Ը��ټ���
	\sa LFL_Trace
	*/
#if LB_Use_LTrace
#	define TraceDe(_lv, ...) LF_Trace(_lv, __VA_ARGS__)
#else
#	define TraceDe(...)
#endif

} // namespace platform;






#endif
