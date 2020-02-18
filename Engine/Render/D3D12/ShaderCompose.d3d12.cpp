#include "ShaderCompose.h"
#include "Context.h"
#include "../Effect/Effect.hpp"
#include "../../Asset/EffectAsset.h"
#include "../ITexture.hpp"
#include "Texture.h"
#include "Convert.h"
#include "RootSignature.h"

using namespace platform_ex::Windows;
using platform::Render::ShaderType;
using platform::Render::ShaderBlob;

namespace {
	class SetTextureSRV {
	public:
		SetTextureSRV(std::tuple<D3D12::ResourceHolder*, leo::uint32, leo::uint32>&srvsrc_, D3D12::ViewSimulation*& srv_, platform::Render::Effect::Parameter * param_)
			:psrvsrc(&srvsrc_), ppsrv(&srv_), param(param_)
		{}

		void operator()() {
			platform::Render::TextureSubresource tex_subres;
			try {
				param->Value(tex_subres);
			}
			catch (leo::bad_any_cast&) {
				LF_Trace(platform::Descriptions::RecordLevel::Warning, "SetTextureSRV(%s) Value Null!", param->Name.c_str());
			}
			if (tex_subres.tex) {
				auto pTexture = dynamic_cast<D3D12::Texture*>(tex_subres.tex.get());
				*psrvsrc = std::make_tuple(static_cast<D3D12::ResourceHolder*>(pTexture),
					tex_subres.first_array_index * tex_subres.tex->GetNumMipMaps() + tex_subres.first_level,

					tex_subres.num_items * tex_subres.num_levels);

				*ppsrv = pTexture->RetriveShaderResourceView(
					tex_subres.first_array_index, tex_subres.num_items,

					tex_subres.first_level, tex_subres.num_levels);
			}
			else {
				std::get<0>(*psrvsrc) = nullptr;
			}
		}
	private:
		std::tuple<D3D12::ResourceHolder*, leo::uint32, leo::uint32>* psrvsrc;
		D3D12::ViewSimulation** ppsrv;
		platform::Render::Effect::Parameter * param;
	};

	class SetBufferSRV {
	public:
		SetBufferSRV(std::tuple<D3D12::ResourceHolder*, leo::uint32, leo::uint32>&srvsrc_, D3D12::ViewSimulation*& srv_, platform::Render::Effect::Parameter * param_)
			:psrvsrc(&srvsrc_), ppsrv(&srv_), param(param_)
		{}
		void operator()() {
			std::shared_ptr<platform::Render::GraphicsBuffer> buffer;
			try {
				param->Value(buffer);
			}
			catch (leo::bad_any_cast&) {
				LF_Trace(platform::Descriptions::RecordLevel::Warning, "SetBufferSRV(%s) Value Null!", param->Name.c_str());
			}
			if (buffer) {
				auto pBuffer = static_cast<D3D12::GraphicsBuffer*>(buffer.get());
				*psrvsrc = std::make_tuple(pBuffer, 0, 1);
				*ppsrv = pBuffer->RetriveShaderResourceView();
			}
			else {
				std::get<0>(*psrvsrc) = nullptr;
			}
		}
	private:
		std::tuple<D3D12::ResourceHolder*, leo::uint32, leo::uint32>* psrvsrc;
		D3D12::ViewSimulation** ppsrv;
		platform::Render::Effect::Parameter * param;
	};

	class SetTextureUAV {
	public:
		SetTextureUAV(std::pair<D3D12::ResourceHolder*, ID3D12Resource*> uavsrc_, D3D12::ViewSimulation*& uav_, platform::Render::Effect::Parameter * param_)
			:puavsrc(&uavsrc_), ppuav(&uav_), param(param_)
		{}

		void operator()() {
			platform::Render::TextureSubresource tex_subres;
			param->Value(tex_subres);
			if (tex_subres.tex) {
				auto pTexture = dynamic_cast<D3D12::Texture*>(tex_subres.tex.get());
				puavsrc->first = pTexture;
				puavsrc->second = nullptr;

				*ppuav = pTexture->RetriveUnorderedAccessView(
					tex_subres.first_array_index, tex_subres.num_items,

					tex_subres.first_level, tex_subres.num_levels);
			}
			else {
				puavsrc->first = nullptr;
				puavsrc->second = nullptr;
			}
		}
	private:
		std::pair<D3D12::ResourceHolder*, ID3D12Resource*>* puavsrc;
		D3D12::ViewSimulation** ppuav;
		platform::Render::Effect::Parameter * param;
	};

