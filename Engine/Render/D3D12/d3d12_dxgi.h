/*! \file Engine\Render\d3d12_dxgi.h
\ingroup Engine
\brief 包装需要动态载入的函数和声明相关接口。
*/
#ifndef LE_RENDER_D3D12_d3d12_dxgi_h
#define LE_RENDER_D3D12_d3d12_dxgi_h 1

#include <LFramework/Win32/LCLib/Mingw32.h>

#include <UniversalDXSDK/d3d12.h>
#include <UniversalDXSDK/dxgi1_5.h>

#include <LFramework/Win32/LCLib/COM.h>

extern "C"
{
	typedef interface ID3D12DescriptorHeapWrap ID3D12DescriptorHeapWrap;

	typedef struct ID3D12DescriptorHeapVtbl
	{
		BEGIN_INTERFACE

			HRESULT(STDMETHODCALLTYPE *QueryInterface)(
				ID3D12DescriptorHeapWrap * This,
				REFIID riid,
				_COM_Outptr_  void **ppvObject);

		ULONG(STDMETHODCALLTYPE *AddRef)(
			ID3D12DescriptorHeapWrap * This);

		ULONG(STDMETHODCALLTYPE *Release)(
			ID3D12DescriptorHeapWrap * This);

		HRESULT(STDMETHODCALLTYPE *GetPrivateData)(
			ID3D12DescriptorHeapWrap * This,
			_In_  REFGUID guid,
			_Inout_  UINT *pDataSize,
			_Out_writes_bytes_opt_(*pDataSize)  void *pData);

		HRESULT(STDMETHODCALLTYPE *SetPrivateData)(
			ID3D12DescriptorHeapWrap * This,
			_In_  REFGUID guid,
			_In_  UINT DataSize,
			_In_reads_bytes_opt_(DataSize)  const void *pData);

		HRESULT(STDMETHODCALLTYPE *SetPrivateDataInterface)(
			ID3D12DescriptorHeapWrap * This,
			_In_  REFGUID guid,
			_In_opt_  const IUnknown *pData);

		HRESULT(STDMETHODCALLTYPE *SetName)(
			ID3D12DescriptorHeapWrap * This,
			_In_z_  LPCWSTR Name);

		HRESULT(STDMETHODCALLTYPE *GetDevice)(
			ID3D12DescriptorHeapWrap * This,
			REFIID riid,
			_COM_Outptr_opt_  void **ppvDevice);

		D3D12_DESCRIPTOR_HEAP_DESC(STDMETHODCALLTYPE *GetDesc)(
			ID3D12DescriptorHeapWrap * This);

		void(STDMETHODCALLTYPE *GetCPUDescriptorHandleForHeapStart)(
			ID3D12DescriptorHeapWrap * This, D3D12_CPU_DESCRIPTOR_HANDLE *pOut);

		D3D12_GPU_DESCRIPTOR_HANDLE(STDMETHODCALLTYPE *GetGPUDescriptorHandleForHeapStart)(
			ID3D12DescriptorHeapWrap * This);

		END_INTERFACE
	} ID3D12DescriptorHeapVtbl;

	interface ID3D12DescriptorHeapWrap
	{
		CONST_VTBL struct ID3D12DescriptorHeapVtbl *lpVtbl;
	};
}


namespace platform_ex {
	namespace Windows {
		namespace D3D {
			template<typename DXCOM>
			void Debug(DXCOM &com, const char * objectname)
			{
#if defined(_DEBUG)
				com->SetPrivateData(::WKPDID_D3DDebugObjectName,static_cast<UINT>(std::strlen(objectname)), objectname);
#endif
			}
			template<typename DXCOM>
			void Debug(DXCOM* com, const char* objectname)
			{
#if defined(_DEBUG)
				com->SetPrivateData(::WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(objectname)), objectname);
#endif
			}
		}

