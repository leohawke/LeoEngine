#include <LFramework/Helper/HostWindow.h>
#include <LFramework/Core/LShell.h>
#include <LBase/scope_gurad.hpp>

namespace leo {
	

	using Messaging::MessageQueue;

	/*!
	\brief ����ʵ����
	*/
	class LF_API Application : public Shell
	{
	private:
		/*!
		\brief ��ʼ���ػ���
		*/
		stack<leo::any> on_exit{};
		/*
		\brief ��ʼ���ػ���������
		*/
		recursive_mutex on_exit_mutex{};
		/*
		\brief ����Ϣ���л�������
		*/
		recursive_mutex queue_mutex{};

	protected:
		/*
		\brief ����Ϣ���С�
		*/
		MessageQueue qMain{};
		/*!
		\brief ��ǰ Shell �����ָʾ��ǰ�߳̿ռ������е� Shell ��
		\note ȫ�ֵ��̣߳��������������ͬ��
		*/
		shared_ptr<Shell> hShell{};

	public:
		//! \brief �޲������죺Ĭ�Ϲ��졣
		Application();

		//! \brief �������ͷ� Shell ����Ȩ��������Դ��
		~Application() override;

		//! \brief ȡ���߳̿ռ��е�ǰ���е� Shell �ľ����
		DefGetter(const lnothrow, shared_ptr<Shell>, ShellHandle, hShell)

			/*!
			\brief ִ����Ϣ���в�����
			\note �̰߳�ȫ��ȫ����Ϣ���л�����ʡ�
			*/
			template<typename _func>
		auto
			AccessQueue(_func f) -> decltype(f(qMain))
		{
			lock_guard<recursive_mutex> lck(queue_mutex);

			return f(qMain);
		}

		/*!
		\pre �����������쳣�׳���
		\note �̰߳�ȫ��ȫ�ֻ�����ʡ�
		*/
		//@{
		template<typename _tParam>
		void
			AddExit(_tParam&& arg)
		{
			lock_guard<recursive_mutex> lck(on_exit_mutex);

			PushExit(lforward(arg));
		}

		template<typename _func>
		void
			AddExitGuard(_func f)
		{
			static_assert(std::is_nothrow_copy_constructible<_func>(),
				"Invalid guard function found.");
			lock_guard<recursive_mutex> lck(on_exit_mutex);

			TryExpr(PushExit(leo::unique_guard(f)))
				catch (...)
			{
				f();
				throw;
			}
		}

		/*!
		\brief ������ӵĳ�ʼ���ػ���
		\note �̰߳�ȫ��ȫ�ֳ�ʼ���ػ�������ʡ�
		*/
		template<typename _tParam>
		locked_ptr<leo::decay_t<_tParam>, recursive_mutex>
			LockAddExit(_tParam&& arg)
		{
			unique_lock<recursive_mutex> lck(on_exit_mutex);

			PushExit(lforward(arg));
			return { leo::unchecked_any_cast<leo::decay_t<_tParam>>(
				&on_exit.top()), std::move(lck) };
		}
		//@}

		/*!
		\brief ������Ϣ���ַ���Ϣ��
		\pre ���ԣ���ǰ Shell �����Ч��
		\exception ���񲢺��� Messaging::MessageSignal �������쳣������
		*/
		void
			OnGotMessage(const Message&) override;

	private:
		template<typename _tParam>
		inline void
			PushExit(_tParam&& arg)
		{
			on_exit.push(lforward(arg));
		}

	public:
		/*!
		\brief �л����������ǿգ����߳̿ռ��е�ǰ���е� Shell �ľ��������
		\return �����Ƿ���Ч��
		*/
		bool
			Switch(shared_ptr<Shell>&) lnothrow;
		/*!
		\brief �л����������ǿգ����߳̿ռ��е�ǰ���е� Shell �ľ��������
		\return �����Ƿ���Ч��
		\warning �վ���ڴ˴��ǿɽ��ܵģ����������п��ܻᵼ�¶���ʧ�ܡ�
		\since build 295
		*/
		PDefH(bool, Switch, shared_ptr<Shell>&& h) lnothrow
			ImplRet(Switch(h))
	};


