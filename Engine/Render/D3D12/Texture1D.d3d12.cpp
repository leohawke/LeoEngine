#include "Texture.h"
#include "RenderView.h"
#include "Convert.h"
#include "Context.h"
#include "ShaderCompose.h"
#include "PipleState.h"

#include <LBase/ALGORITHM.HPP>

using namespace platform_ex::Windows::D3D12;
using BTexture = platform::Render::Texture1D;

Texture1D::Texture1D(uint16 width_, uint8 numMipMaps, uint8 array_size_, EFormat format_, uint32 access_hint, platform::Render::SampleDesc sample_info)
	:BTexture(numMipMaps, array_size_, format_, access_hint, sample_info),
	Texture(format_),
	width(width_)
{
	if (0 == mipmap_size) {
		mipmap_size = 1;
		auto w = width;
		while (w > 1) {
			++mipmap_size;
			w /= 2;
		}
	}
}

void Texture1D::BuildMipSubLevels()
{
	if (IsDepthFormat(format) || IsCompressedFormat(format)) {
		for (uint8 index = 0; index != GetArraySize(); ++index)
		{
			for (uint8 level = 1; level != GetNumMipMaps(); ++level)
			{
				Resize(*this, { index, level, 0,  GetWidth(level) },
				{ index, static_cast<uint8>(level - 1), 0,GetWidth(level - 1) }, true);
			}
		}
	}
	else {
		auto & device = D3D12::Context::Instance().GetDevice();
		auto & effect = *device.BlitEffect();
		auto & tech = effect.BilinearCopy;
		auto & pass = tech.GetPass(0);
		pass.Bind(effect);

		auto& sc = static_cast<ShaderCompose&>(pass.GetShader(effect));

		//RenderLayout& rl = static_cast<RenderLayout&>(PostProcessLayout {});
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gps_desc {};
		gps_desc.pRootSignature = sc.RootSignature();
		{
			gps_desc.VS << sc.VertexShader;
			gps_desc.PS << sc.PixelShader;
			
			gps_desc.DS << nullptr;
			gps_desc.HS << nullptr;
			gps_desc.GS << nullptr;
		}

		gps_desc.StreamOutput << nullptr;
		auto& state = static_cast<PipleState&>(pass.GetState());

		gps_desc.BlendState = state.BlendState;
		gps_desc.SampleMask = 0xFFFFFFFF;
		gps_desc.RasterizerState = state.RasterizerState;
		
		//gps_desc.InputLayout << rl.InputLayout;

		gps_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;//rl.IndexFormat <=
		gps_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		gps_desc.NumRenderTargets = 1;
		gps_desc.RTVFormats[0] = dxgi_format;

		for (uint32_t i = gps_desc.NumRenderTargets; i != leo::arrlen(gps_desc.RTVFormats); ++i)
		{
			gps_desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}

		gps_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		gps_desc.SampleDesc.Count = 1;
		gps_desc.SampleDesc.Quality = 0;
		gps_desc.NodeMask = 0;
		gps_desc.CachedPSO.pCachedBlob = nullptr;
		gps_desc.CachedPSO.CachedBlobSizeInBytes = 0;
		gps_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		COMPtr<ID3D12PipelineState> pso;
		//<<device.CreateGraphicsPipelineState(pso):
		device->CreateGraphicsPipelineState(&gps_desc, COMPtr_RefParam(pso, IID_ID3D12PipelineState));

		auto& cmd_list = D3D12::Context::Instance().GetCommandList(Device::Command_Render);
		cmd_list->SetPipelineState(pso.Get());
		cmd_list->SetGraphicsRootSignature(gps_desc.pRootSignature);

		/*
		COMPtr<ID3D12DescriptorHeap> dynamic_heap = device.CreateDynamicCbcSRVUAVDescriptorHeap(array_size);
		auto & sampler_heap = sc.SamplerHeap();

		*/
		ID3D12DescriptorHeap* heaps[] = { nullptr,nullptr };/*dynamic_heap.Get(),sampler_heap.Get()*/
		cmd_list->SetDescriptorHeaps(static_cast<UINT>(leo::arrlen(heaps)), heaps);

		/*
		if(sampler_heap)
			cmd_List->SetGraphicRootDescriptorTable(1,sampler_heap->GetGPUDescriptorHandleForHeapStart());
		*/

		/*
		auto & vb = static_cast<GraphicsBuffer&>(rl.GetVertexStream());

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = vb.GetGPUVirtualAddress();
		vbv.SizeInBytes = vb.Size();
		vbv.StrideInBytes = rl.VertexSize(0);

		cmd_list->IASetVertexBuffers(0, 1, &vbv);
		*/
		cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		D3D12_VIEWPORT vp;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;

		D3D12_RECT scissor_rc;
		scissor_rc.left = 0;
		scissor_rc.top = 0;

		D3D12_RESOURCE_BARRIER barrier_before[2];
		barrier_before[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before[0].Transition.pResource = texture.Get();
		barrier_before[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier_before[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before[1].Transition.pResource = texture.Get();
		barrier_before[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier_before[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		D3D12_RESOURCE_BARRIER barrier_after[2] = { barrier_before[0],barrier_before[1] };
		std::swap(barrier_after[0].Transition.StateBefore, barrier_after[0].Transition.StateAfter);
		std::swap(barrier_after[1].Transition.StateBefore, barrier_after[1].Transition.StateAfter);

		/*auto cpu_cbv_srv_uav_handle = dynamic_heap->GetCPUDescriptorHandleForHeapStart();
		auto gpu_cbv_srv_uav_handle = dynamic_heap->GetGPUDescriptorHandleForHeapStart();*/
		auto srv_desc_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		for (uint8_t index = 0; index < array_size; ++index)
		{
			for (uint8_t level = 1; level < mipmap_size; ++level)
			{
				/*cmd_list->SetGraphicsRootDescriptorTable(0, gpu_cbv_srv_uav_handle);*/

				barrier_before[0].Transition.Subresource = CalcSubresource(level - 1, index, 0, mipmap_size, array_size);
				barrier_before[1].Transition.Subresource = CalcSubresource(level, index, 0, mipmap_size, array_size);
				cmd_list->ResourceBarrier(2, barrier_before);

				auto const & rt_handle = Texture::RetriveRenderTargetView(index, 1, level)->GetHandle();

				auto const & sr_handle = RetriveShaderResourceView(index, 1, level - 1, 1)->GetHandle();
				/*device->CopyDescriptorsSimple(1, cpu_cbv_srv_uav_handle, sr_handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);*/

				cmd_list->OMSetRenderTargets(1, &rt_handle, false, nullptr);

				vp.Width = static_cast<float>(GetWidth(level));
				vp.Height = 0;
				cmd_list->RSSetViewports(1, &vp);

				scissor_rc.right = GetWidth(level);
				scissor_rc.bottom = 0;
				cmd_list->RSSetScissorRects(1, &scissor_rc);

				cmd_list->DrawInstanced(4, 1, 0, 0);

				barrier_after[0].Transition.Subresource = barrier_before[0].Transition.Subresource;
				barrier_after[1].Transition.Subresource = barrier_before[1].Transition.Subresource;
				cmd_list->ResourceBarrier(2, barrier_after);

				/*cpu_cbv_srv_uav_handle.ptr += srv_desc_size;
				gpu_cbv_srv_uav_handle.ptr += srv_desc_size;*/
			}
		}

		pass.UnBind(effect);
	}
}

void Texture1D::HWResourceCreate(ElementInitData const *  init_data)
{
	Texture::DoCreateHWResource(D3D12_RESOURCE_DIMENSION_TEXTURE1D,
		width, 1, 1, array_size,
		init_data);
}

void Texture1D::HWResourceDelete()
{
	return Texture::DeleteHWResource();
}

bool Texture1D::HWResourceReady() const
{
	return Texture::ReadyHWResource();
}

uint16 Texture1D::GetWidth(uint8 level) const
{
	LAssert(level < mipmap_size, "level out of range");
	return std::max(1, width >> level);
}

void Texture1D::Map(TextureMapAccess tma, void *& data,const Box1D& box)
{
	auto subres = CalcSubresource(box.level, box.array_index, 0, mipmap_size, array_size);

	uint32 row_pitch;
	uint32 slice_pitch;
	DoMap(format,subres, tma, box.x_offset, 0, 0, 1, 1, data, row_pitch, slice_pitch);
}

void Texture1D::UnMap(const Sub1D& box)
{
	auto subres = CalcSubresource(box.level, box.array_index, 0, mipmap_size, array_size);
	DoUnmap(subres);
}

ViewSimulation * Texture1D::RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels)
{
	LAssert(GetAccessMode() & EA_GPURead, "Access mode must have EA_GPURead flag");
	D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	switch (format) {
	case EF_D16:
		desc.Format = DXGI_FORMAT_R16_UNORM;
		break;
	case EF_D24S8:
		desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case EF_D32F:
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	default:
		desc.Format = dxgi_format;
	}

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (GetArraySize() > 1)
	{
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MostDetailedMip = first_level;
		desc.Texture1DArray.MipLevels = num_levels;
		desc.Texture1DArray.ArraySize = num_items;
		desc.Texture1DArray.FirstArraySlice = first_array_index;
		desc.Texture1DArray.ResourceMinLODClamp = 0;
	}
	else
	{
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MostDetailedMip = first_level;
		desc.Texture1D.MipLevels = num_levels;
		desc.Texture1D.ResourceMinLODClamp = 0;
	}

	return RetriveSRV(desc);
}

ViewSimulation * Texture1D::RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUUnordered, "Access mode must have EA_GPUUnordered flag");

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;

	desc.Format = dxgi_format;

	if (GetArraySize() > 1) {
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MipSlice = level;
		desc.Texture1DArray.ArraySize = num_items;
		desc.Texture1DArray.FirstArraySlice = first_array_index;
	}
	else {
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = level;
	}

	return RetriveUAV(desc);
}

ViewSimulation * Texture1D::RetriveRenderTargetView(uint8 first_array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_RENDER_TARGET_VIEW_DESC desc;

	desc.Format = Convert(format);
	if (GetArraySize() > 1) {
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MipSlice = level;
		desc.Texture1DArray.ArraySize = array_size;
		desc.Texture1DArray.FirstArraySlice = first_array_index;
	}
	else {
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = level;
	}

	return RetriveRTV(desc);
}

ViewSimulation * Texture1D::RetriveDepthStencilView(uint8 first_array_index, uint16 first_slice, uint16 num_slices, uint8 level)
{
	LAssert(GetAccessMode() & EA_GPUWrite, "Access mode must have EA_GPUWrite flag");

	D3D12_DEPTH_STENCIL_VIEW_DESC desc;

	desc.Format = Convert(format);
	desc.Flags = D3D12_DSV_FLAG_NONE;

	if (GetArraySize() > 1) {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
		desc.Texture1DArray.MipSlice = level;
		desc.Texture1DArray.ArraySize = array_size;
		desc.Texture1DArray.FirstArraySlice = first_array_index;
	}
	else {
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = level;
	}

	return RetriveDSV(desc);
}
