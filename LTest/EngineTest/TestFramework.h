#pragma once

#include "LBase/sutility.h"
#include "LBase/linttype.hpp"
#ifdef LB_IMPL_MSCPP
#include <string_view>
#else
#include <experimental/string_view> //TODO:replace it
#endif
#include <LFramework/LCLib/NativeAPI.h>
#include <LFramework/Helper/WindowThread.h>

namespace Test {
	using namespace leo;


	//�򻯲��Դ����ܵĻ���,������:
	//��ϷӦ���������������߼�ģ�����ӹܳ�����Ҳ����Ϊ���������߼�ģ��ĳ�������ӿڡ�

	//Create 
	//Run

	//OnCreate
	//OnDestroy()
	//DoUpdate()

	class TestFrameWork :leo::noncopyable {
	public:
		//���º���������״̬������Ҫ��ɵ����� ���ǲ�����Ĭ�ϲ�дReturn��
		enum UpdateRet {
			Nothing = 0,
		};

	public:
		explicit TestFrameWork(const std::wstring_view &name);

		virtual ~TestFrameWork();

		void Create();
		void Run();

		platform_ex::NativeWindowHandle GetNativeHandle();
	protected:
		leo::uint32 Update(leo::uint32 pass);

	private:
		virtual void OnCreate();
		virtual void OnDestroy();
		virtual leo::uint32 DoUpdate(leo::uint32 pass) = 0;

#if LFL_Win32
		unique_ptr<Host::WindowThread> p_wnd_thrd;
#endif

	};
}