	/*!
	\brief ȡӦ�ó���ʵ����
	\throw LoggedEvent �Ҳ���ȫ��Ӧ�ó���ʵ������Ϣ����ʧ�ܡ�
	\note ��֤��ƽ̨��ص�ȫ����Դ��ʼ��֮���ʼ����ʵ����
	\note �̰߳�ȫ��
	*/
	extern LF_API Application&
		FetchAppInstance();


	lconstexpr const Messaging::Priority DefaultQuitPriority(0xF0);


	/*!
	\brief ȫ��Ĭ�϶�����Ϣ���ͺ�����
	\exception LoggedEvent �Ҳ���ȫ��Ӧ�ó���ʵ������Ϣ����ʧ�ܡ�
	\note �̰߳�ȫ��
	*/
	//@{
	LF_API void
		PostMessage(const Message&, Messaging::Priority);
	inline PDefH(void, PostMessage, Messaging::ID id, Messaging::Priority prior,
		const ValueObject& vo = {})
		ImplRet(PostMessage(Message(id, vo), prior))
		inline PDefH(void, PostMessage, Messaging::ID id, Messaging::Priority prior,
			ValueObject&& c)
		ImplRet(PostMessage(Message(id, std::move(c)), prior))
		template<Messaging::MessageID _vID>
	inline void
		PostMessage(Messaging::Priority prior,
			const typename Messaging::SMessageMap<_vID>::TargetType& target)
	{
		PostMessage(_vID, prior, ValueObject(target));
	}
	//@}

	/*!
	\brief ��ָ������������ȼ����� Shell ��ֹ����
	*/
	LF_API void
		PostQuitMessage(int = 0, Messaging::Priority p = DefaultQuitPriority);

	/*!
	\brief GUI ������
	*/
	class LF_API GUIHost
	{
	private:
#if LF_Hosted
		/*!
		\brief �������ڶ���ӳ�䡣
		\note ��ʹ�� ::SetWindowLongPtr �� Windows API ���ֿ�ƽ̨��
		����������������ͻ��
		\warning ���ٴ���ǰ�Ƴ�ӳ�䣬�����������δ������Ϊ��
		\warning ���̰߳�ȫ��Ӧ���������߳��ϲ�����
		*/
		map<Host::NativeWindowHandle, observer_ptr<Host::Window>> wnd_map;
		//! \brief ���ڶ���ӳ������
		mutable mutex wmap_mtx;
		/*!
		\brief �����̼߳�����
		\sa EnterWindowThrad, LeaveWindowThread
		*/
		atomic<size_t> wnd_thrd_count{ 0 };

	public:
		/*!
		\brief �˳���ǡ�
		\sa LeaveWindowThread
		*/
		atomic<bool> ExitOnAllWindowThreadCompleted{ true };

#	if LFL_Win32
		/*!
		\brief ��ӳ�����̡�
		\sa MapCursor
		*/
		std::function<pair<observer_ptr<Host::Window>, Drawing::Point>(
			const Drawing::Point&)> MapPoint{};

	private:
		Host::WindowClass window_class;
#	elif LFL_Android
		/*!
		\brief ��ӳ�����̡�
		\note ���ǿ��� MapCursor ���ô�ʵ�֣�����ʹ�ú�ȱ任��
		*/
		leo::id_func_clr_t<Drawing::Point>* MapPoint = {};
		/*!
		\brief �����������档
		*/
		UI::Desktop Desktop;
#	endif
#endif

	public:
		GUIHost();
		~GUIHost();

#if LF_Hosted
		/*!
		\brief ȡ GUI ǰ�����ڡ�
		\todo �̰߳�ȫ��
		\todo �� Win32 ����ƽ̨ʵ�֡�
		*/
		observer_ptr<Host::Window>
			GetForegroundWindow() const lnothrow;
#endif

#if LF_Hosted
		/*!
		\brief ���봰��ӳ���
		\note �̰߳�ȫ��
		*/
		void
			AddMappedItem(Host::NativeWindowHandle, observer_ptr<Host::Window>);

#	if LF_Multithread == 1
		/*!
		\brief ��ǿ�ʼ�����̣߳����Ӵ����̼߳�����
		\note �̰߳�ȫ��
		\since build 399
		*/
		void
			EnterWindowThread();
#	endif

