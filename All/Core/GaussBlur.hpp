// CopyRight 2014. LeoHawke. All wrongs reserved.

#ifndef Core_GaussBlur_Hpp
#define Core_GaussBlur_Hpp

#include "BaseMacro.h"
#include "memory.hpp"
#include "leoint.hpp"
#include "Singleton.hpp"
#include "..\COM.hpp"



#ifdef LB_IMPL_MSCPP
#pragma once
#endif

struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11ComputeShader;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;

namespace leo
{
	class Camera;
	class effect;
	class LB_API GaussBlur : public leo::Singleton<GaussBlur>
	{
	public:
		~GaussBlur();
	protected:
		GaussBlur(ID3D11Device* device, 
					bool color ,
					uint8	numpasses,
					float	radius,
					const std::pair<uint16, uint16>& size);
		
		
	public:
		void Color();
		void Mono();

		void Passes(uint8 numpasses);

		void Radius(float radius);

		void Size(const std::pair<uint16, uint16>& size);

		void ReCompiler(ID3D11Device* device);

		void Render(ID3D11DeviceContext* context, const Camera& camera);
	public:
		static const std::unique_ptr<GaussBlur>& GetInstance(ID3D11Device* device = nullptr,
			const std::pair<uint16, uint16>& size = { 1280, 768 }, bool color = true,
			uint8	numpasses = 3,
			float	radius = 30.f);
	};
}


#endif