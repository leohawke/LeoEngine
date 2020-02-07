/*! \file Engine\Render\IRayTracingPipelineState.h
\ingroup Engine
*/
#pragma once
#ifndef LE_RENDER_IRayTracingPipelineState_h
#define LE_RENDER_IRayTracingPipelineState_h 1

namespace platform::Render {
	class RayTracingPipelineState
	{
	public:
		virtual ~RayTracingPipelineState();
	};
}

#endif