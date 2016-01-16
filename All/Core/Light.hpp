//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Light.hpp
//  Version:     v1.01
//  Created:     05/09/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供光源类和相关函数
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Light_Hpp
#define Core_Light_Hpp

#include <leo2dmath.hpp>
#include <memory.hpp>
#include "LightStruct.hpp"
#include <leomathutility.hpp>

namespace leo {

	class Camera;

	//windows_system
	ops::Rect CalcScissorRect(const PointLight& wPointLight, const Camera& camera);

#ifdef LEO_MEMORY_LIMIT
	class LB_API LightSource :LightAlloc {
#else
	class LB_API LightSource {
#endif
	public:
		enum light_type {
			point_light = 0,

			spot_light,

			directional_light,

			ambient_light,

			type_count
		};

	public:
		explicit LightSource(light_type type);

		virtual ~LightSource() {}

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
		virtual float SinOuterAngle() const = 0;
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
		float SinOuterAngle() const {
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
		const float3& Directional() const {
			return  *reinterpret_cast<const float3*>(&Directional_Outer);
		}
		//Spot
		float CosOuterAngle() const {
			return Directional_Outer.a;
		}
		//Spot
		float SinOuterAngle() const {
			return Outer_Sin;
		}
		//Spot
		float CosInnerAngle() const {
			return Inner;
		}
		//Spot,Directional
		void Directional(const float3& dir) {
			std::copy(dir.begin(), dir.end(), Directional_Outer.begin());
		}
		//Spot
		void OuterAngle(float angle) {
			Outer_Sin = sincosr(&Directional_Outer.a, angle);
		}

		//Spot
		void InnerAngle(float angle) {
			Inner = cos(angle);
		}
	private:
		float3 mFallOff;
		float4 Directional_Outer;
		float Inner;
		float Outer_Sin;
	};

	class LB_API DirectionalLightSource :public LightSource {
	public:
		DirectionalLightSource();

		const float3& Directional() const {
			return mDirectional;
		}

		void Directional(const float3& dir){
			mDirectional = dir;
		}
	protected:
		float CosInnerAngle() const {
			return 0.f;
		}
		void InnerAngle(float angle) {
		}
		float CosOuterAngle() const {
			return 0.f;
		}
		float SinOuterAngle() const {
			return 0.f;
		}
		void OuterAngle(float angle) {
		}

	private:
		float3 mDirectional;
	};

	class LB_API AmbientLightSource :public LightSource {
	public:
		AmbientLightSource();

		const float3& Directional() const {
			return mDirectional;
		}

		void Directional(const float3& dir) {
			mDirectional = dir;
		}
	protected:
		float CosInnerAngle() const {
			return 0.f;
		}
		void InnerAngle(float angle) {
		}
		float CosOuterAngle() const {
			return 0.f;
		}
		float SinOuterAngle() const {
			return 0.f;
		}
		void OuterAngle(float angle) {
		}
	private:
		float3 mDirectional;
	};
}

namespace std {
	//make_shared overload for PointLightSource
	template<>
	inline shared_ptr<leo::PointLightSource> make_shared()
	{	// make a shared_ptr
#ifdef LEO_MEMORY_LIMIT
		shared_ptr<leo::PointLightSource> _Ret;
		_Ret.reset(new leo::PointLightSource());
#else
		auto _Ret = std::allocate_shared <
			leo::PointLightSource, leo::aligned_alloc < leo::PointLightSource, 16 >>
			(leo::aligned_alloc<leo::PointLightSource, 16>());
#endif
		return (_Ret);
	}

	//make_shared overload for SpotLightSource
	template<>
	inline shared_ptr<leo::SpotLightSource> make_shared()
	{	// make a shared_ptr
#ifdef LEO_MEMORY_LIMIT
		shared_ptr<leo::SpotLightSource> _Ret;
		_Ret.reset(new leo::SpotLightSource());
#else
		auto _Ret = std::allocate_shared <
			leo::SpotLightSource, leo::aligned_alloc < leo::SpotLightSource, 16 >>
			(leo::aligned_alloc<leo::SpotLightSource, 16>());
#endif
		return (_Ret);
	}

	//make_shared overload for DirectionalLightSource
	template<>
	inline shared_ptr<leo::DirectionalLightSource> make_shared()
	{	// make a shared_ptr
#ifdef LEO_MEMORY_LIMIT
		shared_ptr<leo::DirectionalLightSource> _Ret;
		_Ret.reset(new leo::DirectionalLightSource());
#else
		auto _Ret = std::allocate_shared <
			leo::DirectionalLightSource, leo::aligned_alloc < leo::DirectionalLightSource, 16 >>
			(leo::aligned_alloc<leo::DirectionalLightSource, 16>());
#endif
		return (_Ret);
	}

	//make_shared overload for AmbientLightSource
	template<>
	inline shared_ptr<leo::AmbientLightSource> make_shared()
	{	// make a shared_ptr
#ifdef LEO_MEMORY_LIMIT
		shared_ptr<leo::DirectionalLightSource> _Ret;
		_Ret.reset(new leo::DirectionalLightSource());
#else
		auto _Ret = std::allocate_shared <
			leo::AmbientLightSource, leo::aligned_alloc < leo::DirectionalLightSource, 16 >>
			(leo::aligned_alloc<leo::AmbientLightSource, 16>());
#endif
		return (_Ret);
	}
}


#endif