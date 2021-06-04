/*! \file Engine\Render\Color_T.hpp
\ingroup Engine
*/

#ifndef LE_RENDER_ColorT_hpp
#define LE_RENDER_ColorT_hpp 1

#include "../emacro.h"
#include "CoreTypes.h"

#include <utility>

namespace LeoEngine
{
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

	namespace details
	{
		constexpr float f = 1 / 255.f;

		template<typename type_struct, bool anti = false>
		struct vector_4d_impl {
			using component_type = typename type_struct::component_type;
			using vector3d_type = typename type_struct::vector3d_type;
			using vector4d_type = typename type_struct::vector4d_type;
			using scalar_type = component_type;
			using vec_type = vector_4d_impl;

			union {
				lm::type_details::component<type_struct, 0> r;
				lm::type_details::component<type_struct, 1> g;
				lm::type_details::component<type_struct, 2> b;
				lm::type_details::component<type_struct, 3> a;

				lm::type_details::sub_vector2d<type_struct, anti, 0, 1> rg;
				lm::type_details::sub_vector2d<type_struct, anti, 0, 2> rb;
				lm::type_details::sub_vector2d<type_struct, anti, 0, 3> ra;
				lm::type_details::sub_vector2d<type_struct, anti, 1, 0> gr;
				lm::type_details::sub_vector2d<type_struct, anti, 1, 2> gb;
				lm::type_details::sub_vector2d<type_struct, anti, 1, 3> ga;
				lm::type_details::sub_vector2d<type_struct, anti, 2, 0> br;
				lm::type_details::sub_vector2d<type_struct, anti, 2, 1> bg;
				lm::type_details::sub_vector2d<type_struct, anti, 2, 3> ba;
				lm::type_details::sub_vector2d<type_struct, anti, 3, 0> ar;
				lm::type_details::sub_vector2d<type_struct, anti, 3, 1> ag;
				lm::type_details::sub_vector2d<type_struct, anti, 3, 2> ab;

				lm::type_details::sub_vector3d<type_struct, anti, 0, 1, 2> rgb;
				lm::type_details::sub_vector3d<type_struct, anti, 0, 2, 1> rbg;
				lm::type_details::sub_vector3d<type_struct, anti, 0, 1, 3> rga;
				lm::type_details::sub_vector3d<type_struct, anti, 0, 3, 1> rag;
				lm::type_details::sub_vector3d<type_struct, anti, 0, 2, 3> rba;
				lm::type_details::sub_vector3d<type_struct, anti, 0, 3, 2> rab;
				lm::type_details::sub_vector3d<type_struct, anti, 1, 0, 2> grb;
				lm::type_details::sub_vector3d<type_struct, anti, 1, 2, 0> gbr;
				lm::type_details::sub_vector3d<type_struct, anti, 1, 0, 3> gra;
				lm::type_details::sub_vector3d<type_struct, anti, 1, 3, 0> gar;
				lm::type_details::sub_vector3d<type_struct, anti, 1, 2, 3> gba;
				lm::type_details::sub_vector3d<type_struct, anti, 1, 3, 2> gab;
				lm::type_details::sub_vector3d<type_struct, anti, 2, 0, 1> brg;
				lm::type_details::sub_vector3d<type_struct, anti, 2, 1, 0> bgr;
				lm::type_details::sub_vector3d<type_struct, anti, 2, 0, 3> bra;
				lm::type_details::sub_vector3d<type_struct, anti, 2, 3, 0> bar;
				lm::type_details::sub_vector3d<type_struct, anti, 2, 1, 3> bga;
				lm::type_details::sub_vector3d<type_struct, anti, 2, 3, 1> bag;
				lm::type_details::sub_vector3d<type_struct, anti, 3, 0, 1> arg;
				lm::type_details::sub_vector3d<type_struct, anti, 3, 1, 0> agr;
				lm::type_details::sub_vector3d<type_struct, anti, 3, 0, 2> arb;
				lm::type_details::sub_vector3d<type_struct, anti, 3, 2, 0> abr;
				lm::type_details::sub_vector3d<type_struct, anti, 3, 1, 2> agb;
				lm::type_details::sub_vector3d<type_struct, anti, 3, 2, 1> abg;

				lm::type_details::sub_vector4d<type_struct, anti, 0, 1, 2, 3>  rgba;
				lm::type_details::sub_vector4d<type_struct, anti, 0, 1, 3, 2>  rgab;
				lm::type_details::sub_vector4d<type_struct, anti, 0, 2, 1, 3>  rbga;
				lm::type_details::sub_vector4d<type_struct, anti, 0, 2, 3, 1>  rbag;
				lm::type_details::sub_vector4d<type_struct, anti, 0, 3, 1, 2>  ragb;
				lm::type_details::sub_vector4d<type_struct, anti, 0, 3, 2, 1>  rabg;
				lm::type_details::sub_vector4d<type_struct, anti, 1, 0, 2, 3>  grba;
				lm::type_details::sub_vector4d<type_struct, anti, 1, 0, 3, 2>  grab;
				lm::type_details::sub_vector4d<type_struct, anti, 1, 2, 0, 3>  gbra;
				lm::type_details::sub_vector4d<type_struct, anti, 1, 2, 3, 0>  gbar;
				lm::type_details::sub_vector4d<type_struct, anti, 1, 3, 0, 2>  garb;
				lm::type_details::sub_vector4d<type_struct, anti, 1, 3, 2, 0>  gabr;
				lm::type_details::sub_vector4d<type_struct, anti, 2, 0, 1, 3>  brga;
				lm::type_details::sub_vector4d<type_struct, anti, 2, 0, 3, 1>  brag;
				lm::type_details::sub_vector4d<type_struct, anti, 2, 1, 0, 3>  bgra;
				lm::type_details::sub_vector4d<type_struct, anti, 2, 1, 3, 0>  bgar;
				lm::type_details::sub_vector4d<type_struct, anti, 2, 3, 0, 1>  barg;
				lm::type_details::sub_vector4d<type_struct, anti, 2, 3, 1, 0>  bagr;
				lm::type_details::sub_vector4d<type_struct, anti, 3, 0, 1, 2>  argb;
				lm::type_details::sub_vector4d<type_struct, anti, 3, 0, 2, 1>  arbg;
				lm::type_details::sub_vector4d<type_struct, anti, 3, 1, 0, 2>  agrb;
				lm::type_details::sub_vector4d<type_struct, anti, 3, 1, 2, 0>  agbr;
				lm::type_details::sub_vector4d<type_struct, anti, 3, 2, 0, 1>  abrg;
				lm::type_details::sub_vector4d<type_struct, anti, 3, 2, 1, 0>  abgr;

