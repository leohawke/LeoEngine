/*! \file Engine\Render\IRayTracingGeometry.h
\ingroup Engine
\brief ���߼�����Ϣ�ӿ��ࡣ
*/
#ifndef LE_RENDER_IRayRayTracingGeometry_h
#define LE_RENDER_IRayRayTracingGeometry_h 1

namespace platform::Render {
	/** Bottom level ray tracing acceleration structure (contains triangles). */
	class RayTracingGeometry
	{
	public:
		virtual ~RayTracingGeometry();
	};
}

#endif