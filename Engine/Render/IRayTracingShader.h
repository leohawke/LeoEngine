/*! \file Engine\Render\IRayTracingShader.h
\ingroup Engine
\brief RayShader¡£
*/
#pragma once
#ifndef LE_RENDER_IRayTracingShader_h
#define LE_RENDER_IRayTracingShader_h 1

namespace platform::Render {
	class RayTracingShader
	{
	public:
		virtual ~RayTracingShader();
	};

}

#endif