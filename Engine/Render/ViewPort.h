/*! \file Render\ViewPort.h
\ingroup Engine
\brief 视口结构。
*/
#ifndef LE_Render_ViewPort_H
#define LE_Render_ViewPort_H 1

#include <LBase/sutility.h>
#include <LBase/linttype.hpp>

namespace LeoEngine::Render {
	struct ViewPort
	{
		ViewPort() = default;

		int x;
		int y;
		leo::uint16 width;
		leo::uint16 height;

		float zmin = 0;
		float zmax = 1;

		ViewPort(int _x, int _y, leo::uint16 _width, leo::uint16 _height)
			:x(_x), y(_y),
			width(_width),
			height(_height),
			zmin(0),
			zmax(1)
		{
		}
	};
}

#endif