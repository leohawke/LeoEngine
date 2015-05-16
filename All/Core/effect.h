////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/effect.h
//  Version:     v1.01
//  Created:     8/15/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供引擎所需的各种渲染效果,相关配置接口
// -------------------------------------------------------------------------
//  History:
//				8/19/2014 修改为单列模式,改用PIMPL实现,NormalLine侵入Render
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_effect_h
#define Core_effect_h

#include "platform.h"
#include "Singleton.hpp"
#include "leoint.hpp"
#include "utility.hpp"
#include "Light.hpp"
#include "Material.h"
#include "exception.hpp"
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
namespace leo
{
	class EffectConfig : public Singleton < EffectConfig >
	{
	protected:
		EffectConfig();
	private:
		class Delegate;
		std::unique_ptr<Delegate> pDelegate;
	public:
		~EffectConfig();
	public:
		static const std::unique_ptr<EffectConfig>& GetInstance()
		{
			static auto mInstance = unique_raw(new EffectConfig());
			return mInstance;
		}
	public:
		enum class EffectLevel : uint8
		{
			low = 255,
			medium = 127,
			hign = 0
		};

		EffectLevel GetLevel() const lnothrow;
		bool SetLevel(EffectLevel l) lnothrow;

		bool NormalLine() const lnothrow;
		void NormalLine(bool nlstate) lnothrow;
	};

	class Effect : public PassAlloc
	{
	protected:
		Effect();
	public:
		template<typename BUFFPOD>
		struct ShaderConstantBuffer : BUFFPOD
		{
			static_assert(sizeof(BUFFPOD) % 16 == 0, "the template type's bytes must be 16 times");
			typedef BUFFPOD value_type;
			ShaderConstantBuffer(ID3D11Device* device)
			{
				D3D11_BUFFER_DESC Desc;
				Desc.Usage = D3D11_USAGE_DEFAULT;
				Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				Desc.CPUAccessFlags = 0;
				Desc.MiscFlags = 0;
				Desc.StructureByteStride = 0;
				Desc.ByteWidth = sizeof(BUFFPOD);

				dxcall(device->CreateBuffer(&Desc, nullptr, &mBuffer));
			}
			~ShaderConstantBuffer()
			{
				win::ReleaseCOM(mBuffer);
			}
			void Update(ID3D11DeviceContext* context)
			{
				context->UpdateSubresource(mBuffer, 0, nullptr, this, 0, 0);
			}
		public:
			ID3D11Buffer* mBuffer = nullptr;
		};

		enum class LightType : std::uint8_t
		{
			DirectionLight = 0,
			PointLight = 1,
			SpotLight = 2,
			LightCount
		};
	public:
		virtual bool SetLevel(EffectConfig::EffectLevel l) lnothrow = 0;
	};

