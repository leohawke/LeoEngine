////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   HUD/HUDHostRenderer.h
//  Version:     v1.00
//  Created:     12/12/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: HUD宿主渲染器，最后绘制至HUD窗口
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef HUD_HostRenderer_H
#define HUD_HostRenderer_H

#include "HUDRenderer.h"
#include "RenderSystem/TextureX.hpp"
#include <Core\HUDPseudoWindow.h>

LEO_BEGIN

HUD_BEGIN

class LB_API HostRenderer : public BufferedRenderer
{
private:
	lref<IWidget> widget;
	std::unique_ptr<HUDPseudoWindow> window;
public:
	template<typename... Args>
	HostRenderer(IWidget& wgt,Args&&... args)
		:BufferedRenderer(false, X::MakeIImage(GetSizeOf(wgt))),widget(wgt),window(std::make_unique<HUDPseudoWindow>(*this,lforward(args)...)){
		InitWidgetView();
	}

	~HostRenderer();

	void
		SetSize(const Size&) override;

	void
		Render();
private:
	void
		InitWidgetView();
public:
	PDefH(HostRenderer*, clone, ) const override
		ImplThrow(unimplemented("HostRenderer::clone unimplemented."))

};


HUD_END

LEO_END

#endif
