#include "Convert.h"
#include <LBase/exception.h>

using namespace platform::Render;

DXGI_FORMAT platform_ex::Windows::D3D12::Convert(platform::Render::EFormat format)
{
	switch (format)
	{
	case EF_A8:
		return DXGI_FORMAT_A8_UNORM;

	case EF_R5G6B5:
		return DXGI_FORMAT_B5G6R5_UNORM;

	case EF_A1RGB5:
		return DXGI_FORMAT_B5G5R5A1_UNORM;

	case EF_ARGB4:
		return DXGI_FORMAT_B4G4R4A4_UNORM;

	case EF_R8:
		return DXGI_FORMAT_R8_UNORM;

	case EF_SIGNED_R8:
		return DXGI_FORMAT_R8_SNORM;

	case EF_GR8:
		return DXGI_FORMAT_R8G8_UNORM;

	case EF_SIGNED_GR8:
		return DXGI_FORMAT_R8G8_SNORM;

	case EF_ARGB8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;

	case EF_ABGR8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case EF_SIGNED_ABGR8:
		return DXGI_FORMAT_R8G8B8A8_SNORM;

	case EF_A2BGR10:
		return DXGI_FORMAT_R10G10B10A2_UNORM;

	case EF_SIGNED_A2BGR10:
		return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

	case EF_R8UI:
		return DXGI_FORMAT_R8_UINT;

	case EF_R8I:
		return DXGI_FORMAT_R8_SINT;

	case EF_GR8UI:
		return DXGI_FORMAT_R8G8_UINT;

	case EF_GR8I:
		return DXGI_FORMAT_R8G8_SINT;

	case EF_ABGR8UI:
		return DXGI_FORMAT_R8G8B8A8_UINT;

	case EF_ABGR8I:
		return DXGI_FORMAT_R8G8B8A8_SINT;

	case EF_A2BGR10UI:
		return DXGI_FORMAT_R10G10B10A2_UINT;

	case EF_R16:
		return DXGI_FORMAT_R16_UNORM;

	case EF_SIGNED_R16:
		return DXGI_FORMAT_R16_SNORM;

	case EF_GR16:
		return DXGI_FORMAT_R16G16_UNORM;

	case EF_SIGNED_GR16:
		return DXGI_FORMAT_R16G16_SNORM;

	case EF_ABGR16:
		return DXGI_FORMAT_R16G16B16A16_UNORM;

	case EF_SIGNED_ABGR16:
		return DXGI_FORMAT_R16G16B16A16_SNORM;

	case EF_R16UI:
		return DXGI_FORMAT_R16_UINT;

	case EF_R16I:
		return DXGI_FORMAT_R16_SINT;

	case EF_GR16UI:
		return DXGI_FORMAT_R16G16_UINT;

	case EF_GR16I:
		return DXGI_FORMAT_R16G16_SINT;

	case EF_ABGR16UI:
		return DXGI_FORMAT_R16G16B16A16_UINT;

	case EF_ABGR16I:
		return DXGI_FORMAT_R16G16B16A16_SINT;

	case EF_R32UI:
		return DXGI_FORMAT_R32_UINT;

	case EF_R32I:
		return DXGI_FORMAT_R32_SINT;

	case EF_GR32UI:
		return DXGI_FORMAT_R32G32_UINT;

	case EF_GR32I:
		return DXGI_FORMAT_R32G32_SINT;

	case EF_BGR32UI:
		return DXGI_FORMAT_R32G32B32_UINT;

	case EF_BGR32I:
		return DXGI_FORMAT_R32G32B32_SINT;

	case EF_ABGR32UI:
		return DXGI_FORMAT_R32G32B32A32_UINT;

	case EF_ABGR32I:
		return DXGI_FORMAT_R32G32B32A32_SINT;

	case EF_R16F:
		return DXGI_FORMAT_R16_FLOAT;

	case EF_GR16F:
		return DXGI_FORMAT_R16G16_FLOAT;

	case EF_B10G11R11F:
		return DXGI_FORMAT_R11G11B10_FLOAT;

	case EF_ABGR16F:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;

	case EF_R32F:
		return DXGI_FORMAT_R32_FLOAT;

	case EF_GR32F:
		return DXGI_FORMAT_R32G32_FLOAT;

	case EF_BGR32F:
		return DXGI_FORMAT_R32G32B32_FLOAT;

	case EF_ABGR32F:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;

	case EF_BC1:
		return DXGI_FORMAT_BC1_UNORM;

	case EF_BC2:
		return DXGI_FORMAT_BC2_UNORM;

	case EF_BC3:
		return DXGI_FORMAT_BC3_UNORM;

	case EF_BC4:
		return DXGI_FORMAT_BC4_UNORM;

	case EF_SIGNED_BC4:
		return DXGI_FORMAT_BC4_SNORM;

	case EF_BC5:
		return DXGI_FORMAT_BC5_UNORM;

	case EF_SIGNED_BC5:
		return DXGI_FORMAT_BC5_SNORM;

	case EF_BC6:
		return DXGI_FORMAT_BC6H_UF16;

	case EF_SIGNED_BC6:
		return DXGI_FORMAT_BC6H_SF16;

	case EF_BC7:
		return DXGI_FORMAT_BC7_UNORM;

	case EF_D16:
		return DXGI_FORMAT_D16_UNORM;

	case EF_D24S8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;

	case EF_D32F:
		return DXGI_FORMAT_D32_FLOAT;

	case EF_ARGB8_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

	case EF_ABGR8_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	case EF_BC1_SRGB:
		return DXGI_FORMAT_BC1_UNORM_SRGB;

	case EF_BC2_SRGB:
		return DXGI_FORMAT_BC2_UNORM_SRGB;

	case EF_BC3_SRGB:
		return DXGI_FORMAT_BC3_UNORM_SRGB;

	case EF_BC7_SRGB:
		return DXGI_FORMAT_BC7_UNORM_SRGB;

	}
	leo::raise_exception(leo::unsupported());
}

