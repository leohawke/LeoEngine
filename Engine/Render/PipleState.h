/*! \file Engine\Render\PipeState.h
\ingroup Engine\Render
\important It's have default state
*/

#ifndef LE_RENDER_PIPESTATE_h
#define LE_RENDER_PIPESTATE_h 1

#include <LBase/linttype.hpp>
#include <LBase/id.hpp>
#include <LBase/string_utility.hpp>
#include <LBase/exception_type.h>
#include "Color_T.hpp"

#include <cctype>
#include <algorithm>
#include <compare>

namespace platform::Render {
	namespace details {
		template<typename T, typename Y>
		T union_type_as(Y v) {
			union Y2T
			{
				Y y;
				T t;
			} y2t;
			y2t.y = v;
			return y2t.t;
		}
	}



	enum class RasterizerMode {
		Point,
		Line,
		Face,
	};

	enum class CullMode {
		None,
		Front,
		Back
	};

	enum class BlendOp
	{
		Add = 1,
		Sub,
		Rev_Sub,
		Min,
		Max
	};

	enum class BlendFactor
	{
		Zero,
		One,
		Src_Alpha,
		Dst_Alpha,
		Inv_Src_Alpha,
		Inv_Dst_Alpha,
		Src_Color,
		Dst_Color,
		Inv_Src_Color,
		Inv_Dst_Color,
		Src_Alpha_Sat,
		Factor,
		Inv_Factor,
#if     1
		Src1_Alpha,
		Inv_Src1_Alpha,
		Src1_Color,
		Inv_Src1_Color
#endif
	};

	enum class CompareOp
	{
		Fail,
		Pass,
		Less,
		Less_Equal,
		Equal,
		NotEqual,
		GreaterEqual,
		Greater
	};

	enum class StencilOp
	{
		Keep,
		Zero,
		Replace,
		Incr,//clamp
		Decr,
		Invert,//bits invert
		Incr_Wrap,
		Decr_Wrap
	};

	enum class ColorMask : leo::uint8
	{
		Red = 1UL << 0,
		Green = 1UL << 1,
		Blue = 1UL << 2,
		Alpha = 1UL << 3,
		All = Red | Green | Blue | Alpha

	};

	enum class TexAddressingMode
	{
		Wrap,
		Mirror,
		Clamp,
		Border
	};

	enum class AsyncComputeBudget
	{
		All_4 = 4, //Async compute can use the entire GPU.
	};

	namespace details
	{
		enum TexFilterOp {
			Mip_Point = 0x0,
			Mip_Linear = 0x1,
			Mag_Point = 0x0,
			Mag_Linear = 0x2,
			Min_Point = 0x0,
			Min_Linear = 0x4,
			Anisotropic = 0x08,
			Comparison = 0x10,
		};
	}

	enum class TexFilterOp
	{
		Min_Mag_Mip_Point = details::Min_Point | details::Mag_Point | details::Mip_Point,
		Min_Mag_Point_Mip_Linear = details::Min_Point | details::Mag_Point | details::Mip_Linear,
		Min_Point_Mag_Linear_Mip_Point = details::Min_Point | details::Mag_Linear | details::Mip_Point,
		Min_Point_Mag_Mip_Linear = details::Min_Point | details::Mag_Linear | details::Mip_Linear,
		Min_Linear_Mag_Mip_Point = details::Min_Linear | details::Mag_Point | details::Mip_Point,
		Min_Linear_Mag_Point_Mip_Linear = details::Min_Linear | details::Mag_Point | details::Mip_Linear,
		Min_Mag_Linear_Mip_Point = details::Min_Linear | details::Mag_Linear | details::Mip_Point,
		Min_Mag_Mip_Linear = details::Min_Linear | details::Mag_Linear | details::Mip_Linear,
		Anisotropic = details::Anisotropic,

		Cmp_Min_Mag_Mip_Point = details::Comparison | Min_Mag_Mip_Point,
		Cmp_Min_Mag_Point_Mip_Linear = details::Comparison | Min_Mag_Point_Mip_Linear,
		Cmp_Min_Point_Mag_Linear_Mip_Point = details::Comparison | Min_Point_Mag_Linear_Mip_Point,
		Cmp_Min_Point_Mag_Mip_Linear = details::Comparison | Min_Point_Mag_Mip_Linear,
		Cmp_Min_Linear_Mag_Mip_Point = details::Comparison | Min_Linear_Mag_Mip_Point,
		Cmp_Min_Linear_Mag_Point_Mip_Linear = details::Comparison | Min_Linear_Mag_Point_Mip_Linear,
		Cmp_Min_Mag_Linear_Mip_Point = details::Comparison | Min_Mag_Linear_Mip_Point,
		Cmp_Min_Mag_Mip_Linear = details::Comparison | Min_Mag_Mip_Linear,
		Cmp_Anisotropic = details::Comparison | Anisotropic
	};

