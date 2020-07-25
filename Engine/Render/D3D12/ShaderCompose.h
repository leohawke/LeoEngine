/*! \file Engine\Render\D3D12\ShaderCompose.h
\ingroup Engine
\brief 一系列Shader组合。
*/
#ifndef LE_RENDER_D3D12_ShaderCompose_h
#define LE_RENDER_D3D12_ShaderCompose_h 1

#include "../Effect/Effect.hpp"
#include "d3d12_dxgi.h"
#include "View.h"
#include "HardwareShader.h"
#include <optional>


namespace asset {
	class ShaderBlobAsset;
}

namespace platform_ex::Windows::D3D12 {

	using ShaderInfo = platform::Render::ShaderInfo;

	class RootSignature;

	class ShaderCompose:public platform::Render::ShaderCompose
	{
	public:
		using base = platform::Render::ShaderCompose;
	private:
		struct Template
		{
			Template();
			~Template();

			leo::observer_ptr<RootSignature> root_signature;
			COMPtr<ID3D12DescriptorHeap> sampler_heap;

			using ShaderBlobEx = platform::Render::ShaderBlob;

			VertexHWShader* VertexShader;
			PixelHWShader* PixelShader;

			union {
				struct {
					std::optional<ShaderInfo> VertexInfo;
					std::optional<ShaderInfo> PixelInfo;
				};
				std::array<ShaderInfo, NumTypes> Infos;
			};

			union {
				struct {
					std::optional<std::vector<uint8>> VertexIndices;
					std::optional<std::vector<uint8>> PixelIndices;
				};
				std::array<std::vector<uint8>, NumTypes> CBuffIndices;
			};

			//Todo Support
			std::vector<D3D12_SO_DECLARATION_ENTRY> so_decl;
			bool vs_so = false;
			bool ds_so = false;

			//When StreamOuput -> D3D12_SO_NO_RASTERIZED_STREAM
			uint32 rasterized_stream = 0;
			size_t vs_signature;
		};

	public:

		ShaderCompose(std::unordered_map<platform::Render::ShaderType, const asset::ShaderBlobAsset*> pShaderBlob, platform::Render::Effect::Effect* pEffect);

		 void Bind() override;
		 void UnBind() override;

		 const platform::Render::ShaderBlob* GetShaderBlob(platform::Render::ShaderType shader_type) const;


		 VertexHWShader* GetVertexShader() const;
		 PixelHWShader* GetPixelShader() const;
	public:
		using SignatureType = RootSignature;
		SignatureType* RootSignature() const;
	public:
	private:
		void CreateRootSignature();
		void CreateBarriers();

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
	public:
		using ShaderResourceViewSimulation = ShaderResourceView;
		std::array<std::vector<ShaderResourceViewSimulation*>, NumTypes> Srvs;
		using UnorderedAccessViewSimulation = UnorderedAccessView;
		std::array<std::vector<UnorderedAccessViewSimulation*>, NumTypes> Uavs;
		std::array<std::vector<platform::Render::GraphicsBuffer*>, NumTypes> CBuffs;
		std::array<std::vector<std::tuple<ResourceHolder*, uint32_t, uint32_t>>, NumTypes> SrvSrcs;
		std::array<std::vector<platform::Render::TextureSampleDesc>, NumTypes> Samplers;

	private:
		std::unique_ptr<Template> sc_template;

		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		std::array<std::vector<parameter_bind_t>, NumTypes> ParamBinds;
		std::array<std::vector<std::pair<ResourceHolder*, ID3D12Resource*>>, NumTypes> UavSrcs;

		std::vector<leo::observer_ptr<platform::Render::Effect::ConstantBuffer>> AllCBuffs;

		friend class PipleState;
		friend class Texture;
		friend class Context;
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

	inline void operator<<(D3D12_SHADER_BYTECODE& desc, const platform::Render::ShaderBlob& blob)
	{
		desc.BytecodeLength = blob.second;
		desc.pShaderBytecode = blob.first.get();
	}

	inline void operator<<(D3D12_SHADER_BYTECODE& desc, const D3D12HardwareShader* shader)
	{
		if (!shader)
			desc << nullptr;
		else {
			desc << shader->ShaderByteCode;
		}
	}
}

#endif