/*! \file Engine\Render\d3d12_dxgi.h
\ingroup Engine
\brief 包装需要动态载入的D3D12函数和声明相关接口以及相关常量定义。
*/
#ifndef LE_RENDER_D3D12_d3d12_dxgi_h
#define LE_RENDER_D3D12_d3d12_dxgi_h 1

#include <LBase/ldef.h>
#include <LBase/linttype.hpp>
#include <LFramework/Win32/LCLib/COM.h>
#include <UniversalDXSDK/d3d12.h>
#include <UniversalDXSDK/dxgi1_6.h>

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

//CD3DX12
namespace platform_ex::Windows::D3D12 {
	constexpr inline struct CD3DX12_DEFAULT {} D3D12_DEFAULT;

	struct CD3DX12_RECT : public D3D12_RECT
	{
		CD3DX12_RECT()
		{}
		explicit CD3DX12_RECT(const D3D12_RECT& o) :
			D3D12_RECT(o)
		{}
		explicit CD3DX12_RECT(
			LONG Left,
			LONG Top,
			LONG Right,
			LONG Bottom)
		{
			left = Left;
			top = Top;
			right = Right;
			bottom = Bottom;
		}
		~CD3DX12_RECT() {}
		operator const D3D12_RECT& () const { return *this; }
	};

	struct CD3DX12_CLEAR_VALUE : public D3D12_CLEAR_VALUE
	{
		CD3DX12_CLEAR_VALUE()
		{}
		explicit CD3DX12_CLEAR_VALUE(const D3D12_CLEAR_VALUE& o) :
			D3D12_CLEAR_VALUE(o)
		{}
		CD3DX12_CLEAR_VALUE(
			DXGI_FORMAT format,
			const FLOAT color[4])
		{
			Format = format;
			memcpy(Color, color, sizeof(Color));
		}
		CD3DX12_CLEAR_VALUE(
			DXGI_FORMAT format,
			FLOAT depth,
			UINT8 stencil)
		{
			Format = format;
			/* Use memcpy to preserve NAN values */
			memcpy(&DepthStencil.Depth, &depth, sizeof(depth));
			DepthStencil.Stencil = stencil;
		}
		operator const D3D12_CLEAR_VALUE& () const { return *this; }
	};

