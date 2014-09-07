#include "GpuTerrain.hpp"
#include "..\Mgr.hpp"
#include "..\COM.hpp"
#include "..\exception.hpp"
#include "..\file.hpp"
#include "..\RenderStates.hpp"
#include "..\D3DX11\D3DX11.h"
#include "Camera.hpp"
#include <fstream>

namespace leo
{
	const static std::size_t strides[] = { sizeof(GpuTerrain::Vertex) };
	const static std::size_t offsets[] = { 0 };

	const static D3D11_INPUT_ELEMENT_DESC inputdescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	const static wchar_t* volnames[] =
	{
		L"Shader\\noise_half_16cubed_mips_00.raw",
		L"Shader\\noise_half_16cubed_mips_01.vol",
		L"Shader\\noise_half_16cubed_mips_02.vol",
		L"Shader\\noise_half_16cubed_mips_03.vol",
		L"Shader\\packednoise_half_16cubed_mips_00.vol",
		L"Shader\\packednoise_half_16cubed_mips_01.vol",
		L"Shader\\packednoise_half_16cubed_mips_02.vol",
		L"Shader\\packednoise_half_16cubed_mips_03.vol",
	};

	static ID3D11ShaderResourceView* LoadVolFile(ID3D11Device* device, const wchar_t * filename)
	{
		leo::win::File fin(filename, true, false, false);
		const auto buffersize = (16 * 16 * 16 + 8 * 8 * 8 + 4 * 4 * 4 + 2 * 2 * 2 + 1) * 4;
		std::uint16_t buffer[buffersize] = { 0 };
		//std::ifstream fin(filename,std::ios_base::binary);
		//fin.read(reinterpret_cast<char*>(buffer), buffersize);
		auto readsize = fin.Read(buffer, 5 * 4, buffersize * 2);
		D3D11_TEXTURE3D_DESC densityDesc;
		densityDesc.Width = densityDesc.Height = densityDesc.Depth = 16;
		densityDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		densityDesc.MipLevels = 5;
		densityDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		densityDesc.Usage = D3D11_USAGE_IMMUTABLE;
		densityDesc.CPUAccessFlags = 0;
		densityDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA densityData[5];
		densityData[0].pSysMem = buffer;
		densityData[0].SysMemPitch = 16 * 4 * 2;
		densityData[0].SysMemSlicePitch = 16 * 16 * 4 * 2;

		densityData[1].pSysMem = buffer + 16 * 16 * 4;
		densityData[1].SysMemPitch = 8 * 4 * 2;
		densityData[1].SysMemSlicePitch = 8 * 8 * 4 * 2;

		densityData[2].pSysMem = buffer + (16 * 16 + 8 * 8) * 4;
		densityData[2].SysMemPitch = 4 * 4 * 2;
		densityData[2].SysMemSlicePitch = 4 * 4 * 4 * 2;

		densityData[3].pSysMem = buffer + (16 * 16 + 8 * 8 + 4 * 4) * 4;
		densityData[3].SysMemPitch = 2 * 4 * 2;
		densityData[3].SysMemSlicePitch = 2 * 2 * 4 * 2;

		densityData[4].pSysMem = buffer + (16 * 16 + 8 * 8 + 4 * 4 + 2 * 2) * 4;
		densityData[4].SysMemPitch = 1 * 4 * 2;
		densityData[4].SysMemSlicePitch = 1 * 1 * 4 * 2;

		ID3D11Texture3D* vol = nullptr;

		dxcall(device->CreateTexture3D(&densityDesc, densityData, &vol));
		ID3D11DeviceContext * context = nullptr;
		device->GetImmediateContext(&context);
		static bool savefalg = false;
		if (!savefalg)
			dxcall(D3DX11SaveTextureToFileA(context, vol, D3DX11_IFF_DDS, "vol.dds"));
		savefalg = true;
		ID3D11ShaderResourceView* volrsv = nullptr;
		dxcall(device->CreateShaderResourceView(vol, nullptr, &volrsv));
		leo::win::ReleaseCOM(vol);
		return volrsv;
	}