	class EffectNormalMap :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);

		void WorldViewMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);
		void WorldViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);

		void LM_VECTOR_CALL WorldViewMatrix(std::array<__m128,4> matrix, ID3D11DeviceContext* context = nullptr);
		void  LM_VECTOR_CALL WorldViewProjMatrix(std::array<__m128, 4>  matrix, ID3D11DeviceContext* context = nullptr);
		void LM_VECTOR_CALL WorldInvTransposeViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr);


		void ShadowViewProjTexMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);
		void LM_VECTOR_CALL ShadowViewProjTexMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr);

		void EyePos(const float3& pos, ID3D11DeviceContext* context = nullptr);

		void Light(const DirectionalLight& dl, ID3D11DeviceContext* context = nullptr);
		void Light(const PointLight& pl, ID3D11DeviceContext* context = nullptr);
		void Light(const SpotLight& sl, ID3D11DeviceContext* context = nullptr);

		void Mat(const Material& mat, ID3D11DeviceContext* context = nullptr);

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context = nullptr);
		void NormalMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context = nullptr);
		void ShadowMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context = nullptr);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;
	public:
		static const std::unique_ptr<EffectNormalMap>& GetInstance(ID3D11Device* device = nullptr);
	};

	class EffectSprite :public Effect, ABSTRACT
	{
	public:

		void Apply(ID3D11DeviceContext* context);

		void EyePos(const float3& pos, ID3D11DeviceContext* context = nullptr);

		void ViewProj(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;

	public:
		static const std::unique_ptr<EffectSprite>& GetInstance(ID3D11Device* device = nullptr);
	};

	class EffectSky :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);

		void EyePos(const float3& pos, ID3D11DeviceContext* context = nullptr);

		void ViewProj(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);
		void LM_VECTOR_CALL ViewProj(std::array<__m128,4> matrix, ID3D11DeviceContext* context = nullptr);

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;

	public:
		static const std::unique_ptr<EffectSky>& GetInstance(ID3D11Device* device = nullptr);
	};

	//pack SRV<UNKNOW_FOMRAT 2D> to R16G16B16A16FLOAT<RenderTarget>
	class EffectPack :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
		
		void SetPackSRV(ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context = nullptr);
		void SetDstRTV(ID3D11RenderTargetView* rtv);
	public:
		static const std::unique_ptr<EffectPack>& GetInstance(ID3D11Device* device = nullptr);
	};

	//unpack UINT<R11G11B10> to RenderTarget<UNKNOW_FOMRAT 2D>
	class EffectUnPack :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}

		void SetUnpackSRV(ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context = nullptr);

	public:
		static const std::unique_ptr<EffectUnPack>& GetInstance(ID3D11Device* device = nullptr);
	};

	class EffectNormalLine :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);

		void ViewProj(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);
		void World(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);

		void LM_VECTOR_CALL ViewProj(std::array<__m128,4> matrix, ID3D11DeviceContext* context = nullptr);
		void LM_VECTOR_CALL World(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr);

		void Color(const float4& color, ID3D11DeviceContext* context = nullptr);

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
	public:
		static const std::unique_ptr<EffectNormalLine>& GetInstance(ID3D11Device* device = nullptr, const float4& color = float4(1.f, 0.f, 0.f, 1.0f));
	};
	/*
	

	

	

	
	*/

	class context_wrapper
	{
	protected:
		struct state
		{
			uint64 f = 0;


			enum state_type
			{
				//将获得的RT保存在1-D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT
				omsetrendertargets = 1,//上一帧设置,这一帧要还原
				end_omsetrendertargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
				//获得的DS保存在D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT
				omsetdepthstencilview = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT + 1,//上一帧设置,这一帧要还原
				//参考值保存在D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT+1
				stencil_ref = omsetdepthstencilview + 1,//上一帧设置,这一帧要还原
				rssetviewprots = stencil_ref + 1,//上一帧设置,这一帧要还原
				end_rssetviewprots = stencil_ref + D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,
				rssetscissorrects = end_rssetviewprots + 1,//上一帧设置,这一帧要还原
				end_rssetscissorrects = rssetscissorrects + D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,

				total_record = end_rssetscissorrects,

				
				//清除
				hssetshader = total_record + 1,//上一帧设置,这一帧要清除
				//清除
				dssetshader = hssetshader + 1,//上一帧设置,这一帧要清除
				//清除
				gssetshader = dssetshader + 1,//上一帧设置,这一帧要清除
				//清除
				pssetshader = gssetshader + 1,//上一帧设置,这一帧要清除
				//清除
				omsetblendstate = pssetshader + 1,//上一帧设置,这一帧要清除
				//清除
				sosettargets = omsetblendstate + 1,//上一帧设置,这一帧要清除
				//清除
				rssetstate = sosettargets + 1,//上一帧设置,这一帧要清除
				//清除
				omsetdepthstencilstate = rssetstate + 1//上一帧设置,这一帧要清除
			};

			union
			{
				win::UINT n;
				std::uintptr_t p;
				D3D11_VIEWPORT v;
				D3D11_RECT r;
			}ptr[total_record];

			state()
			{
				std::memset(this, 0, sizeof(state));
			}
		};
	public:
		context_wrapper(ID3D11DeviceContext* context, const std::wstring& effect_name = L"defualt_effect")
			:context(context)
		{
#ifndef PROFILE
			this->effect_name = effect_name;
#endif
		}
		~context_wrapper()
		{
			apply_swap();
		}

	public:
		inline void STDMETHODCALLTYPE PSSetShader(
			/* [annotation] */
			_In_opt_  ID3D11PixelShader *pPixelShader,
			/* [annotation] */
			_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
			UINT NumClassInstances);

		inline void STDMETHODCALLTYPE GSSetShader(
			/* [annotation] */
			_In_opt_  ID3D11GeometryShader *pShader,
			/* [annotation] */
			_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
			UINT NumClassInstances);

		void STDMETHODCALLTYPE HSSetShader(
			/* [annotation] */
			_In_opt_  ID3D11HullShader *pShader,
			/* [annotation] */
			_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
			UINT NumClassInstances);

		void STDMETHODCALLTYPE DSSetShader(
			/* [annotation] */
			_In_opt_  ID3D11DomainShader *pShader,
			/* [annotation] */
			_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
			UINT NumClassInstances);

		void STDMETHODCALLTYPE OMSetRenderTargets(
			/* [annotation] */
			_In_range_(0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT)  UINT NumViews,
			/* [annotation] */
			_In_reads_opt_(NumViews)  ID3D11RenderTargetView *const *ppRenderTargetViews,
			/* [annotation] */
			_In_opt_  ID3D11DepthStencilView *pDepthStencilView);

		inline void STDMETHODCALLTYPE OMSetBlendState(
			/* [annotation] */
			_In_opt_  ID3D11BlendState *pBlendState,
			/* [annotation] */
			_In_opt_  const FLOAT BlendFactor[4],
			/* [annotation] */
			_In_  UINT SampleMask);
#ifdef PROFILE
		inline
#endif
			void STDMETHODCALLTYPE OMSetDepthStencilState(
			/* [annotation] */
			_In_opt_  ID3D11DepthStencilState *pDepthStencilState,
			/* [annotation] */
			_In_  UINT StencilRef);

		void STDMETHODCALLTYPE SOSetTargets(
			/* [annotation] */
			_In_range_(0, D3D11_SO_BUFFER_SLOT_COUNT)  UINT NumBuffers,
			/* [annotation] */
			_In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppSOTargets,
			/* [annotation] */
			_In_reads_opt_(NumBuffers)  const UINT *pOffsets);

		void STDMETHODCALLTYPE RSSetState(
			/* [annotation] */
			_In_opt_  ID3D11RasterizerState *pRasterizerState);

		void STDMETHODCALLTYPE RSSetViewports(
			/* [annotation] */
			_In_range_(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
			/* [annotation] */
			_In_reads_opt_(NumViewports)  const D3D11_VIEWPORT *pViewports);

		void STDMETHODCALLTYPE RSSetScissorRects(
			/* [annotation] */
			_In_range_(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
			/* [annotation] */
			_In_reads_opt_(NumRects)  const D3D11_RECT *pRects);

		operator ID3D11DeviceContext*() const
		{
			return context;
		}

		ID3D11DeviceContext* operator->() const
		{
			return context;
		}

	public:
		inline void STDMETHODCALLTYPE VSSetConstantBuffers(
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)  UINT StartSlot,
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot)  UINT NumBuffers,
			/* [annotation] */
			_In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
		{
			context->VSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
		}

		inline void STDMETHODCALLTYPE PSSetShaderResources(
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1)  UINT StartSlot,
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot)  UINT NumViews,
			/* [annotation] */
			_In_reads_opt_(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews)
		{
			context->PSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
		}

		inline void STDMETHODCALLTYPE PSSetSamplers(
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1)  UINT StartSlot,
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot)  UINT NumSamplers,
			/* [annotation] */
			_In_reads_opt_(NumSamplers)  ID3D11SamplerState *const *ppSamplers)
		{
			context->PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
		}

		inline void STDMETHODCALLTYPE VSSetShader(
			/* [annotation] */
			_In_opt_  ID3D11VertexShader *pVertexShader,
			/* [annotation] */
			_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
			UINT NumClassInstances)
		{
			context->VSSetShader(pVertexShader, ppClassInstances, NumClassInstances);
		}

		inline void STDMETHODCALLTYPE PSSetConstantBuffers(
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)  UINT StartSlot,
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot)  UINT NumBuffers,
			/* [annotation] */
			_In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
		{
			context->PSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
		}

		inline void STDMETHODCALLTYPE GSSetConstantBuffers(
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)  UINT StartSlot,
			/* [annotation] */
			_In_range_(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot)  UINT NumBuffers,
			/* [annotation] */
			_In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
		{
			context->GSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
		}
	private:
		ID3D11DeviceContext* context;
#ifndef PROFILE
		static std::wstring effect_name;
#endif
		void apply_swap();
	private:
		static std::unique_ptr<state> swap_states[2];
	};
}

#endif