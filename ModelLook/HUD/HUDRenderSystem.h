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
			uint16 vertex_start;
			uint16 index_start;
			uint16 index_num;
			hud_mat mat;
		};
		
		struct vb_data {
			uint16 vertex_start;
			uint16 vertex_num ;
			float4* vertex;
		};

		struct ib_data {
			uint16 index_start;
			uint16 index_num;
			uint16* index;
		};

		virtual ~HUDRenderSystem();

		virtual void PushRenderCommand(hud_command) = 0;

		virtual vb_data LockVB(uint16) = 0;
		virtual void UnLockVB(const vb_data&) = 0;
		virtual ib_data LockIB(uint16) = 0;
		virtual void UnLockIB(const ib_data&) = 0;
		virtual std::unique_ptr<Texture::Mapper> Map2D(Size) = 0;

		virtual void ExceuteCommand(std::pair<uint16,uint16> windows_size) = 0;

		static HUDRenderSystem& GetInstance();


		//0-----1
		//3-----2
		static void FillQuadIBByVB(const vb_data& vb_info,ib_data& ib_info)
		{
			assert(ib_info.index_num >= 6);
			assert(vb_info.vertex_num >= 4);

			ib_info.index[ib_info.index_start] = vb_info.vertex_start;
			ib_info.index[ib_info.index_start + 1] = vb_info.vertex_start + 1;
			ib_info.index[ib_info.index_start + 2] = vb_info.vertex_start + 3;

			ib_info.index[ib_info.index_start+3] = vb_info.vertex_start +3;
			ib_info.index[ib_info.index_start + 4] = vb_info.vertex_start +1;
			ib_info.index[ib_info.index_start + 5] = vb_info.vertex_start +2;
		}

		static hud_command MakeCommand(const vb_data& vb_info, ib_data& ib_info, const hud_mat& mat= {})
		{
			return{ vb_info.vertex_start,ib_info.index_start,ib_info.index_num,mat };
		}
	};
	HUD_END
}

#endif