platform::Render::EFormat platform_ex::Windows::D3D12::Convert(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_A8_UNORM:
		return EF_A8;

	case DXGI_FORMAT_B5G6R5_UNORM:
		return EF_R5G6B5;

	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return EF_A1RGB5;

	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return EF_ARGB4;

	case DXGI_FORMAT_R8_UNORM:
		return EF_R8;

	case DXGI_FORMAT_R8_SNORM:
		return EF_SIGNED_R8;

	case DXGI_FORMAT_R8G8_UNORM:
		return EF_GR8;

	case DXGI_FORMAT_R8G8_SNORM:
		return EF_SIGNED_GR8;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return EF_ARGB8;

	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return EF_ABGR8;

	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return EF_SIGNED_ABGR8;

	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return EF_A2BGR10;

	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		return EF_SIGNED_A2BGR10;

	case DXGI_FORMAT_R8_UINT:
		return EF_R8UI;

	case DXGI_FORMAT_R8_SINT:
		return EF_R8I;

	case DXGI_FORMAT_R8G8_UINT:
		return EF_GR8UI;

	case DXGI_FORMAT_R8G8_SINT:
		return EF_GR8I;

	case DXGI_FORMAT_R8G8B8A8_UINT:
		return EF_ABGR8UI;

	case DXGI_FORMAT_R8G8B8A8_SINT:
		return EF_ABGR8I;

	case DXGI_FORMAT_R10G10B10A2_UINT:
		return EF_A2BGR10UI;

	case DXGI_FORMAT_R16_UNORM:
		return EF_R16;

	case DXGI_FORMAT_R16_SNORM:
		return EF_SIGNED_R16;

	case DXGI_FORMAT_R16G16_UNORM:
		return EF_GR16;

	case DXGI_FORMAT_R16G16_SNORM:
		return EF_SIGNED_GR16;

	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return EF_ABGR16;

	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return EF_SIGNED_ABGR16;

	case DXGI_FORMAT_R16_UINT:
		return EF_R16UI;

	case DXGI_FORMAT_R16_SINT:
		return EF_R16I;

	case DXGI_FORMAT_R16G16_UINT:
		return EF_GR16UI;

	case DXGI_FORMAT_R16G16_SINT:
		return EF_GR16I;

	case DXGI_FORMAT_R16G16B16A16_UINT:
		return EF_ABGR16UI;

	case DXGI_FORMAT_R16G16B16A16_SINT:
		return EF_ABGR16I;

	case DXGI_FORMAT_R32_UINT:
		return EF_R32UI;

	case DXGI_FORMAT_R32_SINT:
		return EF_R32I;

	case DXGI_FORMAT_R32G32_UINT:
		return EF_GR32UI;

	case DXGI_FORMAT_R32G32_SINT:
		return EF_GR32I;

	case DXGI_FORMAT_R32G32B32_UINT:
		return EF_BGR32UI;

	case DXGI_FORMAT_R32G32B32_SINT:
		return EF_BGR32I;

	case DXGI_FORMAT_R32G32B32A32_UINT:
		return EF_ABGR32UI;

	case DXGI_FORMAT_R32G32B32A32_SINT:
		return EF_ABGR32I;

	case DXGI_FORMAT_R16_FLOAT:
		return EF_R16F;

	case DXGI_FORMAT_R16G16_FLOAT:
		return EF_GR16F;

	case DXGI_FORMAT_R11G11B10_FLOAT:
		return EF_B10G11R11F;

	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return EF_ABGR16F;

	case DXGI_FORMAT_R32_FLOAT:
		return EF_R32F;

	case DXGI_FORMAT_R32G32_FLOAT:
		return EF_GR32F;

	case DXGI_FORMAT_R32G32B32_FLOAT:
		return EF_BGR32F;

	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return EF_ABGR32F;

	case DXGI_FORMAT_BC1_UNORM:
		return EF_BC1;

	case DXGI_FORMAT_BC2_UNORM:
		return EF_BC2;

	case DXGI_FORMAT_BC3_UNORM:
		return EF_BC3;

	case DXGI_FORMAT_BC4_UNORM:
		return EF_BC4;

	case DXGI_FORMAT_BC4_SNORM:
		return EF_SIGNED_BC4;

	case DXGI_FORMAT_BC5_UNORM:
		return EF_BC5;

	case DXGI_FORMAT_BC5_SNORM:
		return EF_SIGNED_BC5;

	case DXGI_FORMAT_BC6H_UF16:
		return EF_BC6;

	case DXGI_FORMAT_BC6H_SF16:
		return EF_SIGNED_BC6;

	case DXGI_FORMAT_BC7_UNORM:
		return EF_BC7;

	case DXGI_FORMAT_D16_UNORM:
		return EF_D16;

	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return EF_D24S8;

	case DXGI_FORMAT_D32_FLOAT:
		return EF_D32F;

	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return EF_ARGB8_SRGB;

	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return EF_ABGR8_SRGB;

	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return EF_BC1_SRGB;

	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return EF_BC2_SRGB;

	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return EF_BC3_SRGB;

	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return EF_BC7_SRGB;


	}
	leo::raise_exception(leo::unsupported());
}