	struct RasterizerDesc
	{
		RasterizerMode mode;
		CullMode cull;
		bool ccw;
		bool depth_clip_enable;
		bool scissor_enable;
		bool multisample_enable;

		RasterizerDesc();
		friend auto operator<=>(const RasterizerDesc& lhs, const RasterizerDesc& rhs) = default;
		friend bool operator==(const RasterizerDesc& lhs, const RasterizerDesc& rhs) = default;

		template<typename T>
		static T to_mode(const std::string & value);


	
	};

	template<>
	inline RasterizerMode   RasterizerDesc::to_mode<RasterizerMode>(const std::string & value) {
		auto hash = leo::constfn_hash(value.c_str());
		switch (hash)
		{
		case leo::constfn_hash("Point"):
			return RasterizerMode::Point;
		case leo::constfn_hash("Line"):
			return RasterizerMode::Line;
		case leo::constfn_hash("Face"):
			return RasterizerMode::Face;
		}

		throw leo::narrowing_error();
	}

	template<>
	inline CullMode RasterizerDesc::to_mode<CullMode>(const std::string & value)
	{
		auto hash = leo::constfn_hash(value.c_str());
		switch (hash)
		{
		case leo::constfn_hash("None"):
			return CullMode::None;
		case leo::constfn_hash("Front"):
			return CullMode::Front;
		case leo::constfn_hash("Back"):
			return CullMode::Back;
		}

		throw leo::narrowing_error();
	}

	struct DepthStencilDesc
	{
		bool				depth_enable;
		bool				depth_write_mask;
		CompareOp		depth_func;

		bool				front_stencil_enable;
		CompareOp		front_stencil_func;
		leo::uint16			front_stencil_ref;
		leo::uint16			front_stencil_read_mask;
		leo::uint16			front_stencil_write_mask;
		StencilOp	front_stencil_fail;
		StencilOp	front_stencil_depth_fail;
		StencilOp	front_stencil_pass;

		bool				back_stencil_enable;
		CompareOp		back_stencil_func;
		leo::uint16			back_stencil_ref;
		leo::uint16			back_stencil_read_mask;
		leo::uint16			back_stencil_write_mask;
		StencilOp	back_stencil_fail;
		StencilOp	back_stencil_depth_fail;
		StencilOp	back_stencil_pass;

		DepthStencilDesc();

		friend auto operator<=>(const DepthStencilDesc& lhs, const DepthStencilDesc& rhs) = default;
		friend bool operator==(const DepthStencilDesc& lhs, const DepthStencilDesc& rhs) = default;

		template<typename T>
		static T to_op(const std::string & value);
	};


	template<>
	inline CompareOp DepthStencilDesc::to_op<CompareOp>(const std::string & value)
	{
		auto lower_value = value;
		leo::to_lower(lower_value);
		auto hash = leo::constfn_hash(lower_value.c_str());
		switch (hash) {
		case leo::constfn_hash("fail"):
			return CompareOp::Fail;
		case leo::constfn_hash("pass"):
			return CompareOp::Pass;
		case leo::constfn_hash("less"):
			return CompareOp::Less;
		case leo::constfn_hash("less_equal"):
			return CompareOp::Less_Equal;
		case leo::constfn_hash("equal"):
			return CompareOp::Equal;
		case leo::constfn_hash("notequal"):
			return CompareOp::NotEqual;
		case leo::constfn_hash("greaterequal"):
			return CompareOp::GreaterEqual;
		case leo::constfn_hash("greater"):
			return CompareOp::Greater;
		}

		throw leo::narrowing_error();
	}

