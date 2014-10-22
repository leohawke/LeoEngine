#include <string>
#include  "..\d3dx11.hpp"
#include "Camera.hpp"
#include "Effect.h"
#include "Sky.hpp"
#include "..\TextureMgr.h"
#include "..\ShaderMgr.h"
#include "Vertex.hpp"
namespace leo
{
	Sky::Sky(ID3D11Device* device, float skySphereRadius)
		:mVertexBuffer(nullptr), mCubeMapSRV(nullptr)
	{
		//maybe ++ 1/(w/h --1/(w/h)
		//NVDIA SDK中QuadScreen坐标不是1,坐标区间扩大了[2/w,2/h]
		float4 Vertices[4] =
		{
			float4(+1.f, +1.f, 1.f, 1.f),
			float4(+1.f, -1.f, 1.f, 1.f),
			float4(-1.f, +1.f, 1.f, 1.f),
			float4(-1.f, -1.f, 1.f, 1.f),
		};
		//TRIANGLESTRIP
		/*
		2-----0
		|     |
		|     |
		3-----1 
		*/

		CD3D11_BUFFER_DESC vb;
		vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vb.ByteWidth = sizeof(Vertices);
		vb.CPUAccessFlags = 0;
		vb.MiscFlags = 0;
		vb.StructureByteStride = 0;
		vb.Usage = D3D11_USAGE_IMMUTABLE;

		D3D11_SUBRESOURCE_DATA vbsubResData;
		vbsubResData.pSysMem = Vertices;

		try{
			dxcall(device->CreateBuffer(&vb, &vbsubResData, &mVertexBuffer));
		}
		Catch_DX_Exception
	}
	Sky::Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius)
		:Sky(device,skySphereRadius)
	{
		TextureMgr mgr;
		mCubeMapSRV = mgr.LoadTextureSRV(cubemapFilename);
		
	}
	Sky::Sky(ID3D11Device* device, ID3D11ShaderResourceView* cubemapSRV, float skySphereRadius)
		: Sky(device, skySphereRadius)
	{
		mCubeMapSRV = (cubemapSRV);
	}
	void Sky::Render(ID3D11DeviceContext* context, const Camera& camera)
	{
		static UINT strides[] = { sizeof(float4) };
		static UINT offsets[] = { 0 };
		ID3D11Buffer* vbs[] = { mVertexBuffer };
		context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::Sky));

		auto & mEffect = EffectSky::GetInstance();
		float3 pos;
		memcpy(pos, camera.GetOrigin());
		auto world = XMMatrixTranslation(pos.x, pos.y, pos.z);
		
		mEffect->EyePos(pos);
		mEffect->ViewProj(world*camera.ViewProj());
		
		context->PSSetShaderResources(0, 1, &mCubeMapSRV);
		mEffect->Apply(context);

		context->Draw(4, 0);
	}
}