	struct CD3DX12_DEPTH_STENCIL_DESC1 : public D3D12_DEPTH_STENCIL_DESC1
	{
		CD3DX12_DEPTH_STENCIL_DESC1()
		{}
		explicit CD3DX12_DEPTH_STENCIL_DESC1(const D3D12_DEPTH_STENCIL_DESC1& o) :
			D3D12_DEPTH_STENCIL_DESC1(o)
		{}
		explicit CD3DX12_DEPTH_STENCIL_DESC1(const D3D12_DEPTH_STENCIL_DESC& o)
		{
			DepthEnable = o.DepthEnable;
			DepthWriteMask = o.DepthWriteMask;
			DepthFunc = o.DepthFunc;
			StencilEnable = o.StencilEnable;
			StencilReadMask = o.StencilReadMask;
			StencilWriteMask = o.StencilWriteMask;
			FrontFace.StencilFailOp = o.FrontFace.StencilFailOp;
			FrontFace.StencilDepthFailOp = o.FrontFace.StencilDepthFailOp;
			FrontFace.StencilPassOp = o.FrontFace.StencilPassOp;
			FrontFace.StencilFunc = o.FrontFace.StencilFunc;
			BackFace.StencilFailOp = o.BackFace.StencilFailOp;
			BackFace.StencilDepthFailOp = o.BackFace.StencilDepthFailOp;
			BackFace.StencilPassOp = o.BackFace.StencilPassOp;
			BackFace.StencilFunc = o.BackFace.StencilFunc;
			DepthBoundsTestEnable = FALSE;
		}
		explicit CD3DX12_DEPTH_STENCIL_DESC1(CD3DX12_DEFAULT)
		{
			DepthEnable = TRUE;
			DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			StencilEnable = FALSE;
			StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
			StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
			const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
			{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
			FrontFace = defaultStencilOp;
			BackFace = defaultStencilOp;
			DepthBoundsTestEnable = FALSE;
		}
		explicit CD3DX12_DEPTH_STENCIL_DESC1(
			BOOL depthEnable,
			D3D12_DEPTH_WRITE_MASK depthWriteMask,
			D3D12_COMPARISON_FUNC depthFunc,
			BOOL stencilEnable,
			UINT8 stencilReadMask,
			UINT8 stencilWriteMask,
			D3D12_STENCIL_OP frontStencilFailOp,
			D3D12_STENCIL_OP frontStencilDepthFailOp,
			D3D12_STENCIL_OP frontStencilPassOp,
			D3D12_COMPARISON_FUNC frontStencilFunc,
			D3D12_STENCIL_OP backStencilFailOp,
			D3D12_STENCIL_OP backStencilDepthFailOp,
			D3D12_STENCIL_OP backStencilPassOp,
			D3D12_COMPARISON_FUNC backStencilFunc,
			BOOL depthBoundsTestEnable)
		{
			DepthEnable = depthEnable;
			DepthWriteMask = depthWriteMask;
			DepthFunc = depthFunc;
			StencilEnable = stencilEnable;
			StencilReadMask = stencilReadMask;
			StencilWriteMask = stencilWriteMask;
			FrontFace.StencilFailOp = frontStencilFailOp;
			FrontFace.StencilDepthFailOp = frontStencilDepthFailOp;
			FrontFace.StencilPassOp = frontStencilPassOp;
			FrontFace.StencilFunc = frontStencilFunc;
			BackFace.StencilFailOp = backStencilFailOp;
			BackFace.StencilDepthFailOp = backStencilDepthFailOp;
			BackFace.StencilPassOp = backStencilPassOp;
			BackFace.StencilFunc = backStencilFunc;
			DepthBoundsTestEnable = depthBoundsTestEnable;
		}
		~CD3DX12_DEPTH_STENCIL_DESC1() {}
		operator const D3D12_DEPTH_STENCIL_DESC1& () const { return *this; }
		operator D3D12_DEPTH_STENCIL_DESC() const
		{
			D3D12_DEPTH_STENCIL_DESC D;
			D.DepthEnable = DepthEnable;
			D.DepthWriteMask = DepthWriteMask;
			D.DepthFunc = DepthFunc;
			D.StencilEnable = StencilEnable;
			D.StencilReadMask = StencilReadMask;
			D.StencilWriteMask = StencilWriteMask;
			D.FrontFace.StencilFailOp = FrontFace.StencilFailOp;
			D.FrontFace.StencilDepthFailOp = FrontFace.StencilDepthFailOp;
			D.FrontFace.StencilPassOp = FrontFace.StencilPassOp;
			D.FrontFace.StencilFunc = FrontFace.StencilFunc;
			D.BackFace.StencilFailOp = BackFace.StencilFailOp;
			D.BackFace.StencilDepthFailOp = BackFace.StencilDepthFailOp;
			D.BackFace.StencilPassOp = BackFace.StencilPassOp;
			D.BackFace.StencilFunc = BackFace.StencilFunc;
			return D;
		}
	};

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

	struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE() {}
		explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE& o) :
			D3D12_CPU_DESCRIPTOR_HANDLE(o)
		{}
		CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT) { ptr = 0; }
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncrementSize)
		{
			InitOffsetted(other, offsetScaledByIncrementSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			ptr += offsetInDescriptors * descriptorIncrementSize;
			return *this;
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetScaledByIncrementSize)
		{
			ptr += offsetScaledByIncrementSize;
			return *this;
		}
		bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const
		{
			return (ptr == other.ptr);
		}
		bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const
		{
			return (ptr != other.ptr);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE& operator=(const D3D12_CPU_DESCRIPTOR_HANDLE& other)
		{
			ptr = other.ptr;
			return *this;
		}

		inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
		{
			InitOffsetted(*this, base, offsetScaledByIncrementSize);
		}

		inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
		}

		static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
		{
			handle.ptr = base.ptr + offsetScaledByIncrementSize;
		}

		static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			handle.ptr = base.ptr + offsetInDescriptors * descriptorIncrementSize;
		}
	};

	//------------------------------------------------------------------------------------------------
	struct CD3DX12_GPU_DESCRIPTOR_HANDLE : public D3D12_GPU_DESCRIPTOR_HANDLE
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE() {}
		explicit CD3DX12_GPU_DESCRIPTOR_HANDLE(const D3D12_GPU_DESCRIPTOR_HANDLE& o) :
			D3D12_GPU_DESCRIPTOR_HANDLE(o)
		{}
		CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT) { ptr = 0; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncrementSize)
		{
			InitOffsetted(other, offsetScaledByIncrementSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			ptr += offsetInDescriptors * descriptorIncrementSize;
			return *this;
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE& Offset(INT offsetScaledByIncrementSize)
		{
			ptr += offsetScaledByIncrementSize;
			return *this;
		}
		inline bool operator==(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other) const
		{
			return (ptr == other.ptr);
		}
		inline bool operator!=(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& other) const
		{
			return (ptr != other.ptr);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE& operator=(const D3D12_GPU_DESCRIPTOR_HANDLE& other)
		{
			ptr = other.ptr;
			return *this;
		}

		inline void InitOffsetted(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
		{
			InitOffsetted(*this, base, offsetScaledByIncrementSize);
		}

		inline void InitOffsetted(_In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
		}

		static inline void InitOffsetted(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetScaledByIncrementSize)
		{
			handle.ptr = base.ptr + offsetScaledByIncrementSize;
		}

		static inline void InitOffsetted(_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& handle, _In_ const D3D12_GPU_DESCRIPTOR_HANDLE& base, INT offsetInDescriptors, UINT descriptorIncrementSize)
		{
			handle.ptr = base.ptr + offsetInDescriptors * descriptorIncrementSize;
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

//const def
namespace platform_ex::Windows::D3D12 {
	// DX12 doesn't support higher MSAA count
	constexpr unsigned DX_MAX_MSAA_COUNT = 8;

	// This is a value that should be tweaked to fit the app, lower numbers will have better performance
	// Titles using many terrain layers may want to set MAX_SRVS to 64 to avoid shader compilation errors. This will have a small performance hit of around 0.1%
	constexpr unsigned MAX_SRVS = 48;
	constexpr unsigned MAX_SAMPLERS = 16;
	constexpr unsigned MAX_UAVS = 16;
	constexpr unsigned MAX_CBS = 16;

	// This value controls how many root constant buffers can be used per shader stage in a root signature.
	// Note: Using root descriptors significantly increases the size of root signatures (each root descriptor is 2 DWORDs).
	constexpr unsigned MAX_ROOT_CBVS = MAX_CBS;

	constexpr DXGI_COLOR_SPACE_TYPE DXGI_HDR_ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;

	using CBVSlotMask = leo::make_width_int<MAX_CBS>::unsigned_least_type;

	constexpr CBVSlotMask GRootCBVSlotMask = (1 << MAX_ROOT_CBVS) - 1; // Mask for all slots that are used by root descriptors.
	constexpr CBVSlotMask GDescriptorTableCBVSlotMask = static_cast<CBVSlotMask>(-1) & ~(GRootCBVSlotMask); // Mask for all slots that are used by a root descriptor table.

	using SRVSlotMask = leo::make_width_int<MAX_SRVS>::unsigned_fast_type;

	using SamplerSlotMask = leo::make_width_int<MAX_SAMPLERS>::unsigned_type;

	using UAVSlotMask = leo::make_width_int<MAX_UAVS>::unsigned_type;
}

#define USE_PIX 1

#define ENABLE_AFTER_MATH 1

#endif