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
#include <memory.hpp>
namespace leo {
	struct PointLight;
	struct DirectionalLight;
	struct SpotLight;

	struct PointLight {
		float3 Position;
		float3 Diffuse;
		float4 FallOff_Range;
	};

	struct DirectionalLight {
		float3 Directional;
		float3 Diffuse;
	};

	struct SpotLight {
		float4 Position_Inner;
		float3 Diffuse;
		float4 FallOff_Range;
		float4 Directional_Outer;
	};

	class Camera;

	//windows_system
	ops::Rect CalcScissorRect(const PointLight& wPointLight, const Camera& camera);

	class LB_API LightSource :public LightAlloc {
	public:
		enum light_type {
			point_light = 0,

			spot_light,

			type_count
		};

	public:
		explicit LightSource(light_type type);

		virtual ~LightSource(){}
		
		light_type Type() const;

		const float3 & Position() const;
		float Range() const;

		const float3& Diffuse() const;

		void Position(const float3& pos);
		void Range(float range);

		void Diffuse(const float3& diffuse);

		//Spot,Directional
		virtual const float3& Directional() const = 0;

		//Spot,Directional
		virtual void Directional(const float3& dir) = 0;
		
		//Spot
		virtual float CosInnerAngle() const = 0;
		//Spot
		virtual void InnerAngle(float angle) = 0;
		//Spot
		virtual float CosOuterAngle() const = 0;
		//Spot
		virtual void OuterAngle(float angle) = 0;

	private:
		float3 mPos;
		float mRange;
		float3 mDiffuse;

		light_type _type;
	};

	class LB_API PointLightSource : public LightSource {
	public:
		PointLightSource();

		const float3& FallOff() const;

		void FallOff(const float3& falloff);

		
	protected:
		//Spot,Directional
		const float3& Directional() const {
			DebugPrintf("Error: Call Error Function");
			return mFallOff;
		}
	
		//Spot,Directional
		void Directional(const float3& dir) {
		}
		
		//Spot
		float CosInnerAngle() const {
			return 0;
		}
		//Spot
		void InnerAngle(float angle) {

		}
		//Spot
		float CosOuterAngle() const {
			return 0;
		}
		//Spot
		void OuterAngle(float angle) {

		}
	private:
		float3 mFallOff;
	};

	class LB_API SpotLightSource :public LightSource {
	public:
		SpotLightSource();

		const float3& FallOff() const;

		void FallOff(const float3& falloff);

		//Spot,Directional
		const float3& Directional() const{
			return  *reinterpret_cast<const float3*>(&Directional_Outer);
		}
		//Spot
		float CosOuterAngle() const {
			return Directional_Outer.a;
		}
		//Spot
		float CosInnerAngle() const{
			return Inner;
		}
		//Spot,Directional
		void Directional(const float3& dir) {
			std::copy(dir.begin(), dir.end(), Directional_Outer.begin());
		}
		//Spot
		void OuterAngle(float angle) {
			Directional_Outer.a = cos(angle);
		}

		//Spot
		void InnerAngle(float angle) {
			Inner = cos(angle);
		}
	private:
		float3 mFallOff;
		float4 Directional_Outer;
		float Inner;
	};

	class LB_API LightSourcesRender {
	public:
		LightSourcesRender(ID3D11Device* device);

		~LightSourcesRender();

		void Draw(ID3D11DeviceContext* context,DeferredRender& pRender, const Camera& camera);

		void AddLight(std::shared_ptr<LightSource> light_source);

	private:
		void Apply(ID3D11DeviceContext* context);


		std::list<std::shared_ptr<LightSource>> mLightSourceList;

	
	};
}


#endif