				scalar_type data[4];
			};

			constexpr vector_4d_impl(scalar_type x, scalar_type y, scalar_type z, scalar_type w) noexcept
				:data{ x,y ,z ,w }
			{
			}

			constexpr vector_4d_impl(vector4d_type xyzw) noexcept
				:data{ xyzw.x,xyzw.y ,xyzw.z ,xyzw.w }
			{
			}

			constexpr vector_4d_impl(vector3d_type xyz, scalar_type w) noexcept
				:data{ xyz.x,xyz.y,xyz.z,w }
			{
			}

			constexpr explicit vector_4d_impl(uint32 dw) noexcept
				:data{ 
				 f* (static_cast<float>(static_cast<uint8>(dw >> 16))),
				 f* (static_cast<float>(static_cast<uint8>(dw >> 8))),
				 f* (static_cast<float>(static_cast<uint8>(dw >> 0))),
				 f* (static_cast<float>(static_cast<uint8>(dw >> 24)))
				}
			{}

			constexpr vector_4d_impl() noexcept = default;

			constexpr size_t size() const {
				return 4;
			}

			const scalar_type& operator[](std::size_t index) const noexcept {
				lassume(index < size());
				return data[index];
			}

			scalar_type& operator[](std::size_t index) noexcept {
				lassume(index < size());
				return data[index];
			}

			const constexpr scalar_type* begin() const noexcept {
				return data + 0;
			}

			const constexpr scalar_type* end() const noexcept {
				return data + size();
			}

			scalar_type* begin() noexcept {
				return data + 0;
			}

			scalar_type* end() noexcept {
				return data + size();
			}

			constexpr uint32 ARGB() const lnoexcept
			{
				auto R = static_cast<uint8>(leo::math::saturate(r) * 255 + 0.5f);
				auto G = static_cast<uint8>(leo::math::saturate(g) * 255 + 0.5f);
				auto B = static_cast<uint8>(leo::math::saturate(b) * 255 + 0.5f);
				auto A = static_cast<uint8>(leo::math::saturate(a) * 255 + 0.5f);

				return (A << 24) | (R << 16) | (G << 8) | (B << 0);
			}

			friend auto constexpr operator<=>(const vector_4d_impl& lhs, const vector_4d_impl& rhs) noexcept {
				return std::lexicographical_compare_three_way(lhs.begin(),lhs.end(),rhs.begin(),rhs.end());
			}

			friend bool operator==(const vector_4d_impl& lhs, const vector_4d_impl& rhs) noexcept {
				return lhs.ARGB() == rhs.ARGB();
			}
		};
	}

	namespace type_details
	{
		template<typename scalar, typename vector2d_impl, typename vector3d_impl, typename vector4d_impl>
		using color_4d = details::vector_4d_impl<lm::type_details::type_vector4d<scalar, vector2d_impl, vector3d_impl, vector4d_impl>>;
	}

	using LinearColor = type_details::color_4d<float, lm::float2, lm::float3, lm::float4>;


	inline LinearColor modulate(LinearColor lhs, LinearColor  rhs) lnoexcept
	{
		return  { lhs.rgba * rhs.rgba };
	}

	inline LinearColor  lerp(LinearColor lhs, LinearColor rhs, float w) {
		return { lm::lerp<float>(lhs.rgba, rhs.rgba, w) };
	}

	inline float dot(LinearColor lhs, LinearColor rhs) {
		return dot(lhs.rgba, rhs.rgba);
	}

	void ConvertFromABGR32F(EFormat fmt, LinearColor const* input, uint32 num_elems, void* output);
	void ConvertToABGR32F(EFormat fmt, void const* input, uint32_t num_elems, LinearColor* output);

	template<EFormat format>
	struct TColor;

	template<>
	struct TColor<EF_ARGB8_SRGB>
	{
		using uint8 = leo::uint8;

		union { struct { uint8 B, G, R, A; }; leo::uint32 AlignmentDummy; };

		TColor() :AlignmentDummy(0) {}

		constexpr TColor(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255)
			: B(InB), G(InG), R(InR), A(InA)
		{}

		leo::uint32& DWColor() { return *((leo::uint32*)this); }
		const leo::uint32& DWColor() const { return *((leo::uint32*)this); }
	};

	template<>
	struct TColor<EF_ABGR32F> :LinearColor
	{};

	//	Stores a color with 8 bits of precision per channel.
	using FColor = TColor<EF_ARGB8_SRGB>;
}

namespace platform
{
	using LinearColor = LeoEngine::LinearColor;
	using FColor = LeoEngine::FColor;

	namespace M {
		using LeoEngine::ConvertFromABGR32F;
		using LeoEngine::ConvertToABGR32F;
	}
}

#endif