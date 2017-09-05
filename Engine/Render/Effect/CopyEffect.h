/*! \file Engine\Render\Effect\BiltEffect.h
\ingroup Engine\Render\Effect
\brief �����ڲ�ͬ��С��ͼ֮�临��(��:����Mipmap)��
*/
#ifndef LE_RENDER_EFFECT_COPY_h
#define LE_RENDER_EFFECT_COPY_h 1

#include "EffectProperty.h"

namespace platform::Render::Effect {

	class CopyEffect : public Effect {
	public:
		CopyEffect(const std::string& name);

		leo::lref<Technique> BilinearCopy;
	};
}

#endif