//Copyright (c) Leo Hawke. All rights reserved.
//must compiler by cl
#pragma once



#include "leoint.hpp"

#include "platform.h"
#include "Core\COM.hpp"
#include <comdef.h>
#include <d3d11.h>
#include <d3d11shader.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxguid.lib")
#ifdef DEBUG
#include <D3D11ShaderTracing.h>
#include <d3d11sdklayers.h> //for D3D11Debug
#else
typedef enum D3D11_SHADER_TYPE{
	D3D11_VERTEX_SHADER = 1,
	D3D11_HULL_SHADER = 2,
	D3D11_DOMAIN_SHADER = 3,
	D3D11_GEOMETRY_SHADER = 4,
	D3D11_PIXEL_SHADER = 5,
	D3D11_COMPUTE_SHADER = 6
} D3D11_SHADER_TYPE;
#endif

#include <string>
#include <map>
#include <algorithm>

namespace leo
{
	//interface ptr
	namespace dx
	{
	}
	
	//core function
	namespace dx
	{
		template<typename DXCOM>
		void DebugCOM(DXCOM* &com, std::size_t ecount, const char * objectname)
		{
#if defined(DEBUG) || defined(PROFILE)
			if (com)
				com->SetPrivateData(::WKPDID_D3DDebugObjectName,static_cast<win::UINT>( ecount), objectname);
#endif
		}
		template<typename DXCOM>
		void DebugCOM(DXCOM &com, std::size_t ecount, const char * objectname)
		{
#if defined(DEBUG) || defined(PROFILE)
			if (com)
				com->SetPrivateData(::WKPDID_D3DDebugObjectName, ecount, objectname);
#endif
		}
		
		template<typename DXCOM>
		void DebugCOM(DXCOM* &com,const char * objectname)
		{
#if defined(DEBUG) || defined(PROFILE)
			if (com)
				com->SetPrivateData(::WKPDID_D3DDebugObjectName,static_cast<win::UINT>(std::strlen(objectname)), objectname);
#endif
		}
		template<typename DXCOM>
		void DebugCOM(DXCOM* &com, const wchar_t * objectname)
		{
#if defined(DEBUG) || defined(PROFILE)
			if (com)
				com->SetPrivateData(::WKPDID_D3DDebugObjectName, static_cast<win::UINT>(std::wcslen(objectname) * 2), objectname);
#endif
		}
		template<typename DXCOM>
		void DebugCOM(DXCOM &com,const char * objectname)
		{
#if defined(DEBUG) || defined(PROFILE)
			if (com)
				com->SetPrivateData(::WKPDID_D3DDebugObjectName, std::strlen(objectname), objectname);
#endif
		}

		template<typename T> class ScopedCOMObject
		{
			ScopedCOMObject(const ScopedCOMObject&) = delete;
			ScopedCOMObject& operator=(const ScopedCOMObject&) = delete;

			T* pointer = nullptr;
		public:
			explicit ScopedCOMObject(T *p = 0) : pointer(p) {}
			~ScopedCOMObject()
			{
				if (pointer)
				{
					pointer->Release();
					pointer = nullptr;
				}
			}

			bool IsNull() const { return (!pointer); }

			T& operator*() { return *pointer; }
			T* operator->() { return pointer; }
			T** operator&() { return &pointer; }

			void Reset(T *p = 0) { if (pointer) { pointer->Release(); } pointer = p; }

			T* Get() const { return pointer; }
		};
	}
	
	//load function
	namespace dx
	{
		// Note: Assumes application has already called CoInitializeEx
		//
		// Warning: CreateWICTextureByWIC functions are not thread-safe if given a ID3D11DeviceContext instance for
		//          auto-gen mipmap support.
		HRESULT CreateWICTextureByWIC(_In_ ID3D11Device * device,
			_Out_ ID3D11Resource* *texture,
			_In_bytecount_(wicDatasize) const std::uint8_t * wicData,
			_In_ size_t wicDataSize,
			_In_opt_ ID3D11DeviceContext * deviceContext = nullptr,
			_Out_opt_ ID3D11ShaderResourceView* *textureView = nullptr,
			_In_ size_t maxsize = 0
			);
		HRESULT CreateWICTextureByWIC(_In_ ID3D11Device* device,
			_Out_ ID3D11Resource* * texture,
			_In_z_ const wchar_t* szFileName,
			_In_opt_ ID3D11DeviceContext* deviceContext = nullptr,
			_Out_opt_ ID3D11ShaderResourceView* * textureView = nullptr,
			_In_ size_t maxsize = 0
			);

#if defined(DEBUG) | defined(_DEBUG)
#ifndef DebugDXCOM
#define DebugDXCOM(x) leo::dx::DebugCOM(x,sizeof(#x) -1,#x)
#endif
#else
#ifndef DebugDXCOM
#define DebugDXCOM(x) {}
#endif
#endif
	}
	
	namespace ops {
		struct Rect;
	}

	//helper function
	namespace dx
	{
		struct d3d11_timer
		{
			static const std::uint64_t querylatency = 5;

