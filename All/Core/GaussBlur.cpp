#include "..\d3dx11.hpp"
#include "..\DeviceMgr.h"
#include "..\ShaderMgr.h"
#include "GaussBlur.hpp"
#include "Effect.h"
#include "Vertex.hpp"
#include "..\exception.hpp"
#include "..\IndePlatform\Geometry.hpp"

#include <d3dcompiler.h>
#pragma warning(disable: 4172)
namespace leo
{
	static float CalculateBoxFilterWidth(float radius, uint8 numpasses)
	{
		// Calculate standard deviation according to cutoff width

		// We use sigma*3 as the width of filtering window
		float sigma = radius / 3.0f;

		// The width of the repeating box filter
		float box_width = sqrt(sigma * sigma * 12.0f / numpasses + 1);

		return box_width;
	}


	//注意.行将在UAV上读而且写,所以并不需要"乒乓"资源,也能达到<将第一个输出作为第二个的输入>
	//并作为输出

	//复制的Dst
	static ID3D11Texture2D* mInTexture = nullptr;
	//由RenderTarget创建的SRV
	//CS的输入<列处理时的输入>
	static ID3D11ShaderResourceView* mRenderTargetSRV = nullptr;
	//渲染时所使用,与mInOutUAV指向同一份资源
	static ID3D11ShaderResourceView* mInSRV = nullptr;
	//CS的输入和输出<列处理时的输出,暗用作行处理的输入,实用作行处理的输出>
	static ID3D11UnorderedAccessView* mInOutUAV = nullptr;

	static ID3D11ComputeShader* mRowCS = nullptr;
	static ID3D11ComputeShader* mColCS = nullptr;


	static ID3D11Buffer* mVertexBuffer = nullptr;
	static ID3D11Buffer* mCSConstantBuffer = nullptr;

	static bool mColor = true;

	static uint8	mNumPasses = 3;
	static float	mRadius = 30.f;
	static std::pair<uint16, uint16> mSize;


	struct CSParams
	{
		float4 box_params;
		uint32 num_pass_parm;
	};


	void GaussBlur::Color()
	{
		mColor = true;
	}
	void GaussBlur::Mono()
	{
		mColor = false;
	}

	void GaussBlur::Passes(uint8 numpasses)
	{
		mNumPasses = numpasses;
	}

	void GaussBlur::Radius(float radius)
	{
		mRadius = radius;
	}

	void GaussBlur::Size(const std::pair<uint16, uint16>& size)
	{
		mSize = size;
	}

