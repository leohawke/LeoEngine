/*! \file Engine\Render\D3D12\ShaderCompose.h
\ingroup Engine
\brief 绘制创建封装。
*/
#ifndef LE_RENDER_D3D12_ShaderCompose_h
#define LE_RENDER_D3D12_ShaderCompose_h 1

#include "../Effect/Effect.hpp"
#include "d3d12_dxgi.h"
#include "RenderView.h"
#include <optional>

namespace asset {
	class ShaderBlobAsset;
}

namespace platform_ex::Windows::D3D12 {

	using ShaderInfo = platform::Render::ShaderInfo;

	class ShaderCompose:public platform::Render::ShaderCompose
	{
	public:
		ShaderCompose(std::unordered_map<ShaderCompose::Type, leo::observer_ptr<const asset::ShaderBlobAsset>> pShaderBlob, leo::observer_ptr<platform::Render::Effect::Effect> pEffect);

		 void Bind() override;
		 void UnBind() override;

	public:
		ID3D12RootSignature* RootSignature() const;

	public:
		std::optional<ShaderBlob> VertexShader;
		std::optional<ShaderBlob> PixelShader;
	private:
		void CreateRootSignature();
		void CreateBarriers();
		void SwapAndPresent();

	private:
		struct ShaderParameterHandle
		{
			uint32_t shader_type;

			D3D_SHADER_VARIABLE_TYPE param_type;

			uint32_t cbuff;

			uint32_t offset;
			uint32_t elements;
			uint8_t rows;
			uint8_t columns;
		};

		struct parameter_bind_t
		{
			platform::Render::Effect::Parameter* param;
			ShaderParameterHandle p_handle;
			std::function<void()> func;
		};
	private:
		parameter_bind_t GetBindFunc(ShaderParameterHandle const & p_handle,platform::Render::Effect::Parameter* param);
	private:

		COMPtr<ID3D12RootSignature> root_signature;
		COMPtr<ID3D12DescriptorHeap> sampler_heap;

		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		std::array<std::vector<parameter_bind_t>, NumTypes> ParamBinds;
		std::array<std::vector<D3D12_SAMPLER_DESC>, NumTypes> Samplers;
		std::array<std::vector<std::tuple<ID3D12Resource*, uint32_t, uint32_t>>, NumTypes> SrvSrcs;
		using ShaderResourceViewSimulation = ViewSimulation;
		std::array<std::vector<ShaderResourceViewSimulation*>, NumTypes> Srvs;
		std::array<std::vector<std::pair<ID3D12Resource*, ID3D12Resource*>>, NumTypes> UavSrcs;
		using UnorderedAccessViewSimulation = ViewSimulation;
		std::array<std::vector<UnorderedAccessViewSimulation*>, NumTypes> Uavs;
		std::array<std::vector<platform::Render::GraphicsBuffer*>, NumTypes> CBuffs;

		std::vector<leo::observer_ptr<platform::Render::Effect::ConstantBuffer>> AllCBuffs;
	};

	inline void operator<<(D3D12_SHADER_BYTECODE& desc, std::nullptr_t)
	{
		desc.BytecodeLength = 0;
		desc.pShaderBytecode = nullptr;
	}

	inline void operator<<(D3D12_STREAM_OUTPUT_DESC& desc, std::nullptr_t)
	{
		desc.pSODeclaration = nullptr;
		desc.NumEntries = 0;
		desc.pBufferStrides = nullptr;
		desc.NumStrides = 0;
		desc.RasterizedStream = 0;
	}

	inline void operator<<(D3D12_SHADER_BYTECODE& desc, const std::optional<ShaderCompose::ShaderBlob>& blob)
	{
		if (!blob)
			desc << nullptr;
		else {
			desc.BytecodeLength = blob->second;
			desc.pShaderBytecode = blob->first.get();
		}
	}

}

#endif