			d3d11_timer()
			{
				std::memset(this, 0, sizeof(d3d11_timer));
			}

			~d3d11_timer()
			{
				for (auto &query : disjoint)
					leo::win::ReleaseCOM(query);
				for (auto &query : timestampstart)
					leo::win::ReleaseCOM(query);
				for (auto &query : timestampend)
					leo::win::ReleaseCOM(query);
			}

			d3d11_timer(d3d11_timer&&  rvalue)
			{
				std::memcpy(this, &rvalue, sizeof(d3d11_timer));
				std::memset(&rvalue, 0, sizeof(d3d11_timer));
			}

			d3d11_timer& operator=(d3d11_timer&& rvalue)
			{
				std::memcpy(this, &rvalue, sizeof(d3d11_timer));
				std::memset(&rvalue, 0, sizeof(d3d11_timer));
				return *this;
			}

			ID3D11Query* disjoint[querylatency];
			ID3D11Query* timestampstart[querylatency];
			ID3D11Query* timestampend[querylatency];
			bool started{ false };
			bool finished{ false };

			void start(std::uint64_t currframe,ID3D11Device* device, ID3D11DeviceContext* context);
			void end(std::uint64_t currframe, ID3D11DeviceContext* context);
			float time(std::uint64_t currframe, ID3D11DeviceContext* context);
		};
		class profiler
		{
		public:
			static profiler global_profiler;
			
			void init(ID3D11Device* device, ID3D11DeviceContext* context);

			void startprofile(const std::wstring& name);
			void endprofile(const std::wstring& name);

			void endframe();//SpriteRenderer,SpriteFont
		protected:
			static const std::uint64_t querylatency = d3d11_timer::querylatency;

			using profilemap = std::map<std::wstring, d3d11_timer>;

			profilemap profiles;
			std::uint64_t currframe;

			ID3D11Device* device;
			ID3D11DeviceContext* context;
		};
		class profileblock
		{
		public:
			profileblock(const std::wstring& name);
			~profileblock();
		protected:
			std::wstring name;
		};

		using std::max;

		inline unsigned int NumMipLevels(unsigned int width,unsigned int height)
		{
			unsigned int nummips = 0;
			unsigned int size = max(width, height);
			while (1U << nummips <= size)
				++nummips;
			
			assert(1U << nummips > size);

			return nummips;
		}

		inline void SetViewPort(ID3D11DeviceContext* contex, UINT widht, UINT height)
		{
			D3D11_VIEWPORT viewport;
			viewport.Width = static_cast<float>(widht);
			viewport.Height = static_cast<float>(height);
			viewport.MinDepth = 0.f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;

			contex->RSSetViewports(1, &viewport);
		}

		inline unsigned int CBSize(unsigned int size)
		{
			decltype(size) cbsize = size + (16 - (size & ~16));
			return cbsize;
		}
	
		D3D11_RECT ScissorRectFromNDCRect(const ops::Rect& ndcRect, const std::pair<uint16, uint16>& size);
	}

	//computer shader
	namespace dx
	{ 
		inline std::uint32_t DisptachSize(std::uint32_t threads, std::uint32_t nums)
		{
			nums += (threads - 1);
			return nums / threads;
		}

		inline void SetCSInputs(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv0, ID3D11ShaderResourceView* srv1,
			ID3D11ShaderResourceView* srv2, ID3D11ShaderResourceView* srv3)
		{
			ID3D11ShaderResourceView* srvs[4] = { srv0, srv1, srv2, srv3 };
			context->CSSetShaderResources(0, 4, srvs);
		}

		inline void ClearCSInputs(ID3D11DeviceContext* context)
		{
			SetCSInputs(context, nullptr, nullptr, nullptr, nullptr);
		}

		inline void SetCSOutputs(ID3D11DeviceContext* context, ID3D11UnorderedAccessView* uav0, ID3D11UnorderedAccessView* uav1,
			ID3D11UnorderedAccessView* uav2, ID3D11UnorderedAccessView* uav3, ID3D11UnorderedAccessView* uav4,
			ID3D11UnorderedAccessView* uav5)
		{
			ID3D11UnorderedAccessView* uavs[6] = { uav0, uav1, uav2, uav3, uav4, uav5 };
			context->CSSetUnorderedAccessViews(0, 6, uavs, nullptr);
		}

		inline void ClearCSOutputs(ID3D11DeviceContext* context)
		{
			SetCSOutputs(context, nullptr, nullptr, nullptr, nullptr,nullptr,nullptr);
		}

		inline void SetCSSamplers(ID3D11DeviceContext* context, ID3D11SamplerState* sampler0, ID3D11SamplerState* sampler1,
			ID3D11SamplerState* sampler2, ID3D11SamplerState* sampler3)
		{
			ID3D11SamplerState* samplers[4] = { sampler0, sampler1, sampler2, sampler3 };
			context->CSSetSamplers(0, 4, samplers);
		}

		inline void SetCSShader(ID3D11DeviceContext* context, ID3D11ComputeShader* shader)
		{
			context->CSSetShader(shader, nullptr, 0);
		}
	}
}