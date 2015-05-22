//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Light.hpp
//  Version:     v1.00
//  Created:     05/09/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供光照结构体和相关函数
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Light_Hpp
#define Core_Light_Hpp

#include <leo2dmath.hpp>
#include <RenderSystem\DeferredRender.hpp>
namespace leo {
	struct PointLight;
	struct DirectionalLight;
	struct SpotLight;

	struct PointLight {
		float4 PositionRange;
		float3 Diffuse;
	};

	struct DirectionalLight {
		float3 Directional;
		float3 Diffuse;
	};

	struct SpotLight : public PointLight {
		float4 DirectionalRadius;
	};

	class Camera;

	//windows_system
	ops::Rect CalcScissorRect(const PointLight& wPointLight, const Camera& camera);

	class LB_API LightSource {
	public:
		enum light_type {
			point_light = 0,

			type_count
		};

	public:
		explicit LightSource(light_type type);
		
		light_type Type() const;

		const float3 & Position() const;
		float Range() const;

		const float3& Diffuse() const;

		void Position(const float3& pos);
		void Range(float range);

		void Diffuse(const float3& diffuse);

	private:
		float3 mPos;
		float mRange;
		float3 mDiffuse;

		light_type _type;
	};

	class LB_API PointLightSource : public LightSource {
	public:
		PointLightSource();
	};

	class LB_API LightRender {
	public:
		void Draw(ID3D11DeviceContext* context,const DeferredRender& pRender,const LightSource& light_source, const Camera& camera);
	};
}


#endif