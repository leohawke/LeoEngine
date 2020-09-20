#include <LFramework/LCLib/Logger.h>

#include "../Effect/Effect.hpp"
#include "../../Asset/EffectAsset.h"
#include "../ITexture.hpp"

#include "View.h"
#include "Context.h"
#include "ShaderCompose.h"
#include "Texture.h"
#include "Convert.h"
#include "RootSignature.h"

using namespace platform_ex::Windows;
using platform::Render::ShaderType;
using platform::Render::ShaderBlob;

namespace {
	class SetTextureSRV {
	public:
		SetTextureSRV(std::tuple<D3D12::ResourceHolder*, leo::uint32, leo::uint32>& srvsrc_, D3D12::ShaderResourceView*& srv_, platform::Render::Effect::Parameter* param_)
			:psrvsrc(&srvsrc_), ppsrv(&srv_), param(param_)
		{}

		void operator()() {
			auto pTexSubRes = param->TryGetValue<platform::Render::TextureSubresource>();
			if (!pTexSubRes)
			{
				LF_Trace(platform::Descriptions::RecordLevel::Warning, "SetTextureSRV(%s) Value Null!", param->Name.c_str());
			}
			else if (pTexSubRes->tex) {
				auto& tex_subres = *pTexSubRes;

				auto pTexture = D3D12::dynamic_cast_texture(tex_subres.tex.get());
				*psrvsrc = std::make_tuple(static_cast<D3D12::ResourceHolder*>(pTexture),
					tex_subres.first_array_index * tex_subres.tex->GetNumMipMaps() + tex_subres.first_level,

					tex_subres.num_items * tex_subres.num_levels);

				auto Desc = D3D12::CreateSRVDesc(*tex_subres.tex, tex_subres.first_array_index, tex_subres.num_items,

					tex_subres.first_level, tex_subres.num_levels);

				auto pDefaultSRV = pTexture->RetriveShaderResourceView();

				if (memcmp(&pDefaultSRV->GetDesc(), &Desc, sizeof(Desc)) == 0)
				{
					*ppsrv = pDefaultSRV;
					return;
				}

				bool bReset = psrv == nullptr;

				if (!bReset)
				{
					auto PrevDesc = psrv->GetDesc();

					if (memcmp(&PrevDesc, &Desc, sizeof(Desc)) != 0 || pTexture != psrv->GetResourceLocation())
					{
						bReset = true;
					}
				}

				if (bReset)
				{
					psrv.reset(new D3D12::ShaderResourceView(D3D12::GetDefaultNodeDevice(), Desc, *pTexture));

					*ppsrv = psrv.get();
				}
			}
			else {
				std::get<0>(*psrvsrc) = nullptr;
			}
		}
	private:
		std::tuple<D3D12::ResourceHolder*, leo::uint32, leo::uint32>* psrvsrc;
		D3D12::ShaderResourceView** ppsrv;
		std::shared_ptr<D3D12::ShaderResourceView> psrv;
		platform::Render::Effect::Parameter* param;
	};

	class SetBufferSRV {
	public:
		SetBufferSRV(std::tuple<D3D12::ResourceHolder*, leo::uint32, leo::uint32>& srvsrc_, D3D12::ShaderResourceView*& srv_, platform::Render::Effect::Parameter* param_)
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
		D3D12::ShaderResourceView** ppsrv;
		platform::Render::Effect::Parameter* param;
	};

	class SetTextureUAV {
	public:
		SetTextureUAV(std::pair<D3D12::ResourceHolder*, ID3D12Resource*>& uavsrc_, D3D12::UnorderedAccessView*& uav_, platform::Render::Effect::Parameter* param_)
			:puavsrc(&uavsrc_), ppuav(&uav_), param(param_)
		{}

		void operator()() {
			platform::Render::TextureSubresource tex_subres;
			param->Value(tex_subres);
			if (tex_subres.tex) {
				auto pTexture = dynamic_cast<D3D12::Texture*>(tex_subres.tex.get());
				puavsrc->first = pTexture;
				puavsrc->second = nullptr;

				auto Desc = D3D12::CreateUAVDesc(*tex_subres.tex, tex_subres.first_array_index, tex_subres.num_items,

					tex_subres.first_level, tex_subres.num_levels);

				puav.reset(new D3D12::UnorderedAccessView(D3D12::GetDefaultNodeDevice(), Desc, *pTexture));

				*ppuav = puav.get();
			}
			else {
				puavsrc->first = nullptr;
				puavsrc->second = nullptr;
			}
		}
	private:
		std::pair<D3D12::ResourceHolder*, ID3D12Resource*>* puavsrc;
		D3D12::UnorderedAccessView** ppuav;
		std::shared_ptr<D3D12::UnorderedAccessView> puav;
		platform::Render::Effect::Parameter* param;
	};

	class SetBufferUAV {
	public:
		SetBufferUAV(std::pair<D3D12::ResourceHolder*, ID3D12Resource*>& uavsrc_, D3D12::UnorderedAccessView*& uav_, platform::Render::Effect::Parameter* param_)
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
		D3D12::UnorderedAccessView** ppuav;
		platform::Render::Effect::Parameter* param;
	};
}

platform_ex::Windows::D3D12::ShaderCompose::Template::~Template() = default;

platform_ex::Windows::D3D12::ShaderCompose::Template::Template()
	:VertexShader(nullptr), PixelShader(nullptr), Infos(), CBuffIndices() {
	//uname union uname struct init
}

