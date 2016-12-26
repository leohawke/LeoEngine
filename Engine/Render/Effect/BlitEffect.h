/*! \file Engine\Render\Effect\BlitEffect.h
\ingroup Engine\Render\Effect
\brief 实现GDI式的Blit效果。
*/
#ifndef LE_RENDER_EFFECT_BLIT_h
#define LE_RENDER_EFFECT_BLIT_h 1

#include "Effect.hpp"

namespace platform::Render::Effect {

	class BlitEffect : public Effect {
	public:
		Technique BilinearCopy;
	};
}

#endif