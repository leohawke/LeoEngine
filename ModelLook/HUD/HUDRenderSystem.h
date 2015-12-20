#ifndef HUD_RenderSystem_H
#define HUD_RenderSystem_H

#include "HUDHostRenderer.h"
#include <Singleton.hpp>
#include <leomathtype.hpp>

namespace leo {
	HUD_BEGIN
	class HUDRenderSystem : public Singleton<HUDRenderSystem> {
	public:

		struct hud_mat {
			hud_mat();

			hud_mat(TexturePtr p);

			Drawing::Color color;
			TexturePtr ptr;
		};

		struct hud_command {
			uint32 vertex_start = 0;
			uint16 index_start = 0;
			uint16 index_num = 0;
			hud_mat mat;
		};
		
		struct vb_data {
			uint32 vertex_start = 0;
			uint32 vertex_num = 0;
			float2* vertex;
		};

		struct ib_data {
			uint16 index_start = 0;
			uint16 index_num = 0;
			uint16* index;
		};

		virtual ~HUDRenderSystem();

		//thread safe
		virtual void PushRenderCommand(hud_command) = 0;

		//thread safe
		virtual vb_data LockVB(uint32) = 0;
		virtual void UnLockVB(const vb_data&) = 0;
		virtual ib_data LockIB(uint16) = 0;
		virtual void UnLockIB(const ib_data&) = 0;
		virtual Texture::Mapper Map2D(Size) = 0;

		virtual void ExceuteCommand() = 0;

		static HUDRenderSystem& GetInstance();
	};
	HUD_END
}

#endif