	class SetBufferUAV {
	public:
		SetBufferUAV(std::pair<D3D12::ResourceHolder*, ID3D12Resource*> uavsrc_, D3D12::ViewSimulation*& uav_, platform::Render::Effect::Parameter * param_)
			:puavsrc(&uavsrc_), ppuav(&uav_), param(param_)
		{}

		void operator()() {
			std::shared_ptr<platform::Render::GraphicsBuffer> buffer;
			param->Value(buffer);
			if (buffer) {
				auto pBuffer = static_cast<D3D12::GraphicsBuffer*>(buffer.get());
				puavsrc->first = pBuffer;
				puavsrc->second = pBuffer->UploadResource();

				*ppuav = pBuffer->RetriveUnorderedAccessView();
			}
			else {
				puavsrc->first = nullptr;
				puavsrc->second = nullptr;
			}
		}
	private:
		std::pair<D3D12::ResourceHolder*, ID3D12Resource*>* puavsrc;
		D3D12::ViewSimulation** ppuav;
		platform::Render::Effect::Parameter * param;
	};
}

platform_ex::Windows::D3D12::ShaderCompose::Template::~Template() = default;

platform_ex::Windows::D3D12::ShaderCompose::Template::Template()
	:VertexShader(std::nullopt), PixelShader(std::nullopt), VertexInfo(std::nullopt),
	PixelInfo(std::nullopt), VertexIndices(std::nullopt), PixelIndices(std::nullopt) {
	//uname union uname struct init
}

platform_ex::Windows::D3D12::ShaderCompose::ShaderCompose(std::unordered_map<platform::Render::ShaderType, leo::observer_ptr<const asset::ShaderBlobAsset>> pShaderBlob, leo::observer_ptr<platform::Render::Effect::Effect> pEffect) :
	sc_template(std::make_unique<Template>())
{
	auto CopyShader = [&](auto& shader, auto type) {
		if (pShaderBlob.count(type)) {
			auto pBlobAsset = pShaderBlob[type];
			shader = std::make_pair(std::make_unique<byte[]>(pBlobAsset->GetBlob().second), pBlobAsset->GetBlob().second);
			std::memcpy(shader.value().first.get(), pBlobAsset->GetBlob().first.get(), pBlobAsset->GetBlob().second);
		}
	};

	CopyShader(sc_template->VertexShader, ShaderType::VertexShader);
	CopyShader(sc_template->PixelShader, ShaderType::PixelShader);

	if (sc_template->VertexShader.has_value())
		sc_template->vs_signature = pShaderBlob[ShaderType::VertexShader]->GetInfo().InputSignature.value();

	for (auto& pair : pShaderBlob) {
		auto index = static_cast<leo::uint8>(pair.first);
		auto& BlobInfo = pair.second->GetInfo();

		sc_template->Infos[index] = BlobInfo;

		for (auto& ConstantBufferInfo : BlobInfo.ConstantBufferInfos) {
			auto cbindex = pEffect->ConstantBufferIndex(ConstantBufferInfo.name_hash);
			auto& ConstantBuffer = pEffect->GetConstantBuffer(cbindex);
			AllCBuffs.emplace_back(&ConstantBuffer);
			CBuffs[index].emplace_back(ConstantBuffer.GetGraphicsBuffer());

			sc_template->CBuffIndices[index].emplace_back(static_cast<uint8>(index));
		}

		Samplers[index].resize(BlobInfo.ResourceCounts.NumSamplers);
		SrvSrcs[index].resize(BlobInfo.ResourceCounts.NumSRVs, std::make_tuple(static_cast<ResourceHolder*>(nullptr), 0, 0));
		Srvs[index].resize(BlobInfo.ResourceCounts.NumSRVs);
		UavSrcs[index].resize(BlobInfo.ResourceCounts.NumUAVs, std::make_pair<ResourceHolder*, ID3D12Resource*>(nullptr, nullptr));
		Uavs[index].resize(BlobInfo.ResourceCounts.NumUAVs);

		for (auto& BoundResourceInfo : BlobInfo.BoundResourceInfos) {
			auto& Parameter = pEffect->GetParameter(BoundResourceInfo.name);

			ShaderParameterHandle p_handle;
			p_handle.shader_type = index;
			p_handle.cbuff = 0;
			p_handle.offset = BoundResourceInfo.bind_point;
			p_handle.elements = 1;
			p_handle.rows = 0;
			p_handle.columns = 1;
			if (D3D_SIT_SAMPLER == BoundResourceInfo.type)
			{
				p_handle.param_type = D3D_SVT_SAMPLER;
				platform::Render::SamplerDesc sampler_desc;
				Parameter.Value(sampler_desc);
				Samplers[p_handle.shader_type][p_handle.offset] = Convert(sampler_desc);
			}
			else
			{
				if (D3D_SRV_DIMENSION_BUFFER == BoundResourceInfo.dimension)
				{
					p_handle.param_type = D3D_SVT_BUFFER;
				}
				else
				{
					p_handle.param_type = D3D_SVT_TEXTURE;
				}

				ParamBinds[index].emplace_back(GetBindFunc(p_handle, &Parameter));
			}
		}
	}

	AllCBuffs.erase(std::unique(AllCBuffs.begin(), AllCBuffs.end()), AllCBuffs.end());

	CreateRootSignature();
}