	void GpuTerrain::Init(ID3D11Device* device)
	{
		ShaderMgr shadermgr;
		m_bdvs = shadermgr.CreateVertexShader(L"Shader\\BuildDensityVS.cso", nullptr, inputdescs, 1, &m_inputlayout);
		m_bdgs = shadermgr.CreateGeometryShader(L"Shader\\BuildDensityGS.cso");
		m_bdps = shadermgr.CreatePixelShader(L"Shader\\BuildDensityPS.cso");

		m_gvgs = shadermgr.CreateGeometryShader(L"Shader\\GenVerticesGS.cso");
		m_gvvs = shadermgr.CreateVertexShader(L"Shader\\GenVerticesVS.cso");//may be can share m_inputlayout

		m_dvvs = shadermgr.CreateVertexShader(L"VertexShader.cso");
		m_dvps = shadermgr.CreatePixelShader(L"PixelShader.cso");

		D3D11_SO_DECLARATION_ENTRY pDecl[] =
		{
			{ 0, "POSITION", 0, 0, 3, 0 }
		};
		auto blob = shadermgr.CreateBlob(L"Shader\\GenVerticesGS.cso");

		dxcall(device->CreateGeometryShaderWithStreamOutput(blob.GetBufferPointer(), blob.GetBufferSize(),
			pDecl, 1, strides, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &m_gvgs));
		//TextureMgr texmgr;
		//m_pixelshader = shadermgr.CreatePixelShader(L"WaterPS.cso");
		try
		{
			//BuildDensityBuffer......
			//the include mRenderTarget,Density,Slice
			{
				D3D11_BUFFER_DESC slicevbDesc;
				slicevbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				slicevbDesc.ByteWidth = sizeof(DirectX::XMFLOAT3) * 4;
				slicevbDesc.CPUAccessFlags = 0;
				slicevbDesc.MiscFlags = 0;
				slicevbDesc.StructureByteStride = 0;
				slicevbDesc.Usage = D3D11_USAGE_IMMUTABLE;

				D3D11_SUBRESOURCE_DATA slicevbResource;

				DirectX::XMFLOAT3 sliceVertices[] =
				{ DirectX::XMFLOAT3(-1.0f, 0.0f, +1.0f), DirectX::XMFLOAT3(+1.0f, 0.0f, +1.0f),
				DirectX::XMFLOAT3(-1.0f, 0.0f, -1.0f), DirectX::XMFLOAT3(+1.0f, 0.0f, -1.0f) };

				slicevbResource.pSysMem = sliceVertices;
				dxcall(device->CreateBuffer(&slicevbDesc, &slicevbResource, &m_bdvb));

				D3D11_BUFFER_DESC sliceibDesc;
				sliceibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				sliceibDesc.ByteWidth = sizeof(UINT)* 6;
				sliceibDesc.CPUAccessFlags = 0;
				sliceibDesc.MiscFlags = 0;
				sliceibDesc.StructureByteStride = 0;
				sliceibDesc.Usage = D3D11_USAGE_IMMUTABLE;

				D3D11_SUBRESOURCE_DATA sliceibResource;

				UINT sliceIndexs[] =
				{
					0, 1, 2,
					2, 1, 3
				};

				sliceibResource.pSysMem = sliceIndexs;
				dxcall(device->CreateBuffer(&sliceibDesc, &sliceibResource, &m_bdib));

				sliceibDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				sliceibDesc.Usage = D3D11_USAGE_DEFAULT;
				sliceibDesc.ByteWidth = sizeof(CBChangerEveryFrame);
				dxcall(device->CreateBuffer(&sliceibDesc, nullptr, &m_bdvscb));

				sliceibDesc.ByteWidth = sizeof(CBChangeOnLod);
				dxcall(device->CreateBuffer(&sliceibDesc, nullptr, &m_bdpscb));

				D3D11_TEXTURE3D_DESC densityDesc;
				densityDesc.Width = 33;
				densityDesc.Height = 33;
				densityDesc.Depth = 33;
				densityDesc.Format = DXGI_FORMAT_R32_FLOAT;
				densityDesc.MipLevels = 1;
				densityDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
				densityDesc.Usage = D3D11_USAGE_DEFAULT;
				densityDesc.CPUAccessFlags = 0;
				densityDesc.MiscFlags = 0;

				dxcall(device->CreateTexture3D(&densityDesc, nullptr, &m_bdrtvbuffer));
				dxcall(device->CreateRenderTargetView(m_bdrtvbuffer, nullptr, &m_bdrtv));

				densityDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				dxcall(device->CreateTexture3D(&densityDesc, nullptr, &mDensityTex));
				dxcall(device->CreateShaderResourceView(mDensityTex, nullptr, &mDensityRes));

				for (std::uint8_t i = 0; i != 8; ++i)
					Vol[i] = LoadVolFile(device, volnames[i]);
			};
			//BuildGenVerteicBuffer......
			//the include src,streamoutput
			//GenVertices and Draw!
			D3D11_BUFFER_DESC buildvbDesc;
			buildvbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buildvbDesc.ByteWidth = sizeof(DirectX::XMFLOAT3);
			buildvbDesc.CPUAccessFlags = 0;
			buildvbDesc.MiscFlags = 0;
			buildvbDesc.StructureByteStride = 0;
			buildvbDesc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA buildvbResource;
			XMFLOAT3 pos = XMFLOAT3(0.f, 0.f, 0.f);
			buildvbResource.pSysMem = &pos;
			dxcall(device->CreateBuffer(&buildvbDesc, &buildvbResource, &m_gvvb));

			//StreamOutput
			D3D11_BUFFER_DESC genvbDesc;
			genvbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
			genvbDesc.ByteWidth = sizeof(DirectX::XMFLOAT3) * 32 * 32 * 32 * 15;
			genvbDesc.CPUAccessFlags = 0;
			genvbDesc.MiscFlags = 0;
			genvbDesc.StructureByteStride = 0;
			genvbDesc.Usage = D3D11_USAGE_DEFAULT;
			dxcall(device->CreateBuffer(&genvbDesc, nullptr, &m_gvsovb));

			genvbDesc.BindFlags = 0;
			genvbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			genvbDesc.Usage = D3D11_USAGE_STAGING;
			dxcall(device->CreateBuffer(&genvbDesc, nullptr, &m_gvsodebugvb));

			D3D11_QUERY_DESC queryDesc;
			queryDesc.MiscFlags = 0;
			queryDesc.Query = D3D11_QUERY_SO_STATISTICS;
			dxcall(device->CreateQuery(&queryDesc, &StreamOutQuery));

			RenderStates sss;
			m_bdpsss[0] = sss.GetSamplerState(L"LinearRepeat");
			m_bdpsss[1] = sss.GetSamplerState(L"NearestClamp");
			m_bdpsss[2] = sss.GetSamplerState(L"NearestRepeat");
			m_bdpsss[3] = sss.GetSamplerState(L"LinearClamp");

			m_dvrs = sss.GetRasterizerState(L"NoCullRS");
			m_dvdss = sss.GetDepthStencilState(L"DefaultDSS");
		}
		Catch_DX_Exception
	}

