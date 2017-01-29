#include "ShaderCompose.h"
#include "Context.h"

using namespace platform_ex::Windows;

void platform_ex::Windows::D3D12::ShaderCompose::Bind()
{
	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	for (leo::uint8 st = 0; st != NumTypes; ++st) {
		//param bind
	}

	if (!barriers.empty()) {
		D3D12::Context::Instance().GetCommandList(D3D12::Device::Command_Render)
			->ResourceBarrier(static_cast<UINT>(barriers.size()),barriers.data());
	}


	SwapAndPresent();
		
	//update cbuffer
}

void platform_ex::Windows::D3D12::ShaderCompose::UnBind()
{
	SwapAndPresent();
}

ID3D12RootSignature * platform_ex::Windows::D3D12::ShaderCompose::RootSignature() const
{
	return nullptr;
}

void platform_ex::Windows::D3D12::ShaderCompose::CreateRootSignature()
{

}

void platform_ex::Windows::D3D12::ShaderCompose::CreateBarriers()
{
	for (leo::uint8 st = 0; st != NumTypes; ++st) {

		D3D12_RESOURCE_BARRIER barrier_before;
		barrier_before.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_before.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_before.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		barrier_before.Transition.StateBefore
			= (Type::PixelShader == (Type)st) ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			: D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		//srv barrier

		barrier_before.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		barrier_before.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		//uav barrier
	}
}

void platform_ex::Windows::D3D12::ShaderCompose::SwapAndPresent()
{
	for (auto & barrier : barriers) {
		std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
	}

	if (!barriers.empty()) {
		D3D12::Context::Instance().GetCommandList(D3D12::Device::Command_Render)
			->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
	}
}

#include <UniversalDXSDK/d3dcompiler.h>
#ifdef LFL_Win64
#pragma comment(lib,"UniversalDXSDK/Lib/x64/d3dcompiler.lib")
#else
#pragma comment(lib,"UniversalDXSDK/Lib/x86/d3dcompiler.lib")
#endif

namespace platform_ex::Windows::D3D12 {
	platform::Render::ShaderInfo * ReflectDXBC(const ShaderCompose::ShaderBlob & blob,ShaderCompose::Type type)
	{
		auto pInfo = std::make_unique<ShaderInfo>(type);
		platform_ex::COMPtr<ID3D12ShaderReflection> pReflection;
		platform_ex::CheckHResult(D3DReflect(blob.first.get(), blob.second, IID_ID3D12ShaderReflection,reinterpret_cast<void**>(&pReflection)));

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
				pInfo->NumSamplers = std::max(pInfo->NumSamplers, BindPoint);
				break;

			case D3D_SIT_TEXTURE:
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_BYTEADDRESS:
				pInfo->NumSrvs = std::max(pInfo->NumSrvs, BindPoint);
				break;

			case D3D_SIT_UAV_RWTYPED:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				pInfo->NumUavs = std::max(pInfo->NumUavs, BindPoint);
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

		if (type == ShaderCompose::Type::VertexShader) {
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

		if (type == ShaderCompose::Type::ComputeShader) {
			UINT x, y, z;
			pReflection->GetThreadGroupSize(&x,&y,&z);
			pInfo->CSBlockSize = leo::math::data_storage<uint16, 3>(static_cast<uint16>(x),
				static_cast<uint16>(y), static_cast<uint16>(z));
		}

		return pInfo.release();
	}
}
