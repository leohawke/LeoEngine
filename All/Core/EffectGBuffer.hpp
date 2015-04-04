////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/EffectGBuffer.hpp
//  Version:     v1.00
//  Created:     3/04/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description:Êä³öGBuffer
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////


#ifndef Core_EffectGBuffer_hpp
#define Core_EffectGBuffer_hpp

#include "effect.h"

namespace leo {
	class EffectGBuffer :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);

		void WorldViewMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);
		void WorldViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);

		void LM_VECTOR_CALL WorldViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr);
		void  LM_VECTOR_CALL WorldViewProjMatrix(std::array<__m128, 4>  matrix, ID3D11DeviceContext* context = nullptr);
		void LM_VECTOR_CALL WorldInvTransposeViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr);

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context = nullptr);
		void NormalsSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context = nullptr);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;

		void OMSetMRT(ID3D11RenderTargetView* rt0, ID3D11RenderTargetView* rt1);
	public:
		static const std::unique_ptr<EffectGBuffer>& GetInstance(ID3D11Device* device = nullptr);
	};

}


#endif