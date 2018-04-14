/*!	\file Timer.h
\ingroup LFrameWork/LCLib
\brief ƽ̨��صļ�ʱ���ӿڡ�
*/

#ifndef LFrameWork_LCLib_Timer_h
#define LFrameWork_LCLib_Timer_h 1

#include <LFramework/LCLib/FCommon.h>

namespace platform {
	/*!
	\brief ȡ tick ����
	\note ��λΪ���롣
	\warning �״ε��� StartTicks ǰ���̰߳�ȫ��
	*/
	LF_API std::uint32_t
		GetTicks() lnothrow;

	/*!
	\brief ȡ�߾��� tick ����
	\note ��λΪ���롣
	\warning �״ε��� StartTicks ǰ���̰߳�ȫ��
	*/
	LF_API std::uint64_t
		GetHighResolutionTicks() lnothrow;
}

#endif