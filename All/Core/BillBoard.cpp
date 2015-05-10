#include "Effect.h"
#include "BillBoard.hpp"
/*
#include <cstdint>
#include "ldef.h"
#include "IndePlatform\memory.hpp"
#include "Camera.hpp"
#include <cstddef>
#include <string>
#include <map>
#include "leomath.hpp"
#include <comdef.h>
#include <d3d11.h>
#include <d3d11shader.h>
*/

#include	"..\RenderStates.hpp"
#include	 "..\TextureMgr.h"
#include "..\ShaderMgr.h"
#include "Camera.hpp"
#include "Vertex.hpp"
#include "..\exception.hpp"
namespace leo
{
	ID3D11Buffer* BillBoard::mVertexBuffer = nullptr;
	ID3D11InputLayout* BillBoard::mIL = nullptr;


	static int bill_count = 0;

	BillBoard::BillBoard(const float3& center, const float2& size, const wchar_t* filename, ID3D11Device* device)
		:mData({ center, size })
	{
		if (bill_count > 0)
			goto NOINIT;
		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.MiscFlags = 0;
		Desc.StructureByteStride = 0;
		Desc.ByteWidth = sizeof(decltype(mData));
		dxcall(device->CreateBuffer(&Desc, nullptr, &mVertexBuffer));
	NOINIT:
		++bill_count;
		TextureMgr tm;
		mTexDiffuse = tm.LoadTextureSRV(filename);
	}

	BillBoard::~BillBoard()
	{
		assert(bill_count > 0);
		--bill_count;
		if (bill_count == 0)
			win::ReleaseCOM(mVertexBuffer);
	}

	void BillBoard::Render(ID3D11DeviceContext* context, const Camera& camera)
	{
		auto center = load(mData.mCenterW);
		auto result = camera.Contains(center);
		if (camera.Contains(center) != CONTAINMENT_TYPE::DISJOINT)
			return;
		//¸üÐÂVB
		D3D11_MAPPED_SUBRESOURCE mapSubRes;
		try{
			dxcall(context->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD,0, &mapSubRes));
		}
		Catch_DX_Exception

		std::memcpy(mapSubRes.pData, &mData, sizeof(mData));
		context->Unmap(mVertexBuffer, 0);

		static UINT strides[] = { sizeof(mData) };
		static UINT offsets[] = { 0 };
		ID3D11Buffer* vbs[] = { mVertexBuffer };

		context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::BillBoard));

		auto& mEffect = EffectSprite::GetInstance();

		float3 eyepos;
		leo::memcpy(eyepos, camera.GetOrigin());
		mEffect->EyePos(eyepos);
		mEffect->ViewProj(camera.ViewProj());
		mEffect->Apply(context);
		context->PSSetShaderResources(0, 1, &mTexDiffuse);

		
		context->Draw(1, 0);

	}
}