void platform_ex::Windows::D3D12::ShaderCompose::Bind()
{
	for (leo::uint8 st = 0; st != NumTypes; ++st) {
		//param bind
		for (auto const & pb : ParamBinds[st]) {
			pb.func();
		}
	}
	CreateBarriers();
	if (!barriers.empty()) {
		D3D12::Context::Instance().GetCommandList(D3D12::Device::Command_Render)
			->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
	}

	//update cbuffer
	for (auto i = 0; i != AllCBuffs.size(); ++i) {
		AllCBuffs[i]->Update();
	}
}

void platform_ex::Windows::D3D12::ShaderCompose::UnBind()
{
}

const std::optional<platform_ex::Windows::D3D12::ShaderCompose::Template::ShaderBlobEx>& platform_ex::Windows::D3D12::ShaderCompose::GetShaderBlob(ShaderType shader_type) const
{
	return sc_template->Shaders[static_cast<uint8>(shader_type)];
}

ID3D12RootSignature * platform_ex::Windows::D3D12::ShaderCompose::RootSignature() const
{
	return sc_template->root_signature->Signature.Get();
}

ID3D12DescriptorHeap * platform_ex::Windows::D3D12::ShaderCompose::SamplerHeap() const
{
	return sc_template->sampler_heap.Get();
}

void platform_ex::Windows::D3D12::ShaderCompose::CreateRootSignature()
{
	QuantizedBoundShaderState QBSS;

	QBSS.AllowIAInputLayout = sc_template->VertexShader.has_value();
	QBSS.AllowStreamOuput = false;

	size_t num_sampler = 0;
	for (auto i = 0; i != NumTypes; ++i) {
		QBSS.RegisterCounts[i].NumCBs = CBuffs[i].size();
		QBSS.RegisterCounts[i].NumSRVs = Srvs[i].size();
		QBSS.RegisterCounts[i].NumUAVs = Uavs[i].size();
		QBSS.RegisterCounts[i].NumSamplers = Samplers[i].size();

		num_sampler += QBSS.RegisterCounts[i].NumSamplers;
	}

	auto& Device = Context::Instance().GetDevice();
	sc_template->root_signature = Device.CreateRootSignature(QBSS);

	if (num_sampler > 0) {
		D3D12_DESCRIPTOR_HEAP_DESC sampler_heap_desc;
		sampler_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		sampler_heap_desc.NumDescriptors = static_cast<UINT>(num_sampler);
		sampler_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		sampler_heap_desc.NodeMask = 0;
		CheckHResult(Device->CreateDescriptorHeap(&sampler_heap_desc, COMPtr_RefParam(sc_template->sampler_heap, IID_ID3D12DescriptorHeap)));

		auto sampler_desc_size = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		auto cpu_sampler_handle = sc_template->sampler_heap->GetCPUDescriptorHandleForHeapStart();
		for (auto i = 0; i != NumTypes; ++i) {
			for (auto j = 0; j != Samplers[i].size(); ++j) {
				Device->CreateSampler(&Samplers[i][j], cpu_sampler_handle);
				cpu_sampler_handle.ptr += sampler_desc_size;
			}
		}
	}
}