	void GaussBlur::ReCompiler(ID3D11Device* device)
	{
		CD3D11_TEXTURE2D_DESC tex2d(DXGI_FORMAT_R8G8B8A8_UNORM, mSize.first, mSize.second);

		leo::win::ReleaseCOM(mInTexture);
		leo::win::ReleaseCOM(mRenderTargetSRV);
		leo::win::ReleaseCOM(mInSRV);
		leo::win::ReleaseCOM(mInOutUAV);
		leo::win::ReleaseCOM(mColCS);
		leo::win::ReleaseCOM(mRowCS);

		tex2d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tex2d.Usage = D3D11_USAGE_DEFAULT;
		tex2d.MipLevels = 1;
		//tex2d.SampleDesc.Count = 1;
		dxcall(device->CreateTexture2D(&tex2d, nullptr, &mInTexture));
		dx::DebugCOM(mInTexture, "GaussBlur::mInTexture");
		CD3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc(D3D11_SRV_DIMENSION_TEXTURE2D);
		resViewDesc.Format = tex2d.Format;


		dxcall(device->CreateShaderResourceView(mInTexture, &resViewDesc, &mRenderTargetSRV));
		dx::DebugCOM(mRenderTargetSRV, "GaussBlur::mRenderTargetSRV");
		//输入是4float
		//UAV的格式是R32_UINT<R11G11B10A2_FLOAT>
		//PS负责打开数据
		tex2d.Format = DXGI_FORMAT_R32_UINT;
		tex2d.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		tex2d.SampleDesc.Count = 1;
		ID3D11Texture2D* mOutTexture = nullptr;
		dxcall(device->CreateTexture2D(&tex2d, nullptr, &mOutTexture));
		dxcall(device->CreateShaderResourceView(mOutTexture, nullptr, &mInSRV));
		dxcall(device->CreateUnorderedAccessView(mOutTexture, nullptr, &mInOutUAV));
		dx::DebugCOM(mOutTexture, "GaussBlur::mOutTexture");
		dx::DebugCOM(mInSRV, "GaussBlur::mInSRV");
		dx::DebugCOM(mInOutUAV, "GaussBlur::mInOutUAV");
		leo::win::ReleaseCOM(mOutTexture);

		D3D_SHADER_MACRO defines[7];

		char str_num_rows[8];
		sprintf(str_num_rows, "%u", tex2d.Height);
		defines[0].Name = "NUM_IMAGE_ROWS";
		defines[0].Definition = str_num_rows;

		char str_num_cols[8];
		sprintf(str_num_cols, "%u", tex2d.Width);
		defines[1].Name = "NUM_IMAGE_COLS";
		defines[1].Definition = str_num_cols;

		DebugPrintf("Height: %s Width: %s \n", str_num_rows, str_num_cols);


		defines[2].Name = "SCAN_COL_PASS";
		defines[2].Definition = "1";

		UINT ThreadPerGroup = mColor ? 128 : 256;

		char str_data_length[8];
		sprintf(str_data_length, "%u", max(tex2d.Height, ThreadPerGroup * 2));
		defines[3].Name = "SCAN_SMEM_SIZE";
		defines[3].Definition = str_data_length;

		// Number of texels per thread handling
		UINT texels_per_thread = (tex2d.Height + ThreadPerGroup - 1) / ThreadPerGroup;
		char str_texels_per_thread[8];
		sprintf(str_texels_per_thread, "%u", texels_per_thread);
		defines[4].Name = "TEXELS_PER_THREAD";
		defines[4].Definition = str_texels_per_thread;

		char str_threads_per_group[8];
		sprintf(str_threads_per_group, "%u", ThreadPerGroup);
		defines[5].Name = "THREADS_PER_GROUP";
		defines[5].Definition = str_threads_per_group;

		defines[6].Name = NULL;
		defines[6].Definition = NULL;

		ID3DBlob* pBlobCS = nullptr;
		ID3DBlob* pErrorBlob = nullptr;

		UINT flags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		HRESULT hr = E_FAIL;

		if (FAILED
			(hr = mColor ?
			D3DCompileFromFile(L"Shader\\Gaussian_color.hlsl", defines, nullptr, "GaussianColor_CS", "cs_5_0", flags, 0, &pBlobCS, &pErrorBlob) :
			D3DCompileFromFile(L"Shader\\Gaussian_mono.hlsl", defines, nullptr, "GaussianMono_CS", "cs_5_0", flags, 0,
			&pBlobCS, &pErrorBlob)
			))
		{
			if (pErrorBlob)
				DebugPrintf((const char*)(pErrorBlob->GetBufferPointer()));
			Raise_DX_Exception(hr)
		}
		dxcall(device->CreateComputeShader(pBlobCS->GetBufferPointer(), pBlobCS->GetBufferSize(), nullptr, &mColCS));
		dx::DebugCOM(mColCS, "GaussBlur::mColCS");
		leo::win::ReleaseCOM(pBlobCS);
		leo::win::ReleaseCOM(pErrorBlob);

		defines[2].Definition = "0";

		sprintf(str_data_length, "%u", max(tex2d.Width, ThreadPerGroup * 2));
		texels_per_thread = (tex2d.Width + ThreadPerGroup - 1) / ThreadPerGroup;
		sprintf(str_texels_per_thread, "%u", texels_per_thread);

		dxcall
			(mColor ?
			D3DCompileFromFile(L"Shader\\Gaussian_color.hlsl", defines, nullptr, "GaussianColor_CS", "cs_5_0", flags, 0, &pBlobCS, &pErrorBlob) :
			D3DCompileFromFile(L"Shader\\Gaussian_mono.hlsl", defines, nullptr, "GaussianMono_CS", "cs_5_0", flags, 0,
			&pBlobCS, &pErrorBlob)
			);

		dxcall(device->CreateComputeShader(pBlobCS->GetBufferPointer(), pBlobCS->GetBufferSize(), nullptr, &mRowCS));
		dx::DebugCOM(mRowCS, "GaussBlur::mRowCS");

		leo::win::ReleaseCOM(pBlobCS);
		leo::win::ReleaseCOM(pErrorBlob);

	}