		/*!
		\brief ȡ���������Ӧ�Ĵ���ָ�롣
		\note �̰߳�ȫ��
		*/
		observer_ptr<Host::Window>
			FindWindow(Host::NativeWindowHandle) const lnothrow;

#	if LF_Multithread == 1
		/*!
		\brief ��ǽ��������̣߳����ٴ����̼߳������ڼ���Ϊ��ʱִ�и��Ӳ�����

		���ٴ��ڼ������飬��Ϊ�����˳���Ǳ�����ʱ�� YSLib ��Ϣ���з����˳���Ϣ��
		*/
		void
			LeaveWindowThread();
#	endif

		/*!
		\brief ӳ���������λ�õ���Զ�����������Ĺ��λ�á�
		\return ʹ�õĶ�������ָ�루��ʹ����Ļ��Ϊ�գ�����Զ������ڻ���Ļ��λ�á�
		\todo ֧�� Win32 �� Android �����ƽ̨��

		����ȷ����Ļ���λ�ã��� MapPoint �ǿ������ MapPoint ȷ���������ڼ��任���꣬
		��󷵻ؽ����
		*/
		pair<observer_ptr<Host::Window>, Drawing::Point>
			MapCursor() const;

#	if LFL_Win32
		/*!
		\brief ӳ�䶥�����ڵĵ㡣

		���ȵ���ʹ��ָ���Ĳ�����Ϊ��Ļ���λ��ȷ���������ڡ�
		������ָ���Ķ������ڣ�����ô��ڵ� MapCursor ����ȷ����������򷵻���Чֵ��
		*/
		pair<observer_ptr<Host::Window>, Drawing::Point>
			MapTopLevelWindowPoint(const Drawing::Point&) const;
#	endif
		/*!
		\brief �Ƴ�����ӳ���
		\note �̰߳�ȫ��
		*/
		void
			RemoveMappedItem(Host::NativeWindowHandle) lnothrow;

		void
			UpdateRenderWindows();
#endif



	};


	/*!
	\brief ƽ̨��ص�Ӧ�ó����ࡣ
	\note ��Ĭ�Ͻӿڡ�
	*/
	class LF_API GUIApplication : public Application
	{
	private:
		struct InitBlock final
		{
			//! \brief ����״̬��
			//! \brief GUI ����״̬��
			mutable GUIHost host{};

			InitBlock(Application&);
		};

		leo::call_once_init<InitBlock, once_flag> init;

	public:
		/*!
		\brief �û�����������Ӧ��ֵ��
		\sa DSApplication::Run

		��������Ϣ���е���Ϣѭ���п��ƺ�̨��Ϣ���ɲ��Ե�ȫ����Ϣ���ȼ���
		*/
		Messaging::Priority UIResponseLimit = 0x40;

		/*!
		\brief �޲������졣
		\pre ���ԣ�Ψһ�ԡ�
		*/
		GUIApplication();
		/*!
		\brief �������ͷ���Դ��
		*/
		~GUIApplication() override;

		GUIHost&
			GetGUIHostRef() const lnothrow;

		/*!
		\brief ����ǰ��Ϣ��
		\return ѭ��������
		\note �̰߳�ȫ��ȫ����Ϣ���л�����ʡ�
		\note ���ȼ�С�� UIResponseLimit ����Ϣʱ��Ϊ��̨��Ϣ������Ϊǰ̨��Ϣ��
		\note ��Ϣ����Ӧ��Ϣ���Ƴ���
		\warning Ӧ��֤���Ƴ����ڱ���Ӧ����Ϣ��

		������Ϣ����Ϊ�գ����������Ϣ�����������Ϣ����ȡ�����ַ���Ϣ��
		��ȡ������Ϣ�ı�ʶΪ SM_Quit ʱ��Ϊ��ֹѭ����
		�Ժ�̨��Ϣ���ַ�ǰ���ú�̨��Ϣ������򣺷ַ�������Ϣ���ɽ���ʱ����ơ�
		*/
		bool
			DealMessage();
	};


	/*!
	\brief ȡȫ��Ӧ�ó���ʵ����
	\throw GeneralEvent �����ڳ�ʼ����ɵ�Ӧ�ó���ʵ����
	\warning �����̰߳�ȫ��������֤���ý�����ʵ�������������ڡ�
	*/
	//@{
	LF_API limpl(GUIApplication&)
		FetchGlobalInstance();

	inline PDefH(GUIHost&, FetchGUIHost, )
		ImplRet(FetchGlobalInstance().GetGUIHostRef())
}
