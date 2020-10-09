/*! \file Engine\Render\Color_T.hpp
\ingroup Engine
\brief 中间颜色格式。
*/

#ifndef LE_RENDER_ColorT_hpp
#define LE_RENDER_ColorT_hpp 1

#include <LBase/lmath.hpp>

#include "../emacro.h"
#include "IFormat.hpp"

namespace platform {
	namespace M {
		using namespace leo::inttype;

		inline float linear_to_srgb(float linear) lnoexcept
		{
			if (linear < 0.0031308f)
			{
				return 12.92f * linear;
			}
			else
			{
				float const ALPHA = 0.055f;
				return (1 + ALPHA) * pow(linear, 1 / 2.4f) - ALPHA;
			}
		}

		inline float srgb_to_linear(float srgb) lnoexcept
		{
			if (srgb < 0.04045f)
			{
				return srgb / 12.92f;
			}
			else
			{
				float const ALPHA = 0.055f;
				return pow((srgb + ALPHA) / (1 + ALPHA), 2.4f);
			}
		}

		template <typename T>
		class Color_T;

		// RGBA，用4个浮点数表示r, g, b, a
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		class Color_T
		{
		public:
			typedef T value_type;

			typedef T* pointer;
			typedef const T* const_pointer;

			typedef T& reference;
			typedef const T& const_reference;

			typedef pointer iterator;
			typedef const_pointer const_iterator;

		public:
			Color_T() lnoexcept
			{
			}
			explicit Color_T(T const * rhs) lnoexcept;
			Color_T(Color_T const & rhs) lnoexcept;
			Color_T(Color_T&& rhs) lnoexcept;
			Color_T(T r, T g, T b, T a) lnoexcept;
			explicit Color_T(uint32 dw) lnoexcept;

			// 取颜色
			iterator begin() lnoexcept
			{
				return col_.begin();
			}
			const_iterator begin() const lnoexcept
			{
				return col_.begin();
			}
			iterator end() lnoexcept
			{
				return col_.end();
			}
			const_iterator end() const lnoexcept
			{
				return col_.end();
			}
			reference operator[](size_t index) lnoexcept
			{
				return col_[index];
			}
			const_reference operator[](size_t index) const lnoexcept
			{
				return col_[index];
			}

			reference r() lnoexcept
			{
				return col_[0];
			}
			const_reference r() const lnoexcept
			{
				return col_[0];
			}
			reference g() lnoexcept
			{
				return col_[1];
			}
			const_reference g() const lnoexcept
			{
				return col_[1];
			}
			reference b() lnoexcept
			{
				return col_[2];
			}
			const_reference b() const lnoexcept
			{
				return col_[2];
			}
			reference a() lnoexcept
			{
				return col_[3];
			}
			const_reference a() const lnoexcept
			{
				return col_[3];
			}

			void RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const lnoexcept;

			uint32 ARGB() const lnoexcept;
			uint32 ABGR() const lnoexcept;

			// 赋值操作符
			Color_T& operator+=(Color_T<T> const & rhs) lnoexcept;
			Color_T& operator-=(Color_T<T> const & rhs) lnoexcept;
			Color_T& operator*=(T rhs) lnoexcept;
			Color_T& operator*=(Color_T<T> const & rhs) lnoexcept;
			Color_T& operator/=(T rhs) lnoexcept;

			Color_T& operator=(Color_T const & rhs) lnoexcept;
			Color_T& operator=(Color_T&& rhs) lnoexcept;

			// 一元操作符
			Color_T const operator+() const lnoexcept;
			Color_T const operator-() const lnoexcept;

			bool operator==(Color_T<T> const & rhs) const lnoexcept;

			friend Color_T<float> lerp(const Color_T<float>& lhs, const Color_T<float>& rhs,float w);
			friend float dot(const Color_T<float>& lhs, const Color_T<float>& rhs);
		private:
			leo::math::float4 col_;
		};

		using Color = Color_T<float>;

		template <typename T>
		Color_T<T> modulate(Color_T<T> const & lhs, Color_T<T> const & rhs) lnoexcept
		{
			return Color_T<T>(lhs.r() * rhs.r(), lhs.g() * rhs.g(), lhs.b() * rhs.b(), lhs.a() * rhs.a());
		}

		inline Color_T<float>  lerp(const Color_T<float> & lhs, const Color_T<float> & rhs,float w) {
			auto r = lerp(lhs.col_, rhs.col_, w);
			return Color(r.data);
		}

		inline float dot(const Color_T<float>& lhs, const Color_T<float>& rhs) {
			return dot(lhs.col_, rhs.col_);
		}

		void ConvertFromABGR32F(Render::EFormat fmt, Color const * input, uint32 num_elems, void* output);
		void ConvertToABGR32F(Render::EFormat fmt, void const * input, uint32_t num_elems, Color* output);
	}

	template<Render::EFormat format>
	struct TColor;

	template<>
	struct TColor<Render::EF_ARGB8_SRGB>
	{
		using uint8 = leo::uint8;

		union { struct { uint8 B, G, R, A; }; leo::uint32 AlignmentDummy; };

		TColor():AlignmentDummy(0){}

		constexpr TColor(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255)
			: B(InB), G(InG), R(InR), A(InA)
		{}

		leo::uint32& DWColor() { return *((leo::uint32*)this); }
		const leo::uint32& DWColor() const { return *((leo::uint32*)this); }
	};

	//	Stores a color with 8 bits of precision per channel.  
	using FColor = TColor<Render::EF_ARGB8_SRGB>;
}

#endif