	GaussBlur::GaussBlur(ID3D11Device* device, bool color,
		uint8	numpasses,
		float	radius,
		const std::pair<uint16, uint16>& size)
	{
		mColor = color;
		mNumPasses = numpasses;
		mRadius = radius;
		mSize = size;
		if (!device)
			return;
		auto& vertices = helper::CreateFullscreenQuad();

		D3D11_BUFFER_DESC vbDesc;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.ByteWidth = vertices.size() * sizeof(Vertex::PostEffect);
		vbDesc.CPUAccessFlags = 0;
		vbDesc.MiscFlags = 0;
		vbDesc.StructureByteStride = 0;
		vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

		D3D11_SUBRESOURCE_DATA subData;
		subData.pSysMem = vertices.data();

		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;
		Desc.StructureByteStride = 0;
		Desc.ByteWidth = sizeof(CSParams);
		try{
			dxcall(device->CreateBuffer(&vbDesc, &subData, &mVertexBuffer));
			dxcall(device->CreateBuffer(&Desc, nullptr, &mCSConstantBuffer));

			dx::DebugCOM(mVertexBuffer, "GaussBlur::mVertexBuffer");
			dx::DebugCOM(mCSConstantBuffer, "GaussBlur::mCSConstantBuffer");

			ReCompiler(device);
		}
		Catch_DX_Exception
	}

	GaussBlur::~GaussBlur()
	{
		leo::win::ReleaseCOM(mVertexBuffer);
		leo::win::ReleaseCOM(mCSConstantBuffer);
		leo::win::ReleaseCOM(mInTexture);
		leo::win::ReleaseCOM(mRenderTargetSRV);
		leo::win::ReleaseCOM(mInSRV);
		leo::win::ReleaseCOM(mInOutUAV);
		leo::win::ReleaseCOM(mColCS);
		leo::win::ReleaseCOM(mRowCS);
	}

	void GaussBlur::Render(ID3D11DeviceContext* context, const Camera& camera)
	{
		DeviceMgr dm;
		auto rtt = dm.GetRenderTargetTexture2D();

		context->CopyResource(mInTexture, rtt);


		float box_width = CalculateBoxFilterWidth(mRadius, mNumPasses);

		CSParams params;
		//g_HalfBoxFilterWidth
		params.box_params.x = box_width*0.5f;
		//g_InvFracHalfBoxFilterWidthf
		params.box_params.y = (params.box_params.x + 0.5f) - (int)(params.box_params.x + 0.5f);
		//g_InvFracHalfBoxFilterWidthf
		params.box_params.z = 1.f - params.box_params.y;
		//g_RcpBoxFilterWidth
		params.box_params.w = 1.f / box_width;
		params.num_pass_parm = mNumPasses;

		context->UpdateSubresource(mCSConstantBuffer, 0, nullptr, &params, 0, 0);


		context->VSSetShader(nullptr, nullptr, 0);
		context->PSSetShader(nullptr, nullptr, 0);

		//列 ,col
		context->CSSetShader(mColCS, nullptr, 0);
		context->CSSetConstantBuffers(0, 1, &mCSConstantBuffer);
		context->CSSetShaderResources(0, 1, &mRenderTargetSRV);
		context->CSSetUnorderedAccessViews(0, 1, &mInOutUAV, nullptr);

		context->Dispatch(mSize.first, 1, 1);


		// Unbound CS resource and output
		ID3D11ShaderResourceView* srv_array[] = { nullptr, nullptr, nullptr, nullptr };
		context->CSSetShaderResources(0, 4, srv_array);
		ID3D11UnorderedAccessView* uav_array[] = { nullptr, nullptr, nullptr, nullptr };
		context->CSSetUnorderedAccessViews(0, 4, uav_array, nullptr);

		//行, row
		context->CSSetShader(mRowCS, nullptr, 0);
		context->CSSetShaderResources(0, 1, &mRenderTargetSRV);
		context->CSSetUnorderedAccessViews(0, 1, &mInOutUAV, nullptr);

		context->Dispatch(mSize.second, 1, 1);

		// Unbound CS resource and output
		context->CSSetShaderResources(0, 4, srv_array);
		context->CSSetUnorderedAccessViews(0, 4, uav_array, nullptr);

		//渲染
		context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::PostEffect));
		UINT strides[] = { sizeof(Vertex::PostEffect) };
		UINT offsets[] = { 0 };
		context->IASetVertexBuffers(0, 1, &mVertexBuffer, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		auto & mEffect = EffectUnPack::GetInstance();
		mEffect->Apply(context);

		context->PSSetShaderResources(0, 1, &mInSRV);
		context->Draw(4, 0);

		context->PSSetShaderResources(0, 1, srv_array);

	}

	const std::unique_ptr<GaussBlur>& GaussBlur::GetInstance(ID3D11Device* device,
		const std::pair<uint16, uint16>& size, bool color,
		uint8	numpasses,
		float	radius)
	{
		static auto mInstance = unique_raw(new GaussBlur(device, color, numpasses, radius, size));
		return mInstance;
	}

}