D3D12_SAMPLER_DESC platform_ex::Windows::D3D12::Convert(platform::Render::SamplerDesc desc)
{
	D3D12_SAMPLER_DESC sampler_desc;
	sampler_desc.Filter = Convert(desc.filtering);
	sampler_desc.AddressU = Convert(desc.address_mode_u);
	sampler_desc.AddressV = Convert(desc.address_mode_v);
	sampler_desc.AddressW = Convert(desc.address_mode_w);

	sampler_desc.MipLODBias = desc.mip_map_lod_bias;
	sampler_desc.MaxAnisotropy = desc.max_anisotropy;
	sampler_desc.ComparisonFunc = Convert(desc.cmp_func);

	sampler_desc.BorderColor[0] = desc.border_clr.r();
	sampler_desc.BorderColor[1] = desc.border_clr.g();
	sampler_desc.BorderColor[2] = desc.border_clr.b();
	sampler_desc.BorderColor[3] = desc.border_clr.a();
	sampler_desc.MinLOD = desc.min_lod;
	sampler_desc.MaxLOD = desc.max_lod;

	return sampler_desc;
}

D3D12_TEXTURE_ADDRESS_MODE platform_ex::Windows::D3D12::Convert(platform::Render::TexAddressingMode mode)
{
	switch (mode)
	{
	case platform::Render::TexAddressingMode::Wrap:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		break;
	case platform::Render::TexAddressingMode::Mirror:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		break;
	case platform::Render::TexAddressingMode::Clamp:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		break;
	case platform::Render::TexAddressingMode::Border:
		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		break;
	}
	leo::raise_exception(leo::unsupported());
}

