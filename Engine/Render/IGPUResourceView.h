/*! \file Engine\Render\IFormat.hpp
\ingroup Engine
\brief GPU×ÊÔ´View¡£
*/
#ifndef LE_RENDER_IGPUResourceView_h
#define LE_RENDER_IGPUResourceView_h 1

#include "ITexture.hpp"

namespace platform {
	namespace Render {
		class FrameBuffer;

		class ShaderResourceView{
		public:
			virtual ~ShaderResourceView();
		};

		class UnorderedAccessView :public RObject
		{
		public:
			virtual ~UnorderedAccessView();
		};
	}
}

#endif