void platform_ex::Windows::D3D12::ShaderCompose::CreateBarriers()
{
	barriers.clear();
	for (leo::uint8 st = 0; st != NumTypes; ++st) {

		D3D12_RESOURCE_BARRIER barrier_before;
		barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		//srv barrier
		for (auto j = 0; j != SrvSrcs[st].size(); ++j) {
			for (auto subres = 0; subres != std::get<2>(SrvSrcs[st][j]); ++subres) {
				auto pResourceHolder = std::get<0>(SrvSrcs[st][j]);
				if (pResourceHolder != nullptr) {
					if (pResourceHolder->UpdateResourceBarrier(barrier_before, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)) {
						barriers.emplace_back(barrier_before);
					}
				}
			}
		}

		//uav barrier
		for (auto j = 0; j != UavSrcs[st].size(); ++j) {
			auto pResourceHolder = UavSrcs[st][j].first;
			if (pResourceHolder != nullptr) {
				LAssert(std::find_if(SrvSrcs[st].begin(), SrvSrcs[st].end(), [&](auto& tuple) {return std::get<0>(tuple) == pResourceHolder; }) == SrvSrcs[st].end(), "Resource Input&Ouput !!!");

				if (pResourceHolder->UpdateResourceBarrier(barrier_before, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)) {
					barriers.emplace_back(barrier_before);
				}
			}
		}
	}
}

platform_ex::Windows::D3D12::ShaderCompose::parameter_bind_t platform_ex::Windows::D3D12::ShaderCompose::GetBindFunc(ShaderParameterHandle const & p_handle, platform::Render::Effect::Parameter * param)
{
	using platform::Render::EffectParamType;

	parameter_bind_t ret;
	ret.param = param;
	std::memcpy(&ret.p_handle, &p_handle, sizeof(p_handle));


	switch (param->GetType())
	{
	case EffectParamType::EPT_texture1D:
	case EffectParamType::EPT_texture2D:
	case EffectParamType::EPT_texture3D:
	case EffectParamType::EPT_textureCUBE:
	case EffectParamType::EPT_texture1DArray:
	case EffectParamType::EPT_texture2DArray:
	case EffectParamType::EPT_texture3DArray:
	case EffectParamType::EPT_textureCUBEArray:
		ret.func = ::SetTextureSRV(SrvSrcs[p_handle.shader_type][p_handle.offset], Srvs[p_handle.shader_type][p_handle.offset], param);
		break;

	case EffectParamType::EPT_buffer:
	case EffectParamType::EPT_StructuredBuffer:
	case EffectParamType::EPT_ConsumeStructuredBuffer:
	case EffectParamType::EPT_AppendStructuredBuffer:
	case EffectParamType::EPT_byteAddressBuffer:
		ret.func = ::SetBufferSRV(SrvSrcs[p_handle.shader_type][p_handle.offset], Srvs[p_handle.shader_type][p_handle.offset], param);
		break;


	case EffectParamType::EPT_rwtexture1D:
	case EffectParamType::EPT_rwtexture2D:
	case EffectParamType::EPT_rwtexture3D:
	case EffectParamType::EPT_rwtexture1DArray:
	case EffectParamType::EPT_rwtexture2DArray:
		ret.func = ::SetTextureUAV(UavSrcs[p_handle.shader_type][p_handle.offset], Uavs[p_handle.shader_type][p_handle.offset], param);
		break;

	case EffectParamType::EPT_rwbuffer:
	case EffectParamType::EPT_rwstructured_buffer:
	case EffectParamType::EPT_rwbyteAddressBuffer:
		ret.func = ::SetBufferUAV(UavSrcs[p_handle.shader_type][p_handle.offset], Uavs[p_handle.shader_type][p_handle.offset], param);
		break;
	}

	return ret;
}

#include <UniversalDXSDK/d3dcompiler.h>
#ifdef LFL_Win64
#pragma comment(lib,"UniversalDXSDK/Lib/x64/d3dcompiler.lib")
#else
#pragma comment(lib,"UniversalDXSDK/Lib/x86/d3dcompiler.lib")
#endif