	void GpuTerrain::Draw(ID3D11DeviceContext* context, const Camera& camera)
	{
		D3D11_QUERY_DATA_SO_STATISTICS  soQueryResult;
		static bool save = true;
		for (auto x = 0; x != 5; ++x)
			for (auto z = 0; z != 5; ++z)
			{
				auto world = XMMatrixTranslation((float)x, (float)0, (float)z);
				BuildDensity(context, world);
				context->CopyResource(mDensityTex, m_bdrtvbuffer);
				GenVertices(context, world);
				
				while (context->GetData(StreamOutQuery, &soQueryResult, sizeof(D3D11_QUERY_DATA_SO_STATISTICS), 0) != S_OK)
					;
				if (soQueryResult.NumPrimitivesWritten > 0)
					DrawVertices(context, camera.ViewProj());
			}
#define DEBUGSO 0
#if defined _DEBUG || defined DEBUG
		if (!save){
			D3DX11SaveTextureToFileA(context, mDensityTex, D3DX11_IFF_DDS, "density.dds");
			context->CopyResource(m_gvsodebugvb, m_gvsovb);
			D3D11_MAPPED_SUBRESOURCE densityData;
			context->Map(m_gvsodebugvb, 0, D3D11_MAP_READ, 0, &densityData);
			auto pdensity = reinterpret_cast<XMFLOAT3*>(densityData.pData);
			std::ofstream fd("vertices.txt");
#if DEBUGSO
			auto casecount = 0;
			for (UINT i = 0; i != 32*32*32; ++i)
			{
				if (pdensity[i * 3].x != 0.f && pdensity[i * 3].x != 255.f)
				{
					casecount++;
					
					fd << "Case: " << pdensity[i * 3].x;
					fd << std::endl;
					fd << "Density: " << pdensity[i * 3].y << ' ' << pdensity[i * 3].z << ' ' << pdensity[i * 3 + 1].x << ' ' << pdensity[i * 3 + 1].y << ' '
					<< pdensity[i * 3 + 1].z << ' ' << pdensity[i * 3 + 2].x << ' ' << pdensity[i * 3 + 2].y << ' ' << pdensity[i * 3 + 2].z;
					fd << std::endl;
					
				}
			}
			fd<<"CaseCount: "<<casecount;
#else
			
				;
			for (UINT i = 0; i != max((UINT64)32, soQueryResult.NumPrimitivesWritten); ++i)
			{
				auto data = pdensity[i*3];
				fd << "Tri: " << i;
				fd << std::endl;
				fd << data.x << ' ' << data.y << ' ' << data.z << ' ';
				data = pdensity[i * 3 + 1];
				fd << data.x << ' ' << data.y << ' ' << data.z << ' ';
				data = pdensity[i * 3 + 2];
				fd << data.x << ' ' << data.y << ' ' << data.z << ' ';
				fd << std::endl;
			}
#endif
			context->Unmap(m_gvsodebugvb,0);
		}
#endif
		//DrawVertices(context, camera.ViewProj());
		if (!save)
			try{
#if defined _DEBUG || defined DEBUG
			//dxcall(D3DX11SaveTextureToFileA(context, mDensityTex, D3DX11_IFF_DDS, "densityd.dds"));
#else
			//dxcall(D3DX11SaveTextureToFileA(context, mDensityTex, D3DX11_IFF_DDS, "densityr.dds"));
#endif
		}
			Catch_DX_Exception
		save = true;
		/*
		static bool save = false;
		dx::profileblock pb(L"GPU Terrain");
		for (int x = 0; x != 5; ++x)
			for (int y = 0; y != 5; ++y)
				for (int z = 0; z != 5; ++z)
				{
					auto world = XMMatrixTranslation((float)x, (float)y, (float)z);
					BuildDensity(context, world);
					context->CopyResource(mDensityTex, m_bdrtvbuffer);
					char name[260];
#if defined _DEBUG || defined DEBUG
					name[sprintf(name, "%s_%d_%d_%d.dds", "densityd", x, y, z)] = '\0';
#else
					name[sprintf(name, "%s_%d_%d_%d.dds", "densityr", x, y, z)] = '\0';
#endif
					if (!save)
					{
						try{
							dxcall(D3DX11SaveTextureToFileA(context, mDensityTex, D3DX11_IFF_DDS, name));
						}
						Catch_DX_Exception
					}
					//GenVertices(context, world);
					//DrawVertices(context, camera.ViewProj());
				}
		save = true;
		*/
	}

