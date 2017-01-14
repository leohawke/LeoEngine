#include "PipleState.h"

namespace platform::Render {

	RasterizerDesc::RasterizerDesc()
		: mode(RasterizerMode::Face),
		cull(CullMode::Back),
		depth_clip_enable(true),
		scissor_enable(false),
		multisample_enable(true)
	{
	}

	bool operator<(RasterizerDesc const & lhs, RasterizerDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}

	DepthStencilDesc::DepthStencilDesc()
		: depth_enable(true),
		depth_write_mask(true),
		depth_func(CompareOp::Less),
		front_stencil_enable(false),
		front_stencil_func(CompareOp::Pass),
		front_stencil_ref(0),
		front_stencil_read_mask(0xFFFF),
		front_stencil_write_mask(0xFFFF),
		front_stencil_fail(StencilOp::Keep),
		front_stencil_depth_fail(StencilOp::Keep),
		front_stencil_pass(StencilOp::Keep),
		back_stencil_enable(false),
		back_stencil_func(CompareOp::Pass),
		back_stencil_ref(0),
		back_stencil_read_mask(0xFFFF),
		back_stencil_write_mask(0xFFFF),
		back_stencil_fail(StencilOp::Keep),
		back_stencil_depth_fail(StencilOp::Keep),
		back_stencil_pass(StencilOp::Keep)
	{
	}

	bool operator<(DepthStencilDesc const & lhs, DepthStencilDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}

	BlendDesc::BlendDesc()
		: blend_factor(1, 1, 1, 1), sample_mask(0xFFFFFFFF),
		alpha_to_coverage_enable(false), independent_blend_enable(false)
	{
		blend_enable.fill(false);
		logic_op_enable.fill(false);
		blend_op.fill(BlendOp::Add);
		src_blend.fill(BlendFactor::One);
		dst_blend.fill(BlendFactor::Zero);
		blend_op_alpha.fill(BlendOp::Add);
		src_blend_alpha.fill(BlendFactor::One);
		dst_blend_alpha.fill(BlendFactor::Zero);
		color_write_mask.fill((leo::uint8)ColorMask::All);
	}

	bool operator<(BlendDesc const & lhs, BlendDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}

	SamplerDesc::SamplerDesc()
		: border_clr(0, 0, 0, 0),
		addr_mode_u(TexAddressingMode::Wrap), addr_mode_v(TexAddressingMode::Wrap), addr_mode_w(TexAddressingMode::Wrap),
		filter(TexFilterOp::Min_Mag_Mip_Point),
		max_anisotropy(16),
		min_lod(0), max_lod(std::numeric_limits<float>::max()),
		mip_map_lod_bias(0),
		cmp_func(CompareOp::Fail)
	{
	}

	bool operator<(SamplerDesc const & lhs, SamplerDesc const & rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
	}
}