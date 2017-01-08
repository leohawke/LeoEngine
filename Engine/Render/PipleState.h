/*! \file Engine\Render\PipeState.h
\ingroup Engine\Render
\important It's have default state
*/

#ifndef LE_RENDER_PIPESTATE_h
#define LE_RENDER_PIPESTATE_h 1

#include <LBase/linttype.hpp>

#include "Color_T.hpp"

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
		friend bool operator<(const RasterizerDesc& lhs, const RasterizerDesc& rhs);
	};


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

		friend bool operator<(const DepthStencilDesc  & lhs, const DepthStencilDesc  & rhs);
	};

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
		std::array<BlendFactor, 8>	dest_blend;
		std::array<BlendOp, 8>	blend_op_alpha;
		std::array<BlendFactor, 8>	src_blend_alpha;
		std::array<BlendFactor, 8>	dest_blend_alpha;
		std::array<leo::uint8, 8>			color_write_mask;

		BlendDesc();

		friend bool operator<(const BlendDesc  & lhs, const BlendDesc  & rhs);
	};

	class PipleState {
	public:
		explicit PipleState(const RasterizerDesc r_desc = {},const DepthStencilDesc& ds_desc = {},
			const BlendDesc& b_desc = {})
			:RasterizerState(r_desc),DepthStencilState(ds_desc),BlendState(b_desc)
		{}

	public:
		RasterizerDesc RasterizerState;
		DepthStencilDesc DepthStencilState;
		BlendDesc BlendState;
	};

	struct SamplerDesc
	{
		M::Color border_clr;

		TexAddressingMode addr_mode_u;
		TexAddressingMode addr_mode_v;
		TexAddressingMode addr_mode_w;

		TexFilterOp filter;

		leo::uint8 max_anisotropy;
		float min_lod;
		float max_lod;
		float mip_map_lod_bias;

		CompareOp cmp_func;

		SamplerDesc();

		friend bool operator<(const SamplerDesc  & lhs, const SamplerDesc  & rhs);
	};

}

#endif