		/*
		\note 桌面平台这些函数是直接通过LoadProc来实现
		\warning 引擎对于这些函数不会也不能频繁调用,无视LoadProc的开销
		\warning 若相应模块未事先载入 抛出Win32Exception
		\todo UWP支持
		*/
		namespace DXGI {
			using namespace leo;

			HRESULT CreateFactory2(UINT Flags, REFIID riid, void** ppFactory);

			HRESULT CreateFactory1(REFIID riid, void** ppFactory);
		}
		namespace D3D12 {
			using namespace leo;

			HRESULT CreateDevice(IUnknown* pAdapter,
				D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid,
				void** ppDevice);
			HRESULT GetDebugInterface(REFIID riid, void** ppvDebug);
			HRESULT SerializeRootSignature(D3D12_ROOT_SIGNATURE_DESC const * pRootSignature,
				D3D_ROOT_SIGNATURE_VERSION Version, ID3D10Blob** ppBlob, ID3D10Blob** ppErrorBlob);
			HRESULT SerializeVersionedRootSignature(
				 const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
				 ID3DBlob** ppBlob,
				 ID3DBlob** ppErrorBlob);

			/*
			inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap * This) {
				auto ThisWrap = reinterpret_cast<ID3D12DescriptorHeapWrap*>(This);
				D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
				ThisWrap->lpVtbl->GetCPUDescriptorHandleForHeapStart(ThisWrap, &handle);
				return handle;
			}
			*/
			struct ResourceStateTransition {
				D3D12_RESOURCE_STATES StateBefore = D3D12_RESOURCE_STATE_COMMON;
				D3D12_RESOURCE_STATES StateAfter = D3D12_RESOURCE_STATE_COMMON;
			};

			struct TransitionBarrier :D3D12_RESOURCE_BARRIER {
				TransitionBarrier(ResourceStateTransition state_trans, COMPtr<ID3D12Resource>& pResource, 
					UINT Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE) {
					Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					Flags = flags;
					Transition.pResource = pResource.Get();
					Transition.StateBefore = state_trans.StateBefore;
					Transition.StateAfter = state_trans.StateAfter;

				}

				operator D3D12_RESOURCE_BARRIER*() {
					return this;
				}

				D3D12_RESOURCE_BARRIER* operator!() {
					std::swap(Transition.StateBefore, Transition.StateAfter);
					return this;
				}

			};

			

			inline UINT CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize)
			{
				return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
			}
		}
	}
}

namespace platform_ex::Windows::D3D12 {
	constexpr inline struct CD3DX12_DEFAULT {} D3D12_DEFAULT;

	struct CD3DX12_ROOT_CONSTANTS : public D3D12_ROOT_CONSTANTS
	{
		CD3DX12_ROOT_CONSTANTS() {}
		explicit CD3DX12_ROOT_CONSTANTS(const D3D12_ROOT_CONSTANTS& o) :
			D3D12_ROOT_CONSTANTS(o)
		{}
		CD3DX12_ROOT_CONSTANTS(
			UINT num32BitValues,
			UINT shaderRegister,
			UINT registerSpace = 0)
		{
			Init(num32BitValues, shaderRegister, registerSpace);
		}

		inline void Init(
			UINT num32BitValues,
			UINT shaderRegister,
			UINT registerSpace = 0)
		{
			Init(*this, num32BitValues, shaderRegister, registerSpace);
		}

		static inline void Init(
			_Out_ D3D12_ROOT_CONSTANTS& rootConstants,
			UINT num32BitValues,
			UINT shaderRegister,
			UINT registerSpace = 0)
		{
			rootConstants.Num32BitValues = num32BitValues;
			rootConstants.ShaderRegister = shaderRegister;
			rootConstants.RegisterSpace = registerSpace;
		}
	};

