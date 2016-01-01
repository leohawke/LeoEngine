#pragma once
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   Core/HUDPseudoWindow.h
//  Version:     v1.01
//  Created:     12/12/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: HUDäÖÈ¾Î±´°¿Ú
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_HUDPseudoWindow_Hpp
#define Core_HUDPseudoWindow_Hpp

#include <ldef.h>
#include <leoint.hpp>
#include <memory>

LEO_BEGIN

namespace HUD {
	class HostRenderer;
}


class HUDPseudoWindow{
public:
	HUDPseudoWindow(HUD::HostRenderer&);

	using size_type = std::pair<uint16, uint16>;

	void Resize(const size_type& size);

	void RePos(const size_type& pos);

	void Update(float t);

	void Render(size_type);

	bool IsMined();

	~HUDPseudoWindow();

private:
	struct HUDPseudoWindowImpl;
	std::unique_ptr<HUDPseudoWindowImpl> pImpl;
};

LEO_END


#endif
