/*! \file Engine\Render\Effect\EffectProperty.h
\ingroup Engine\Render\Effect
\brief 实现C#式的属性访问。
\todo 抽象成更通用的系统
*/
#ifndef LE_RENDER_EFFECT_PROPERTY_h
#define LE_RENDER_EFFECT_PROPERTY_h 1

#include "Effect.hpp"
#include <LBase/ref.hpp>
#include <LBase/lmathtype.hpp>

namespace platform::Render::Effect {
	template<typename T>
	class scalarproperty {
	public:
		template<typename F>
		scalarproperty(leo::lref<T> v,F&& s,F && g)
			:ref(v),getter(g),setter(s)
		{}

		scalarproperty(leo::lref<T> v)
			:ref(v), getter([] {}), setter([] {})
		{}

		scalarproperty& operator=(T v) {
			ref = v;
			setter();
		}

		operator T()  const lnothrow {
			getter();
			return ref;
		}
	private:
		leo::lref<T> ref;
		std::function<void()> setter;
		std::function<void()> getter;
	};

	using floatproperty = scalarproperty<float>;
	using intproperty = scalarproperty<int>;

	class float3property {
	public:
		using float3 = leo::math::float3;
		template<typename F>
		float3property(leo::lref<float3> v, F&& s, F && g)
			:ref(v), getter(g), setter(s),
			x(v.get().x, s, g), y(v.get().y, s, g), z(v.get().z, s, g),
			r(v.get().x, s, g), g(v.get().y, s, g), b(v.get().z, s, g),
			u(v.get().x, s, g), v(v.get().y, s, g), w(v.get().z, s, g)
		{}

		float3property(leo::lref<float3> v)
			:ref(v), getter([] {}), setter([] {}),
			x(v.get().x.operator float &()), y(v.get().y.operator float &()), z(v.get().z.operator float &()),
			r(v.get().x.operator float &()), g(v.get().y.operator float &()), b(v.get().z.operator float &()),
			u(v.get().x.operator float &()), v(v.get().y.operator float &()), w(v.get().z.operator float &())
		{}

		float3property& operator=(float3 v) {
			ref = v;
			setter();
		}

		operator float3()  const lnothrow {
			getter();
			return ref;
		}

		floatproperty x;
		floatproperty y;
		floatproperty z;

		floatproperty r;
		floatproperty g;
		floatproperty b;

		floatproperty u;
		floatproperty v;
		floatproperty w;
	private:
		leo::lref<float3> ref;
		std::function<void()> setter;
		std::function<void()> getter;
	};

	class float4property {
	public:
		using float4 = leo::math::float4;
		template<typename F>
		float4property(leo::lref<float4> v, F&& s, F && g)
			:ref(v), getter(g), setter(s), 
			x(v.get().x,s,g),y(v.get().y, s, g), z(v.get().z, s, g), w(v.get().w, s, g),
			r(v.get().x, s, g), g(v.get().y, s, g), b(v.get().z, s, g), a(v.get().w, s, g)
		{}

		float4property(leo::lref<float4> v)
			:ref(v), getter([] {}), setter([] {}),
			x(v.get().x.operator float &()), y(v.get().y.operator float &()), z(v.get().z.operator float &()), w(v.get().w.operator float &()),
			r(v.get().x.operator float &()), g(v.get().y.operator float &()), b(v.get().z.operator float &()), a(v.get().w.operator float &())
		{}

		float4property& operator=(float4 v) {
			ref = v;
			setter();
		}

		operator float4()  const lnothrow {
			getter();
			return ref;
		}

		floatproperty x;
		floatproperty y;
		floatproperty z;
		floatproperty w;

		floatproperty r;
		floatproperty g;
		floatproperty b;
		floatproperty a;
	private:
		leo::lref<float4> ref;
		std::function<void()> setter;
		std::function<void()> getter;
	};
}


#endif