	struct CD3DX12_ROOT_SIGNATURE_DESC : public D3D12_ROOT_SIGNATURE_DESC
	{
		CD3DX12_ROOT_SIGNATURE_DESC() {}
		explicit CD3DX12_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC& o) :
			D3D12_ROOT_SIGNATURE_DESC(o)
		{}
		CD3DX12_ROOT_SIGNATURE_DESC(
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			Init(numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
		}
		CD3DX12_ROOT_SIGNATURE_DESC(CD3DX12_DEFAULT)
		{
			Init(0, NULL, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_NONE);
		}

		inline void Init(
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			Init(*this, numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
		}

		static inline void Init(
			_Out_ D3D12_ROOT_SIGNATURE_DESC& desc,
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			desc.NumParameters = numParameters;
			desc.pParameters = _pParameters;
			desc.NumStaticSamplers = numStaticSamplers;
			desc.pStaticSamplers = _pStaticSamplers;
			desc.Flags = flags;
		}
	};

	struct CD3DX12_DESCRIPTOR_RANGE1 : public D3D12_DESCRIPTOR_RANGE1
	{
		CD3DX12_DESCRIPTOR_RANGE1() { }
		explicit CD3DX12_DESCRIPTOR_RANGE1(const D3D12_DESCRIPTOR_RANGE1& o) :
			D3D12_DESCRIPTOR_RANGE1(o)
		{}
		CD3DX12_DESCRIPTOR_RANGE1(
			D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
			UINT numDescriptors,
			UINT baseShaderRegister,
			UINT registerSpace = 0,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
			UINT offsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		{
			Init(rangeType, numDescriptors, baseShaderRegister, registerSpace, flags, offsetInDescriptorsFromTableStart);
		}

		inline void Init(
			D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
			UINT numDescriptors,
			UINT baseShaderRegister,
			UINT registerSpace = 0,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
			UINT offsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		{
			Init(*this, rangeType, numDescriptors, baseShaderRegister, registerSpace, flags, offsetInDescriptorsFromTableStart);
		}

		static inline void Init(
			_Out_ D3D12_DESCRIPTOR_RANGE1& range,
			D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
			UINT numDescriptors,
			UINT baseShaderRegister,
			UINT registerSpace = 0,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
			UINT offsetInDescriptorsFromTableStart =
			D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
		{
			range.RangeType = rangeType;
			range.NumDescriptors = numDescriptors;
			range.BaseShaderRegister = baseShaderRegister;
			range.RegisterSpace = registerSpace;
			range.Flags = flags;
			range.OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
		}
	};

	struct CD3DX12_ROOT_DESCRIPTOR_TABLE1 : public D3D12_ROOT_DESCRIPTOR_TABLE1
	{
		CD3DX12_ROOT_DESCRIPTOR_TABLE1() {}
		explicit CD3DX12_ROOT_DESCRIPTOR_TABLE1(const D3D12_ROOT_DESCRIPTOR_TABLE1& o) :
			D3D12_ROOT_DESCRIPTOR_TABLE1(o)
		{}
		CD3DX12_ROOT_DESCRIPTOR_TABLE1(
			UINT numDescriptorRanges,
			_In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges)
		{
			Init(numDescriptorRanges, _pDescriptorRanges);
		}

		inline void Init(
			UINT numDescriptorRanges,
			_In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges)
		{
			Init(*this, numDescriptorRanges, _pDescriptorRanges);
		}

		static inline void Init(
			_Out_ D3D12_ROOT_DESCRIPTOR_TABLE1& rootDescriptorTable,
			UINT numDescriptorRanges,
			_In_reads_opt_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* _pDescriptorRanges)
		{
			rootDescriptorTable.NumDescriptorRanges = numDescriptorRanges;
			rootDescriptorTable.pDescriptorRanges = _pDescriptorRanges;
		}
	};

	struct CD3DX12_ROOT_DESCRIPTOR1 : public D3D12_ROOT_DESCRIPTOR1
	{
		CD3DX12_ROOT_DESCRIPTOR1() {}
		explicit CD3DX12_ROOT_DESCRIPTOR1(const D3D12_ROOT_DESCRIPTOR1& o) :
			D3D12_ROOT_DESCRIPTOR1(o)
		{}
		CD3DX12_ROOT_DESCRIPTOR1(
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			Init(shaderRegister, registerSpace, flags);
		}

		inline void Init(
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			Init(*this, shaderRegister, registerSpace, flags);
		}

		static inline void Init(
			_Out_ D3D12_ROOT_DESCRIPTOR1& table,
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			table.ShaderRegister = shaderRegister;
			table.RegisterSpace = registerSpace;
			table.Flags = flags;
		}
	};

	struct CD3DX12_ROOT_PARAMETER1 : public D3D12_ROOT_PARAMETER1
	{
		CD3DX12_ROOT_PARAMETER1() {}
		explicit CD3DX12_ROOT_PARAMETER1(const D3D12_ROOT_PARAMETER1& o) :
			D3D12_ROOT_PARAMETER1(o)
		{}

		static inline void InitAsDescriptorTable(
			_Out_ D3D12_ROOT_PARAMETER1& rootParam,
			UINT numDescriptorRanges,
			_In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam.ShaderVisibility = visibility;
			CD3DX12_ROOT_DESCRIPTOR_TABLE1::Init(rootParam.DescriptorTable, numDescriptorRanges, pDescriptorRanges);
		}

		static inline void InitAsConstants(
			_Out_ D3D12_ROOT_PARAMETER1& rootParam,
			UINT num32BitValues,
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParam.ShaderVisibility = visibility;
			CD3DX12_ROOT_CONSTANTS::Init(rootParam.Constants, num32BitValues, shaderRegister, registerSpace);
		}

		static inline void InitAsConstantBufferView(
			_Out_ D3D12_ROOT_PARAMETER1& rootParam,
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParam.ShaderVisibility = visibility;
			CD3DX12_ROOT_DESCRIPTOR1::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
		}

		static inline void InitAsShaderResourceView(
			_Out_ D3D12_ROOT_PARAMETER1& rootParam,
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			rootParam.ShaderVisibility = visibility;
			CD3DX12_ROOT_DESCRIPTOR1::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
		}

		static inline void InitAsUnorderedAccessView(
			_Out_ D3D12_ROOT_PARAMETER1& rootParam,
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
			rootParam.ShaderVisibility = visibility;
			CD3DX12_ROOT_DESCRIPTOR1::Init(rootParam.Descriptor, shaderRegister, registerSpace, flags);
		}

		inline void InitAsDescriptorTable(
			UINT numDescriptorRanges,
			_In_reads_(numDescriptorRanges) const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			InitAsDescriptorTable(*this, numDescriptorRanges, pDescriptorRanges, visibility);
		}

		inline void InitAsConstants(
			UINT num32BitValues,
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			InitAsConstants(*this, num32BitValues, shaderRegister, registerSpace, visibility);
		}

		inline void InitAsConstantBufferView(
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			InitAsConstantBufferView(*this, shaderRegister, registerSpace, flags, visibility);
		}

		inline void InitAsShaderResourceView(
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			InitAsShaderResourceView(*this, shaderRegister, registerSpace, flags, visibility);
		}

		inline void InitAsUnorderedAccessView(
			UINT shaderRegister,
			UINT registerSpace = 0,
			D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			InitAsUnorderedAccessView(*this, shaderRegister, registerSpace, flags, visibility);
		}
	};

	struct CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC : public D3D12_VERSIONED_ROOT_SIGNATURE_DESC
	{
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC() {}
		explicit CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& o) :
			D3D12_VERSIONED_ROOT_SIGNATURE_DESC(o)
		{}
		explicit CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC& o)
		{
			Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
			Desc_1_0 = o;
		}
		explicit CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC1& o)
		{
			Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			Desc_1_1 = o;
		}
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			Init_1_0(numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
		}
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			Init_1_1(numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
		}
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC(CD3DX12_DEFAULT)
		{
			Init_1_1(0, NULL, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_NONE);
		}

		inline void Init_1_0(
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			Init_1_0(*this, numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
		}

		static inline void Init_1_0(
			_Out_ D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc,
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
			desc.Desc_1_0.NumParameters = numParameters;
			desc.Desc_1_0.pParameters = _pParameters;
			desc.Desc_1_0.NumStaticSamplers = numStaticSamplers;
			desc.Desc_1_0.pStaticSamplers = _pStaticSamplers;
			desc.Desc_1_0.Flags = flags;
		}

		inline void Init_1_1(
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			Init_1_1(*this, numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
		}

		static inline void Init_1_1(
			_Out_ D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc,
			UINT numParameters,
			_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER1* _pParameters,
			UINT numStaticSamplers = 0,
			_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = NULL,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE)
		{
			desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			desc.Desc_1_1.NumParameters = numParameters;
			desc.Desc_1_1.pParameters = _pParameters;
			desc.Desc_1_1.NumStaticSamplers = numStaticSamplers;
			desc.Desc_1_1.pStaticSamplers = _pStaticSamplers;
			desc.Desc_1_1.Flags = flags;
		}
	};

	//------------------------------------------------------------------------------------------------
	// D3D12 exports a new method for serializing root signatures in the Windows 10 Anniversary Update.
	// To help enable root signature 1.1 features when they are available and not require maintaining
	// two code paths for building root signatures, this helper method reconstructs a 1.0 signature when
	// 1.1 is not supported.
	inline HRESULT D3DX12SerializeVersionedRootSignature(
		_In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION MaxVersion,
		_Outptr_ ID3DBlob** ppBlob,
		_Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
	{
		if (ppErrorBlob != NULL)
		{
			*ppErrorBlob = NULL;
		}

		switch (MaxVersion)
		{
		case D3D_ROOT_SIGNATURE_VERSION_1_0:
			switch (pRootSignatureDesc->Version)
			{
			case D3D_ROOT_SIGNATURE_VERSION_1_0:
				return SerializeRootSignature(&pRootSignatureDesc->Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);

			case D3D_ROOT_SIGNATURE_VERSION_1_1:
			{
				HRESULT hr = S_OK;
				const D3D12_ROOT_SIGNATURE_DESC1& desc_1_1 = pRootSignatureDesc->Desc_1_1;

				const SIZE_T ParametersSize = sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters;
				void* pParameters = (ParametersSize > 0) ? HeapAlloc(GetProcessHeap(), 0, ParametersSize) : NULL;
				if (ParametersSize > 0 && pParameters == NULL)
				{
					hr = E_OUTOFMEMORY;
				}
				D3D12_ROOT_PARAMETER* pParameters_1_0 = reinterpret_cast<D3D12_ROOT_PARAMETER*>(pParameters);

				if (SUCCEEDED(hr))
				{
					for (UINT n = 0; n < desc_1_1.NumParameters; n++)
					{
						__analysis_assume(ParametersSize == sizeof(D3D12_ROOT_PARAMETER) * desc_1_1.NumParameters);
						pParameters_1_0[n].ParameterType = desc_1_1.pParameters[n].ParameterType;
						pParameters_1_0[n].ShaderVisibility = desc_1_1.pParameters[n].ShaderVisibility;

						switch (desc_1_1.pParameters[n].ParameterType)
						{
						case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
							pParameters_1_0[n].Constants.Num32BitValues = desc_1_1.pParameters[n].Constants.Num32BitValues;
							pParameters_1_0[n].Constants.RegisterSpace = desc_1_1.pParameters[n].Constants.RegisterSpace;
							pParameters_1_0[n].Constants.ShaderRegister = desc_1_1.pParameters[n].Constants.ShaderRegister;
							break;

						case D3D12_ROOT_PARAMETER_TYPE_CBV:
						case D3D12_ROOT_PARAMETER_TYPE_SRV:
						case D3D12_ROOT_PARAMETER_TYPE_UAV:
							pParameters_1_0[n].Descriptor.RegisterSpace = desc_1_1.pParameters[n].Descriptor.RegisterSpace;
							pParameters_1_0[n].Descriptor.ShaderRegister = desc_1_1.pParameters[n].Descriptor.ShaderRegister;
							break;

						case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
							const D3D12_ROOT_DESCRIPTOR_TABLE1& table_1_1 = desc_1_1.pParameters[n].DescriptorTable;

							const SIZE_T DescriptorRangesSize = sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges;
							void* pDescriptorRanges = (DescriptorRangesSize > 0 && SUCCEEDED(hr)) ? HeapAlloc(GetProcessHeap(), 0, DescriptorRangesSize) : NULL;
							if (DescriptorRangesSize > 0 && pDescriptorRanges == NULL)
							{
								hr = E_OUTOFMEMORY;
							}
							D3D12_DESCRIPTOR_RANGE* pDescriptorRanges_1_0 = reinterpret_cast<D3D12_DESCRIPTOR_RANGE*>(pDescriptorRanges);

							if (SUCCEEDED(hr))
							{
								for (UINT x = 0; x < table_1_1.NumDescriptorRanges; x++)
								{
									__analysis_assume(DescriptorRangesSize == sizeof(D3D12_DESCRIPTOR_RANGE) * table_1_1.NumDescriptorRanges);
									pDescriptorRanges_1_0[x].BaseShaderRegister = table_1_1.pDescriptorRanges[x].BaseShaderRegister;
									pDescriptorRanges_1_0[x].NumDescriptors = table_1_1.pDescriptorRanges[x].NumDescriptors;
									pDescriptorRanges_1_0[x].OffsetInDescriptorsFromTableStart = table_1_1.pDescriptorRanges[x].OffsetInDescriptorsFromTableStart;
									pDescriptorRanges_1_0[x].RangeType = table_1_1.pDescriptorRanges[x].RangeType;
									pDescriptorRanges_1_0[x].RegisterSpace = table_1_1.pDescriptorRanges[x].RegisterSpace;
								}
							}

							D3D12_ROOT_DESCRIPTOR_TABLE& table_1_0 = pParameters_1_0[n].DescriptorTable;
							table_1_0.NumDescriptorRanges = table_1_1.NumDescriptorRanges;
							table_1_0.pDescriptorRanges = pDescriptorRanges_1_0;
						}
					}
				}

				if (SUCCEEDED(hr))
				{
					CD3DX12_ROOT_SIGNATURE_DESC desc_1_0(desc_1_1.NumParameters, pParameters_1_0, desc_1_1.NumStaticSamplers, desc_1_1.pStaticSamplers, desc_1_1.Flags);
					hr = SerializeRootSignature(&desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, ppBlob, ppErrorBlob);
				}

				if (pParameters)
				{
					for (UINT n = 0; n < desc_1_1.NumParameters; n++)
					{
						if (desc_1_1.pParameters[n].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
						{
							HeapFree(GetProcessHeap(), 0, reinterpret_cast<void*>(const_cast<D3D12_DESCRIPTOR_RANGE*>(pParameters_1_0[n].DescriptorTable.pDescriptorRanges)));
						}
					}
					HeapFree(GetProcessHeap(), 0, pParameters);
				}
				return hr;
			}
			}
			break;

		case D3D_ROOT_SIGNATURE_VERSION_1_1:
			return SerializeVersionedRootSignature(pRootSignatureDesc, ppBlob, ppErrorBlob);
		}

		return E_INVALIDARG;
	}
}

// This is a value that should be tweaked to fit the app, lower numbers will have better performance
// Titles using many terrain layers may want to set MAX_SRVS to 64 to avoid shader compilation errors. This will have a small performance hit of around 0.1%
#define MAX_SRVS		48
#define MAX_SAMPLERS	16
#define MAX_UAVS		16
#define MAX_CBS			16

// This value controls how many root constant buffers can be used per shader stage in a root signature.
// Note: Using root descriptors significantly increases the size of root signatures (each root descriptor is 2 DWORDs).
#define MAX_ROOT_CBVS	MAX_CBS

#endif