	template<>
	inline StencilOp DepthStencilDesc::to_op<StencilOp>(const std::string& value)
	{
		auto lower_value = value;
		leo::to_lower(lower_value);
		auto hash = leo::constfn_hash(lower_value.c_str());
		switch (hash) {
		case leo::constfn_hash("keep"):
			return StencilOp::Keep;
		case leo::constfn_hash("zero"):
			return StencilOp::Zero;
		case leo::constfn_hash("replace"):
			return StencilOp::Replace;
		case leo::constfn_hash("decr"):
			return StencilOp::Decr;
		case leo::constfn_hash("invert"):
			return StencilOp::Invert;
		case leo::constfn_hash("incr_wrap"):
			return StencilOp::Incr_Wrap;
		case leo::constfn_hash("decr_wrap"):
			return StencilOp::Decr_Wrap;
		}

		throw leo::narrowing_error();
	}

	struct BlendDesc
	{
		M::Color blend_factor;
		uint32_t sample_mask;

		bool				alpha_to_coverage_enable;
		bool				independent_blend_enable;

		std::array<bool, 8>				blend_enable;
		std::array<bool, 8>				logic_op_enable;
		std::array<BlendOp, 8>	blend_op;
		std::array<BlendFactor, 8>	src_blend;
		std::array<BlendFactor, 8>	dst_blend;
		std::array<BlendOp, 8>	blend_op_alpha;
		std::array<BlendFactor, 8>	src_blend_alpha;
		std::array<BlendFactor, 8>	dst_blend_alpha;
		std::array<leo::uint8, 8>			color_write_mask;

		BlendDesc();

		friend auto operator<=>(const BlendDesc& lhs, const BlendDesc& rhs) = default;
		friend bool operator==(const BlendDesc& lhs, const BlendDesc& rhs) = default;


		static BlendFactor to_factor(const std::string& value)
		{
			auto hash = leo::constfn_hash(value.c_str());
			switch (hash) {
			case leo::constfn_hash("Zero"):
				return BlendFactor::Zero;
			case leo::constfn_hash("One"):
				return BlendFactor::One;
			case leo::constfn_hash("Src_Alpha"):
				return BlendFactor::Src_Alpha;
			case leo::constfn_hash("Dst_Alpha"):
				return BlendFactor::Dst_Alpha;
			case leo::constfn_hash("Inv_Src_Alpha"):
				return BlendFactor::Inv_Src_Alpha;
			case leo::constfn_hash("Inv_Dst_Alpha"):
				return BlendFactor::Inv_Dst_Alpha;
			case leo::constfn_hash("Src_Color"):
				return BlendFactor::Src_Color;
			case leo::constfn_hash("Dst_Color"):
				return BlendFactor::Dst_Color;
			case leo::constfn_hash("Inv_Src_Color"):
				return BlendFactor::Inv_Src_Color;
			case leo::constfn_hash("Inv_Dst_Color"):
				return BlendFactor::Inv_Dst_Color;
			case leo::constfn_hash("Src_Alpha_Sat"):
				return BlendFactor::Src_Alpha_Sat;
			case leo::constfn_hash("Factor"):
				return BlendFactor::Factor;
			case leo::constfn_hash("Inv_Factor"):
				return BlendFactor::Inv_Factor;
#if     1			
			case leo::constfn_hash("Src1_Alpha"):
				return BlendFactor::Src1_Alpha;
			case leo::constfn_hash("Inv_Src1_Alpha"):
				return BlendFactor::Inv_Src1_Alpha;
			case leo::constfn_hash("Src1_Color"):
				return BlendFactor::Src1_Color;
			case leo::constfn_hash("Inv_Src1_Color"):
				return BlendFactor::Inv_Src1_Color;
#endif
			}
			throw leo::narrowing_error();
		}

		static BlendOp to_op(const std::string& value) {
			auto hash = leo::constfn_hash(value.c_str());
			switch (hash) {
			case leo::constfn_hash("Add"):
				return BlendOp::Add;
			case leo::constfn_hash("Sub"):
				return BlendOp::Sub;
			case leo::constfn_hash("Rev_Sub"):
				return BlendOp::Rev_Sub;
			case leo::constfn_hash("Min"):
				return BlendOp::Min;
			case leo::constfn_hash("Max"):
				return BlendOp::Max;
			}
			throw leo::narrowing_error();
		}
	};

	class PipleState {
	public:
		explicit PipleState(const RasterizerDesc r_desc = {}, const DepthStencilDesc& ds_desc = {},
			const BlendDesc& b_desc = {})
			:RasterizerState(r_desc), DepthStencilState(ds_desc), BlendState(b_desc)
		{}

		virtual ~PipleState();

	public:
		RasterizerDesc RasterizerState;
		DepthStencilDesc DepthStencilState;
		BlendDesc BlendState;
	};

