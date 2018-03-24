/*! \file Engine\Render\IDisplay.h
\ingroup Engine
\brief ��ʾ�߼����,ƽ̨UI����Žӡ�
*/

#ifndef LE_RENDER_IDisplay_h
#define LE_RENDER_IDisplay_h 1

#include <LBase/sutility.h>

namespace platform::Render {

	/* \warning ��������
	*/
	class Display :private limpl(leo::noncopyable), private limpl(leo::nonmovable) {
	public:
		virtual void SwapBuffers() = 0;
		virtual void WaitOnSwapBuffers() = 0;
	};
}

#endif