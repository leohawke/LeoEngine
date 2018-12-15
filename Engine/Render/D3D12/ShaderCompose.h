/*! \file Engine\Render\D3D12\ShaderCompose.h
\ingroup Engine
\brief 一系列Shader组合。
*/
#ifndef LE_RENDER_D3D12_ShaderCompose_h
#define LE_RENDER_D3D12_ShaderCompose_h 1

#include "../Effect/Effect.hpp"
#include "d3d12_dxgi.h"
#include "ResourceView.h"
#include <optional>


namespace asset {
	class ShaderBlobAsset;
}

namespace platform_ex::Windows::D3D12 {

	using ShaderInfo = platform::Render::ShaderInfo;

	class ShaderCompose:public platform::Render::ShaderCompose
	{
	public:
		using base = platform::Render::ShaderCompose;
	private:
		struct Template
		{
			Template();
			~Template();

			leo::observer_ptr<ID3D12RootSignature> root_signature;
			COMPtr<ID3D12DescriptorHeap> sampler_heap;

			using ShaderBlobEx = base::ShaderBlob;

			union {
				struct {
					std::optional<ShaderBlobEx> VertexShader;
					std::optional<ShaderBlobEx> PixelShader;
				};
				std::array<std::optional<ShaderBlobEx>, NumTypes> Shaders;
			};

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

		ShaderCompose(std::unordered_map<ShaderCompose::Type, leo::observer_ptr<const asset::ShaderBlobAsset>> pShaderBlob, leo::observer_ptr<platform::Render::Effect::Effect> pEffect);

		 void Bind() override;
		 void UnBind() override;

		 const std::optional<Template::ShaderBlobEx>& GetShaderBlob(Type shader_type) const;

	public:
		ID3D12RootSignature* RootSignature() const;

		ID3D12DescriptorHeap* SamplerHeap() const;
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
		using ShaderResourceViewSimulation = ViewSimulation;
		std::array<std::vector<ShaderResourceViewSimulation*>, NumTypes> Srvs;
		using UnorderedAccessViewSimulation = ViewSimulation;
		std::array<std::vector<UnorderedAccessViewSimulation*>, NumTypes> Uavs;
		std::array<std::vector<platform::Render::GraphicsBuffer*>, NumTypes> CBuffs;
		std::array<std::vector<std::tuple<ResourceHolder*, uint32_t, uint32_t>>, NumTypes> SrvSrcs;
		std::array<std::vector<D3D12_SAMPLER_DESC>, NumTypes> Samplers;

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