/*! \file Core\LeoEngine.h
\ingroup LeoEngine
\brief Í¼ÐÎÒýÇæ¡£
*/
#ifndef LE_Core_Leo_Engine_H
#define LE_Core_Leo_Engine_H 1


#include "GraphicsEngine.h"

namespace LeoEngine::GraphicsEngine {
	class  LeoEngine final : public ILeoEngine {
		GraphicsView* GetOrCreateRenderView(IGraphicsView::ViewType type) override;
	};
}

#endif