	void GpuTerrain::BuildDensity(ID3D11DeviceContext* context, CXMMATRIX world)
	{
		static ID3D11RenderTargetView* rtv = nullptr;
		static ID3D11DepthStencilView* dsv = nullptr;
		context->OMGetRenderTargets(1, &rtv, &dsv);

		//更新vscb和pscb
		CBChangerEveryFrame vscb;
		vscb.World =XMMatrixTranspose(world);
		context->UpdateSubresource(m_bdvscb, 0, nullptr, &vscb, 0, 0);

		CBChangeOnLod pscb;
		//真是个糟糕的获取随机数的版本,不过更加糟糕的文件系统没有搞定
		pscb.octaveMat6 = XMMatrixTranspose(XMMatrixRotationX((float)rand() / RAND_MAX));
		pscb.octaveMat7 = XMMatrixTranspose(XMMatrixRotationZ((float)rand() / RAND_MAX));
		//fuck,这个需要传入摄像机才能计算,算了,暂时扔在这里
		pscb.wsChunkSize = 4.f;
		context->UpdateSubresource(m_bdpscb, 0, nullptr, &pscb, 0, 0);

		//IA阶段
		context->IASetIndexBuffer(m_bdib, DXGI_FORMAT_R32_UINT, 0);
		context->IASetInputLayout(m_inputlayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetVertexBuffers(0, 1, &m_bdvb, strides, offsets);

		//OM设置
		CD3D11_VIEWPORT vp;
		vp.Width = 33;
		vp.Height = 33;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.MaxDepth = 1.0f;
		vp.MinDepth = 0.0f;
		UINT numsvp;
		D3D11_VIEWPORT *vps;
		context->RSGetViewports(&numsvp, nullptr);
		vps = new D3D11_VIEWPORT[numsvp];
		context->RSGetViewports(&numsvp, vps);
		context->RSSetViewports(1, &vp);
		context->OMSetRenderTargets(1, &m_bdrtv, nullptr);
		static float rgba[4] = {};
		context->ClearRenderTargetView(m_bdrtv, rgba);

		//VS设置
		context->VSSetShader(m_bdvs, nullptr, 0);
		context->VSSetConstantBuffers(0, 1, &m_bdvscb);

		//GS设置
		context->GSSetShader(m_bdgs, nullptr, 0);

		//PS设置
		context->PSSetShader(m_bdps, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &m_bdpscb);
		context->PSSetShaderResources(0, 8, Vol);
		context->PSSetSamplers(0,4, m_bdpsss);

		context->DrawIndexedInstanced(6, 33, 0, 0, 0);

		//反向设置,防止状态泄露
		context->OMSetRenderTargets(1, &rtv, dsv);
		context->RSSetViewports(numsvp, vps);
		delete[] vps;
		context->PSSetShader(nullptr, nullptr, 0);
		context->GSSetShader(nullptr, nullptr, 0);
		context->VSSetShader(nullptr, nullptr, 0);
	}

	void GpuTerrain::GenVertices(ID3D11DeviceContext* context, CXMMATRIX world)
	{
		//流输出设置//唔
		//IA阶段
		context->IASetInputLayout(m_inputlayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		context->IASetVertexBuffers(0, 1, &m_gvvb, strides, offsets);

		//VS设置
		context->VSSetShader(m_gvvs, nullptr, 0);
		context->VSSetShaderResources(0, 1, &mDensityRes);
		context->VSSetSamplers(0, 1, &m_bdpsss[3]);
		context->VSSetConstantBuffers(0, 0, nullptr);
		//GS设置
		CBChangerEveryFrame gscb;
		gscb.World = XMMatrixTranspose(world);
		context->UpdateSubresource(m_bdvscb, 0, nullptr, &gscb, 0, 0);

		context->GSSetShader(m_gvgs,nullptr,0);
		context->GSSetConstantBuffers(0, 1, &m_bdvscb);

		UINT offsets[1] = { 0 };
		context->SOSetTargets(1, &m_gvsovb,offsets);

		context->PSSetShader(nullptr, nullptr, 0);
		context->Begin(StreamOutQuery);
		context->DrawInstanced(1, 32*32*32, 0, 0);
		context->End(StreamOutQuery);
		context->GSSetShader(nullptr, nullptr, 0);
		context->VSSetShader(nullptr, nullptr, 0);
		context->SOSetTargets(0, nullptr, nullptr);
	}

	void GpuTerrain::DrawVertices(ID3D11DeviceContext* context, CXMMATRIX viewproj)
	{
		//IA阶段
		context->IASetInputLayout(m_inputlayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetVertexBuffers(0, 1, &m_gvsovb, strides, offsets);

		//VS设置
		context->VSSetShader(m_dvvs, nullptr, 0);
		CBChangerEveryFrame vscb;
		vscb.World = XMMatrixTranspose(XMMatrixScaling(2,2,2) *viewproj);
		context->UpdateSubresource(m_bdvscb, 0, nullptr, &vscb, 0, 0);
		context->VSSetConstantBuffers(0, 1, &m_bdvscb);

		//PS设置
		context->PSSetShader(m_dvps, nullptr, 0);
		ID3D11RasterizerState* prevRs = nullptr;
		context->RSGetState(&prevRs);
		context->RSSetState(m_dvrs);
		context->OMSetDepthStencilState(m_dvdss, 0);
		context->DrawAuto();

		//状态清除
		context->PSSetShader(nullptr, nullptr, 0);
		context->VSSetShader(nullptr, nullptr, 0);
		context->RSSetState(prevRs);
	}

	GpuTerrain::~GpuTerrain()
	{
		for (auto & v : Vol)
			leo::win::ReleaseCOM(v);
		leo::win::ReleaseCOM(m_bdvb);
		leo::win::ReleaseCOM(m_bdib);
		leo::win::ReleaseCOM(m_bdvb);
		leo::win::ReleaseCOM(m_bdvscb);
		leo::win::ReleaseCOM(m_bdpscb);
		leo::win::ReleaseCOM(m_bdrtvbuffer);
		leo::win::ReleaseCOM(m_bdrtv);

		leo::win::ReleaseCOM(m_gvgs);
		leo::win::ReleaseCOM(m_gvsovb);
		leo::win::ReleaseCOM(m_gvvb);
	}
}