D3D12_FILTER platform_ex::Windows::D3D12::Convert(platform::Render::TexFilterOp op)
{
	switch (op)
	{
	case platform::Render::TexFilterOp::Min_Mag_Mip_Point:
		return D3D12_FILTER_MIN_MAG_MIP_POINT;
		break;
	case platform::Render::TexFilterOp::Min_Mag_Point_Mip_Linear:
		return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		break;
	case platform::Render::TexFilterOp::Min_Point_Mag_Linear_Mip_Point:
		return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	case platform::Render::TexFilterOp::Min_Point_Mag_Mip_Linear:
		return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		break;
	case platform::Render::TexFilterOp::Min_Linear_Mag_Mip_Point:
		return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		break;
	case platform::Render::TexFilterOp::Min_Linear_Mag_Point_Mip_Linear:
		return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	case platform::Render::TexFilterOp::Min_Mag_Linear_Mip_Point:
		return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case platform::Render::TexFilterOp::Min_Mag_Mip_Linear:
		return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case platform::Render::TexFilterOp::Anisotropic:
		return D3D12_FILTER_ANISOTROPIC;
		break;
	case platform::Render::TexFilterOp::Cmp_Min_Mag_Mip_Point:
		return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		break;
	case platform::Render::TexFilterOp::Cmp_Min_Mag_Point_Mip_Linear:
		return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		break;
	case platform::Render::TexFilterOp::Cmp_Min_Point_Mag_Linear_Mip_Point:
		return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	case platform::Render::TexFilterOp::Cmp_Min_Point_Mag_Mip_Linear:
		return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		break;
	case platform::Render::TexFilterOp::Cmp_Min_Linear_Mag_Mip_Point:
		return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		break;
	case platform::Render::TexFilterOp::Cmp_Min_Linear_Mag_Point_Mip_Linear:
		return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	case platform::Render::TexFilterOp::Cmp_Min_Mag_Linear_Mip_Point:
		return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case platform::Render::TexFilterOp::Cmp_Min_Mag_Mip_Linear:
		return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		break;
	case platform::Render::TexFilterOp::Cmp_Anisotropic:
		return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		break;
	}
	leo::raise_exception(leo::unsupported());
}

D3D12_COMPARISON_FUNC platform_ex::Windows::D3D12::Convert(platform::Render::CompareOp op)
{
	switch (op)
	{
	case platform::Render::CompareOp::Fail:
		return D3D12_COMPARISON_FUNC_NEVER;
		break;
	case platform::Render::CompareOp::Pass:
		return D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case platform::Render::CompareOp::Less:
		return D3D12_COMPARISON_FUNC_LESS;
		break;
	case platform::Render::CompareOp::Less_Equal:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case platform::Render::CompareOp::Equal:
		return D3D12_COMPARISON_FUNC_EQUAL;
		break;
	case platform::Render::CompareOp::NotEqual:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		break;
	case platform::Render::CompareOp::GreaterEqual:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		break;
	case platform::Render::CompareOp::Greater:
		return D3D12_COMPARISON_FUNC_GREATER;
		break;
	}
	leo::raise_exception(leo::unsupported());
}

