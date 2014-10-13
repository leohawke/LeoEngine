// CopyRight 2014. LeoHawke. All wrongs reserved.

#ifndef Core_BillBoard_Hpp
#define Core_BillBoard_Hpp

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "CoreObject.hpp"
/*
#include <cstdint>
#include "..\leomath.hpp"
#include "ldef.h"
#include "..\IndePlatform\memory.hpp"
#include "Camera.hpp"
*/

#include "..\d3dx11.hpp"
/*
#include <cstddef>
#include <cstdint>
#include <string>
#include <map>
#include "leomath.hpp"
#include <comdef.h>
#include <d3d11.h>
#include <d3d11shader.h>
*/

#include "..\LightBuffer.h"

namespace leo
{
	class effect;
	class Camera;
	//[unsafe]
	class BillBoard
	{
	private:
		static ID3D11Buffer* mVertexBuffer;

		static ID3D11InputLayout* mIL;
	private:
		ID3D11ShaderResourceView* mTexDiffuse = nullptr;
	private:
		struct
		{
			float3 mCenterW;
			float2 mSizeW;
		} mData;

	public:
		BillBoard(const float3& center, const float2& size,const wchar_t* filename, ID3D11Device* device);

		BillBoard(const float3& center, const float2& size, const std::wstring& filename, ID3D11Device* device)
			:BillBoard(center, size, filename.c_str(), device)
		{}

		~BillBoard();

		void Render(ID3D11DeviceContext* context, const Camera& camera);
	};
}

#endif