platform_ex::Windows::D3D12::ShaderCompose::ShaderCompose(std::unordered_map<platform::Render::ShaderType, const asset::ShaderBlobAsset*> pShaderBlob, platform::Render::Effect::Effect * pEffect) :
	sc_template(std::make_unique<Template>())
{
	auto InitShader = [&](auto& shader, auto type) {
		if (pShaderBlob.count(type)) {
			auto pBlobAsset = pShaderBlob[type];

			platform::Render::ShaderInitializer initializer;
			initializer.pBlob = &pBlobAsset->GetBlob();
			initializer.pInfo = &pBlobAsset->GetInfo();

			shader = new std::remove_reference_t<decltype(*shader)> (initializer);
		}
	};

	InitShader(sc_template->VertexShader, ShaderType::VertexShader);
	InitShader(sc_template->PixelShader, ShaderType::PixelShader);

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
				platform::Render::TextureSampleDesc sampler_desc;
				Parameter.Value(sampler_desc);
				Samplers[p_handle.shader_type][p_handle.offset] = sampler_desc;
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
		for (auto const& pb : ParamBinds[st]) {
			pb.func();
		}
	}

	//update cbuffer
	for (auto i = 0; i != AllCBuffs.size(); ++i) {
		AllCBuffs[i]->Update();
	}
}

void platform_ex::Windows::D3D12::ShaderCompose::UnBind()
{
}

const platform::Render::ShaderBlob* platform_ex::Windows::D3D12::ShaderCompose::GetShaderBlob(ShaderType shader_type) const
{
	switch (shader_type)
	{
	case platform::Render::Shader::VertexShader:
		if (sc_template->VertexShader)
			return &sc_template->VertexShader->ShaderByteCode;
		break;
	case platform::Render::Shader::PixelShader:
		if(sc_template->PixelShader)
			return &sc_template->PixelShader->ShaderByteCode;
		break;
	}
	return nullptr;
}

D3D12::VertexHWShader* D3D12::ShaderCompose::GetVertexShader() const
{
	return sc_template->VertexShader;
}

D3D12::PixelHWShader* D3D12::ShaderCompose::GetPixelShader() const
{
	return sc_template->PixelShader;
}

platform_ex::Windows::D3D12::ShaderCompose::SignatureType* platform_ex::Windows::D3D12::ShaderCompose::RootSignature() const
{
	return sc_template->root_signature.get();
}

void platform_ex::Windows::D3D12::ShaderCompose::CreateRootSignature()
{
	QuantizedBoundShaderState QBSS;

	QBSS.AllowIAInputLayout = !!sc_template->VertexShader;
	QBSS.AllowStreamOuput = false;

	size_t num_sampler = 0;
	for (auto i = 0; i != NumTypes; ++i) {
		QBSS.RegisterCounts[i].NumCBs = static_cast<leo::uint16>(CBuffs[i].size());
		QBSS.RegisterCounts[i].NumSRVs = static_cast<leo::uint16>(Srvs[i].size());
		QBSS.RegisterCounts[i].NumUAVs = static_cast<leo::uint16>(Uavs[i].size());
		QBSS.RegisterCounts[i].NumSamplers = static_cast<leo::uint16>(Samplers[i].size());

		num_sampler += QBSS.RegisterCounts[i].NumSamplers;
	}

	auto& Device = Context::Instance().GetDevice();
	sc_template->root_signature = Device.CreateRootSignature(QBSS);
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

platform_ex::Windows::D3D12::ShaderCompose::parameter_bind_t platform_ex::Windows::D3D12::ShaderCompose::GetBindFunc(ShaderParameterHandle const& p_handle, platform::Render::Effect::Parameter * param)
{
	using platform::Render::ShaderParamType;

	parameter_bind_t ret;
	ret.param = param;
	std::memcpy(&ret.p_handle, &p_handle, sizeof(p_handle));


	switch (param->GetType())
	{
	case ShaderParamType::SPT_texture1D:
	case ShaderParamType::SPT_texture2D:
	case ShaderParamType::SPT_texture3D:
	case ShaderParamType::SPT_textureCUBE:
	case ShaderParamType::SPT_texture1DArray:
	case ShaderParamType::SPT_texture2DArray:
	case ShaderParamType::SPT_texture3DArray:
	case ShaderParamType::SPT_textureCUBEArray:
		ret.func = ::SetTextureSRV(SrvSrcs[p_handle.shader_type][p_handle.offset], Srvs[p_handle.shader_type][p_handle.offset], param);
		break;

	case ShaderParamType::SPT_buffer:
	case ShaderParamType::SPT_StructuredBuffer:
	case ShaderParamType::SPT_ConsumeStructuredBuffer:
	case ShaderParamType::SPT_AppendStructuredBuffer:
	case ShaderParamType::SPT_byteAddressBuffer:
		ret.func = ::SetBufferSRV(SrvSrcs[p_handle.shader_type][p_handle.offset], Srvs[p_handle.shader_type][p_handle.offset], param);
		break;


	case ShaderParamType::SPT_rwtexture1D:
	case ShaderParamType::SPT_rwtexture2D:
	case ShaderParamType::SPT_rwtexture3D:
	case ShaderParamType::SPT_rwtexture1DArray:
	case ShaderParamType::SPT_rwtexture2DArray:
		ret.func = ::SetTextureUAV(UavSrcs[p_handle.shader_type][p_handle.offset], Uavs[p_handle.shader_type][p_handle.offset], param);
		break;

	case ShaderParamType::SPT_rwbuffer:
	case ShaderParamType::SPT_rwstructured_buffer:
	case ShaderParamType::SPT_rwbyteAddressBuffer:
		ret.func = ::SetBufferUAV(UavSrcs[p_handle.shader_type][p_handle.offset], Uavs[p_handle.shader_type][p_handle.offset], param);
		break;
	}

	return ret;
}