std::vector<D3D12_INPUT_ELEMENT_DESC> platform_ex::Windows::D3D12::Convert(const platform::Render::Vertex::Stream & stream)
{
	using platform::Render::Vertex::Usage;
	std::vector<D3D12_INPUT_ELEMENT_DESC> result;

	uint16 elem_offset = 0;
	for (auto& element : stream.elements) {
		D3D12_INPUT_ELEMENT_DESC d3d12_element;
		d3d12_element.SemanticIndex = element.usage_index;
		d3d12_element.Format = Convert(element.format);
		d3d12_element.AlignedByteOffset = elem_offset;
		if (stream.type == platform::Render::Vertex::Stream::Geomerty) {
			d3d12_element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			d3d12_element.InstanceDataStepRate = 0;
		}

		switch (element.usage) {
		case Usage::Position:
			d3d12_element.SemanticName = "POSITION";
			break;
		case Usage::Normal:
			d3d12_element.SemanticName = "NORMAL";
			break;
		case Usage::Binoraml:
			d3d12_element.SemanticName = "BINORMAL";
			break;
		case Usage::Tangent:
			d3d12_element.SemanticName = "TANGENT";
			break;

		case Usage::Diffuse:
		case Usage::Specular:
			d3d12_element.SemanticName = "COLOR";
			break;

		case Usage::BlendIndex:
			d3d12_element.SemanticName = "BLENDINDEX";
			break;

		case Usage::BlendWeight:
			d3d12_element.SemanticName = "BLENDWEIGHT";

		case Usage::TextureCoord:
			d3d12_element.SemanticName = "TEXCOORD";
		}

		elem_offset += element.GetElementSize();
		result.emplace_back(d3d12_element);
	}

	return result;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE platform_ex::Windows::D3D12::Convert(platform::Render::InputLayout::TopologyType type)
{
	switch (type)
	{
	case platform::Render::InputLayout::PointList:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case platform::Render::InputLayout::LineList:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case platform::Render::InputLayout::LineStrip:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case platform::Render::InputLayout::TriangleList:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case platform::Render::InputLayout::TriangleStrip:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

D3D12_BLEND_DESC platform_ex::Windows::D3D12::Convert(const platform::Render::BlendDesc & desc)
{
	D3D12_BLEND_DESC result;
	result.AlphaToCoverageEnable = desc.alpha_to_coverage_enable;
	result.IndependentBlendEnable = desc.independent_blend_enable;
	for (auto i = 0; i != leo::arrlen(result.RenderTarget); ++i) {
		result.RenderTarget[i].BlendEnable = desc.blend_enable[i];
		//TODO Support LogicOp
		result.RenderTarget[i].LogicOpEnable = false/* desc.logic_op_enable[i]*/;
		result.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
		result.RenderTarget[i].SrcBlend = Convert(desc.src_blend[i]);
		result.RenderTarget[i].DestBlend = Convert(desc.dst_blend[i]);
		result.RenderTarget[i].BlendOp = Convert(desc.blend_op[i]);
		result.RenderTarget[i].SrcBlendAlpha = Convert(desc.src_blend_alpha[i]);
		result.RenderTarget[i].DestBlendAlpha = Convert(desc.dst_blend_alpha[i]);
		result.RenderTarget[i].BlendOpAlpha = Convert(desc.blend_op_alpha[i]);
		result.RenderTarget[i].RenderTargetWriteMask = desc.color_write_mask[i];
	}
	return result;
}


D3D12_RASTERIZER_DESC platform_ex::Windows::D3D12::Convert(const platform::Render::RasterizerDesc & desc)
{
	D3D12_RASTERIZER_DESC result;
	result.CullMode = Convert(desc.cull);
	result.FillMode = Convert(desc.mode);
	result.FrontCounterClockwise = desc.ccw;
	result.DepthClipEnable = desc.depth_clip_enable;
	result.MultisampleEnable = desc.multisample_enable;

	//TODO Support DepthBias
	result.DepthBias = 0;
	result.DepthBiasClamp = 0;
	result.SlopeScaledDepthBias = 0;

	result.AntialiasedLineEnable = false;
	result.ForcedSampleCount = 0;
	result.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return result;
}

D3D12_STENCIL_OP platform_ex::Windows::D3D12::Convert(platform::Render::StencilOp op)
{
	switch (op)
	{
	case StencilOp::Keep:
		return D3D12_STENCIL_OP_KEEP;
	case StencilOp::Zero:
		return D3D12_STENCIL_OP_ZERO;
	case StencilOp::Replace:
		return D3D12_STENCIL_OP_REPLACE;
	case StencilOp::Incr:
		return D3D12_STENCIL_OP_INCR_SAT;
	case StencilOp::Decr:
		return D3D12_STENCIL_OP_DECR_SAT;
	case StencilOp::Invert:
		return D3D12_STENCIL_OP_INVERT;
	case StencilOp::Incr_Wrap:
		return D3D12_STENCIL_OP_INCR;
	case StencilOp::Decr_Wrap:
		return D3D12_STENCIL_OP_DECR;

	};
	return D3D12_STENCIL_OP_KEEP;

}

D3D12_DEPTH_STENCIL_DESC platform_ex::Windows::D3D12::Convert(const platform::Render::DepthStencilDesc & desc)
{
	D3D12_DEPTH_STENCIL_DESC result;

	result.DepthEnable = desc.depth_enable;
	result.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)desc.depth_write_mask;
	result.DepthFunc =Convert(desc.depth_func);
	result.StencilEnable = desc.front_stencil_enable;
	result.StencilReadMask =static_cast<UINT8>(desc.front_stencil_read_mask);
	result.StencilWriteMask = static_cast<UINT8>(desc.front_stencil_write_mask);

	result.BackFace.StencilFunc = Convert(desc.back_stencil_func);
	result.BackFace.StencilDepthFailOp =Convert( desc.back_stencil_depth_fail);
	result.BackFace.StencilFailOp = Convert( desc.back_stencil_fail);
	result.BackFace.StencilPassOp = Convert(desc.back_stencil_pass);

	result.FrontFace.StencilFunc = Convert(desc.front_stencil_func);
	result.FrontFace.StencilDepthFailOp = Convert(desc.front_stencil_depth_fail);
	result.FrontFace.StencilFailOp = Convert(desc.front_stencil_fail);
	result.FrontFace.StencilPassOp = Convert(desc.front_stencil_pass);

	return result;
}

D3D12_BLEND_OP platform_ex::Windows::D3D12::Convert(platform::Render::BlendOp op)
{
	switch (op)
	{
	case platform::Render::BlendOp::Add:
		return D3D12_BLEND_OP_ADD;
	case platform::Render::BlendOp::Sub:
		return D3D12_BLEND_OP_SUBTRACT;
	case platform::Render::BlendOp::Rev_Sub:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case platform::Render::BlendOp::Min:
		return D3D12_BLEND_OP_MIN;
	case platform::Render::BlendOp::Max:
		return D3D12_BLEND_OP_MIN;
	}
	throw leo::unsupported();
}

D3D12_BLEND platform_ex::Windows::D3D12::Convert(platform::Render::BlendFactor factor)
{
	switch (factor)
	{
	case platform::Render::BlendFactor::Zero:
		return D3D12_BLEND_ZERO;
	case platform::Render::BlendFactor::One:
		return D3D12_BLEND_ONE;
	case platform::Render::BlendFactor::Src_Alpha:
		return D3D12_BLEND_SRC_ALPHA;
	case platform::Render::BlendFactor::Dst_Alpha:
		return D3D12_BLEND_DEST_ALPHA;
	case platform::Render::BlendFactor::Inv_Src_Alpha:
		return D3D12_BLEND_SRC_ALPHA;
	case platform::Render::BlendFactor::Inv_Dst_Alpha:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case platform::Render::BlendFactor::Src_Color:
		return D3D12_BLEND_SRC_COLOR;
	case platform::Render::BlendFactor::Dst_Color:
		return D3D12_BLEND_DEST_COLOR;
	case platform::Render::BlendFactor::Inv_Src_Color:
		return D3D12_BLEND_INV_SRC_COLOR;
	case platform::Render::BlendFactor::Inv_Dst_Color:
		return D3D12_BLEND_INV_DEST_COLOR;
	case platform::Render::BlendFactor::Src_Alpha_Sat:
		return D3D12_BLEND_SRC_ALPHA_SAT;
	case platform::Render::BlendFactor::Factor:
		return D3D12_BLEND_BLEND_FACTOR;
	case platform::Render::BlendFactor::Inv_Factor:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	case platform::Render::BlendFactor::Src1_Alpha:
		return D3D12_BLEND_SRC1_ALPHA;
	case platform::Render::BlendFactor::Inv_Src1_Alpha:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case platform::Render::BlendFactor::Src1_Color:
		return D3D12_BLEND_SRC1_COLOR;
	case platform::Render::BlendFactor::Inv_Src1_Color:
		return D3D12_BLEND_INV_SRC1_COLOR;
	}
	throw leo::unsupported();
}

D3D12_FILL_MODE platform_ex::Windows::D3D12::Convert(platform::Render::RasterizerMode mode)
{
	switch (mode) {
		//unsupported??
	case RasterizerMode::Point:
	case RasterizerMode::Line:
		return D3D12_FILL_MODE_WIREFRAME;
	case RasterizerMode::Face:
		return D3D12_FILL_MODE_SOLID;
	}
	throw leo::unsupported();
}

D3D12_CULL_MODE platform_ex::Windows::D3D12::Convert(platform::Render::CullMode mode)
{
	switch (mode) {
	case CullMode::Back:
		return D3D12_CULL_MODE_BACK;
	case CullMode::Front:
		return D3D12_CULL_MODE_FRONT;
	case CullMode::None:
		return D3D12_CULL_MODE_NONE;
	}
	throw leo::unsupported();
}