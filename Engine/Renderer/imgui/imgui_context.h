#pragma once

#include "../../Render/IContext.h"
#include <imgui/imgui.h>

namespace platform::imgui
{
	// cmd_list is the command list that the implementation will use to render imgui draw lists.
	// Before calling the render function, caller must prepare cmd_list by resetting it and setting the appropriate
	// render target and descriptor heap that contains font_srv_cpu_desc_handle/font_srv_gpu_desc_handle.
	// font_srv_cpu_desc_handle and font_srv_gpu_desc_handle are handles to a single SRV descriptor to use for the internal font texture.
	bool     Context_Init(Render::Context& context);
	void     Context_Shutdown();
	void     Context_NewFrame();
	void     Context_RenderDrawData(ImDrawData* draw_data);
}