/*! \file Core\WhiteEngine.h
\ingroup WhiteEngine
\brief 3DÒýÇæ¡£
*/
#ifndef LE_Core_White_Engine_H
#define LE_Core_White_Engine_H 1

#include "GraphicsEngine.h"

namespace LeoEngine::Core {
	class WhiteEngine final: public IWhiteEngine{
	public:
		Camera* GetGraphicsPassCamera(const Camera& camera) override;
	};
}


#endif