namespace platform_ex::Windows::D3D12 {
	platform::Render::ShaderInfo * ReflectDXBC(const ShaderBlob & blob, ShaderType type)
	{
		auto pInfo = std::make_unique<ShaderInfo>(type);
		platform_ex::COMPtr<ID3D12ShaderReflection> pReflection;
		platform_ex::CheckHResult(D3DReflect(blob.first.get(), blob.second, IID_ID3D12ShaderReflection, reinterpret_cast<void**>(&pReflection)));

		D3D12_SHADER_DESC desc;
		pReflection->GetDesc(&desc);

		for (UINT i = 0; i != desc.ConstantBuffers; ++i) {
			auto pReflectionConstantBuffer = pReflection->GetConstantBufferByIndex(i);

			D3D12_SHADER_BUFFER_DESC buffer_desc;
			pReflectionConstantBuffer->GetDesc(&buffer_desc);
			if ((D3D_CT_CBUFFER == buffer_desc.Type) || (D3D_CT_TBUFFER == buffer_desc.Type)) {
				ShaderInfo::ConstantBufferInfo  ConstantBufferInfo;
				ConstantBufferInfo.name = buffer_desc.Name;
				ConstantBufferInfo.name_hash = leo::constfn_hash(buffer_desc.Name);
				ConstantBufferInfo.size = buffer_desc.Size;

				for (UINT v = 0; v != buffer_desc.Variables; ++v) {
					auto pReflectionVar = pReflectionConstantBuffer->GetVariableByIndex(v);
					D3D12_SHADER_VARIABLE_DESC variable_desc;
					pReflectionVar->GetDesc(&variable_desc);

					D3D12_SHADER_TYPE_DESC type_desc;
					pReflectionVar->GetType()->GetDesc(&type_desc);

					ShaderInfo::ConstantBufferInfo::VariableInfo VariableInfo;
					VariableInfo.name = variable_desc.Name;
					VariableInfo.start_offset = variable_desc.StartOffset;
					VariableInfo.type = variable_desc.StartOffset;
					VariableInfo.rows = variable_desc.StartOffset;
					VariableInfo.columns = variable_desc.StartOffset;
					VariableInfo.elements = variable_desc.StartOffset;

					ConstantBufferInfo.var_desc.emplace_back(std::move(VariableInfo));
				}
				pInfo->ConstantBufferInfos.emplace_back(std::move(ConstantBufferInfo));
			}
		}

		for (UINT i = 0; i != desc.BoundResources; ++i) {
			D3D12_SHADER_INPUT_BIND_DESC input_bind_desc;
			pReflection->GetResourceBindingDesc(i, &input_bind_desc);

			auto BindPoint = static_cast<uint16>(input_bind_desc.BindPoint + 1);
			switch (input_bind_desc.Type)
			{
			case D3D_SIT_SAMPLER:
				pInfo->ResourceCounts.NumSamplers = std::max(pInfo->ResourceCounts.NumSamplers, BindPoint);
				break;

			case D3D_SIT_TEXTURE:
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_BYTEADDRESS:
				pInfo->ResourceCounts.NumSRVs = std::max(pInfo->ResourceCounts.NumSRVs, BindPoint);
				break;

			case D3D_SIT_UAV_RWTYPED:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				pInfo->ResourceCounts.NumUAVs = std::max(pInfo->ResourceCounts.NumUAVs, BindPoint);
				break;

			default:
				break;
			}

			switch (input_bind_desc.Type)
			{
			case D3D_SIT_TEXTURE:
			case D3D_SIT_SAMPLER:
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_BYTEADDRESS:
			case D3D_SIT_UAV_RWTYPED:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			{
				ShaderInfo::BoundResourceInfo BoundResourceInfo;
				BoundResourceInfo.name = input_bind_desc.Name;
				BoundResourceInfo.type = static_cast<uint8_t>(input_bind_desc.Type);
				BoundResourceInfo.bind_point = static_cast<uint16_t>(input_bind_desc.BindPoint);
				pInfo->BoundResourceInfos.emplace_back(std::move(BoundResourceInfo));
			}
			break;

			default:
				break;
			}
		}

		if (type == ShaderType::VertexShader) {
			union {
				D3D12_SIGNATURE_PARAMETER_DESC signature_desc;
				stdex::byte signature_data[sizeof(D3D12_SIGNATURE_PARAMETER_DESC)];
			} s2d;

			size_t signature = 0;
			for (UINT i = 0; i != desc.InputParameters; ++i) {
				pReflection->GetInputParameterDesc(i, &s2d.signature_desc);
				auto seed = leo::hash(s2d.signature_data);
				leo::hash_combine(signature, seed);
			}

			pInfo->InputSignature = signature;
		}

		if (type == ShaderType::ComputeShader) {
			UINT x, y, z;
			pReflection->GetThreadGroupSize(&x, &y, &z);
			pInfo->CSBlockSize = leo::math::data_storage<uint16, 3>(static_cast<uint16>(x),
				static_cast<uint16>(y), static_cast<uint16>(z));
		}

		return pInfo.release();
	}
}
