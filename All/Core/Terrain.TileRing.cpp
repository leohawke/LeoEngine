#include "..\IndePlatform\platform.h"
#include "..\d3dx11.hpp"

#include "Terrain.Tessation.Effect.hpp"
#include "Terrain.TileRing.h"
#include "..\ShaderMgr.h"
namespace leo
{
	namespace Vertex
	{
		struct InstanceData
		{
			float2 position;
			Adjacency adjacency;
			//int32 VertexId;
			//int32 InstanceId;
		};
	}

	TileRing::TileRing(ID3D11Device* device, int holeWidth, int outerWidth, float tileSize)
		:mHoleWidth(holeWidth), mOuterWidth(outerWidth), 
		mRingWidth((outerWidth-holeWidth)/2),
		mnTiles(outerWidth*outerWidth-holeWidth*holeWidth),
		mtileSize(tileSize)
	{
		assert((outerWidth - holeWidth) % 2 == 0);
		CreateInstanceDataVB(device);
	}

	TileRing::~TileRing()
	{
		leo::win::ReleaseCOM(mPositionsVB);
	}

	bool TileRing::InRing(int x, int y) const
	{
		assert(x >= 0 && x < mOuterWidth);
		assert(y >= 0 && y < mOuterWidth);
		return x < mRingWidth || y < mRingWidth ||
			x >= mOuterWidth - mRingWidth || y >= mOuterWidth - mRingWidth;
	}

	void TileRing::AssignNeighbourSizes(int x, int y, Vertex::Adjacency& adj) const
	{
		adj.neighbourPlusX = 1.f;
		adj.neighbourPlusY = 1.f;
		adj.neighbourMinusX = 1.f;
		adj.neighbourMinusY = 1.f;

		// TBD: these aren't necessarily 2x different.  Depends on the relative tiles sizes supplied to ring ctors.
		const float innerNeighbourSize = 0.5f;
		const float outerNeighbourSize = 2.0f;

		// Inner edges abut tiles that are smaller.  (But not on the inner-most.)
		if (mHoleWidth > 0)
		{
			if (y >= mRingWidth && y < mOuterWidth - mRingWidth)
			{
				if (x == mRingWidth - 1)
					adj.neighbourPlusX = innerNeighbourSize;
				if (x == mOuterWidth - mRingWidth)
					adj.neighbourMinusX = innerNeighbourSize;
			}
			if (x >= mRingWidth && x < mOuterWidth - mRingWidth)
			{
				if (y == mRingWidth - 1)
					adj.neighbourPlusY = innerNeighbourSize;
				if (y == mOuterWidth - mRingWidth)
					adj.neighbourMinusY = innerNeighbourSize;
			}
		}

		// Outer edges abut tiles that are larger.  We could skip this on the outer-most ring.  But it will
		// make almost zero visual or perf difference.
		if (x == 0)
			adj.neighbourMinusX = outerNeighbourSize;
		if (y == 0)
			adj.neighbourMinusY = outerNeighbourSize;
		if (x == mOuterWidth - 1)
			adj.neighbourPlusX = outerNeighbourSize;
		if (y == mOuterWidth - 1)
			adj.neighbourPlusY = outerNeighbourSize;
	}

	void TileRing::CreateInstanceDataVB(ID3D11Device* device)
	{
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));

		int index = 0;
		mVBData.resize(mnTiles);

		const float halfWidth = 0.5f * (float)mOuterWidth;
		for (int y = 0; y < mOuterWidth; ++y)
		{
			for (int x = 0; x < mOuterWidth; ++x)
			{
				if (InRing(x, y))
				{
					mVBData[index].position.x = mtileSize * ((float)x - halfWidth);
					mVBData[index].position.y = mtileSize * ((float)y - halfWidth);
					AssignNeighbourSizes(x, y,mVBData[index].adjacency);
					index++;
				}
			}
		}
		assert(index == mnTiles);

		initData.pSysMem = mVBData.data();
		D3D11_BUFFER_DESC vbDesc =
		{
			sizeof(Vertex::InstanceData) * mnTiles,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0
		};

		try{
			dxcall(device->CreateBuffer(&vbDesc, &initData, &mPositionsVB));
		}
		Catch_DX_Exception
	}

	void TileRing::IASet(ID3D11DeviceContext* context) const
	{
		UINT uiVertexStrides[] = { sizeof(Vertex::InstanceData) };
		UINT uOffsets[] = { 0 };

		context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::Terrain));
		context->IASetVertexBuffers(0, 1, &mPositionsVB, uiVertexStrides, uOffsets);
	}
}