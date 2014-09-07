// CopyRight 2014. LeoHawke. All wrongs reserved.

#ifndef Core_Sky_Hpp
#define Core_Sky_Hpp

#include "..\IndePlatform\BaseMacro.h"
#include "..\COM.hpp"
#include <string>
#ifdef LB_IMPL_MSCPP
#pragma once
#endif

struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11DeviceContext;

namespace leo
{
	class Camera;
	class effect;

	class LB_API Sky
	{
	public:
		Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius = 5000.f);
		Sky(ID3D11Device* device, ID3D11ShaderResourceView* cubemapSRV, float skySphereRadius = 5000.f);

		DefGetter(const lnothrow, ID3D11ShaderResourceView*, CubeMapSRV, mCubeMapSRV);

		void Render(ID3D11DeviceContext* context, const Camera& camera, effect& eff);

		~Sky()
		{
			leo::win::ReleaseCOM(mVertexBuffer);
		}
	private:
		Sky(ID3D11Device* device, float skySphereRadius = 5000.f);
	private:
		ID3D11Buffer* mVertexBuffer = nullptr;

		ID3D11ShaderResourceView* mCubeMapSRV;

	};
}

#endif