	struct TextureSampleDesc
	{
		M::Color border_clr;

		TexAddressingMode address_mode_u;
		TexAddressingMode address_mode_v;
		TexAddressingMode address_mode_w;

		TexFilterOp filtering;

		leo::uint8 max_anisotropy;
		float min_lod;
		float max_lod;
		float mip_map_lod_bias;

		CompareOp cmp_func;

		TextureSampleDesc();

		friend auto operator<=>(const TextureSampleDesc& lhs, const TextureSampleDesc& rhs) = default;
		friend bool operator==(const TextureSampleDesc& lhs, const TextureSampleDesc& rhs) = default;

		template<typename T>
		static T to_op(const std::string & value);

	

		static TexAddressingMode to_mode(const std::string& value) {
			auto lower_value = value;
			leo::to_lower(lower_value);
			auto hash = leo::constfn_hash(lower_value.c_str());
			switch (hash) {
			case leo::constfn_hash("wrap"):
				return TexAddressingMode::Wrap;
			case leo::constfn_hash("mirror"):
				return TexAddressingMode::Mirror;
			case leo::constfn_hash("clamp"):
				return TexAddressingMode::Clamp;
			case leo::constfn_hash("border"):
				return TexAddressingMode::Border;
			}
			throw leo::narrowing_error();
		}

		
	};

	template<>
	inline CompareOp TextureSampleDesc::to_op<CompareOp>(const std::string & value) {
		return DepthStencilDesc::to_op<CompareOp>(value);
	}

	template<>
	inline TexFilterOp TextureSampleDesc::to_op<TexFilterOp>(const std::string& value) {
		auto lower_value = value;
		leo::to_lower(lower_value);
		auto hash = leo::constfn_hash(lower_value.c_str());
		switch (hash) {
		case leo::constfn_hash("min_mag_mip_point"):
			return TexFilterOp::Min_Mag_Mip_Point;
		case leo::constfn_hash("min_mag_point_mip_linear"):
			return TexFilterOp::Min_Mag_Point_Mip_Linear;
		case leo::constfn_hash("min_point_mag_linear_mip_point"):
			return TexFilterOp::Min_Point_Mag_Linear_Mip_Point;
		case leo::constfn_hash("min_point_mag_mip_linear"):
			return TexFilterOp::Min_Point_Mag_Mip_Linear;
		case leo::constfn_hash("min_linear_mag_mip_point"):
			return TexFilterOp::Min_Linear_Mag_Mip_Point;
		case leo::constfn_hash("min_linear_mag_point_mip_linear"):
			return TexFilterOp::Min_Linear_Mag_Point_Mip_Linear;
		case leo::constfn_hash("min_mag_linear_mip_point"):
			return TexFilterOp::Min_Mag_Linear_Mip_Point;
		case leo::constfn_hash("min_mag_mip_linear"):
			return TexFilterOp::Min_Mag_Mip_Linear;
		case leo::constfn_hash("anisotropic"):
			return TexFilterOp::Anisotropic;
		case leo::constfn_hash("cmp_min_mag_mip_point"):
			return TexFilterOp::Cmp_Min_Mag_Mip_Point;
		case leo::constfn_hash("cmp_min_mag_point_mip_linear"):
			return TexFilterOp::Cmp_Min_Mag_Mip_Point;
		case leo::constfn_hash("cmp_min_point_mag_linear_mip_point"):
			return TexFilterOp::Cmp_Min_Point_Mag_Linear_Mip_Point;
		case leo::constfn_hash("cmp_min_point_mag_mip_linear"):
			return TexFilterOp::Cmp_Min_Point_Mag_Mip_Linear;
		case leo::constfn_hash("cmp_min_linear_mag_mip_point"):
			return TexFilterOp::Cmp_Min_Linear_Mag_Mip_Point;
		case leo::constfn_hash("cmp_min_linear_mag_point_mip_linear"):
			return TexFilterOp::Cmp_Min_Linear_Mag_Point_Mip_Linear;
		case leo::constfn_hash("cmp_min_mag_linear_mip_point"):
			return TexFilterOp::Cmp_Min_Mag_Linear_Mip_Point;
		case leo::constfn_hash("cmp_min_mag_mip_linear"):
			return TexFilterOp::Cmp_Min_Mag_Mip_Linear;
		case leo::constfn_hash("cmp_anisotropic"):
			return TexFilterOp::Cmp_Anisotropic;
		}
		throw leo::narrowing_error();
	}

}

#endif