/*! \file Engine\Render\D3D12\ShaderCompose.h
\ingroup Engine
\brief 绘制创建封装。
*/
#ifndef LE_RENDER_D3D12_ShaderCompose_h
#define LE_RENDER_D3D12_ShaderCompose_h 1

#include "../Effect/Effect.hpp"
#include "d3d12_dxgi.h"
#include <optional>

namespace platform_ex::Windows::D3D12 {

	class ShaderCompose:public platform::Render::ShaderCompose
	{
	public:


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
		COMPtr<ID3D12RootSignature> root_signature;
		COMPtr<ID3D12DescriptorHeap> sampler_heap;

		std::vector<D3D12_RESOURCE_BARRIER> barriers;
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