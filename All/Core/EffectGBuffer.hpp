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
#include "CoreObject.hpp"

namespace leo {
	class EffectGBuffer :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);

		void WorldViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context = nullptr);

		void  LM_VECTOR_CALL WorldViewProjMatrix(std::array<__m128, 4>  matrix, ID3D11DeviceContext* context = nullptr);
		void LM_VECTOR_CALL InvTransposeWorldViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr);

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context = nullptr);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;

		void Specular(const float4& specular_power, ID3D11DeviceContext* context = nullptr);
	public:
		static const std::unique_ptr<EffectGBuffer>& GetInstance(ID3D11Device* device = nullptr);
	};

	class EffectSkinGBuffer :public Effect, ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);

		void SkinMatrix(float4x4Object* globalmatrix, std::uint32_t numJoint);

		void  LM_VECTOR_CALL WorldViewProjMatrix(std::array<__m128, 4>  matrix, ID3D11DeviceContext* context = nullptr);
		void LM_VECTOR_CALL WorldMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr);
		void LM_VECTOR_CALL ViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr);

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context = nullptr);
		void NormalSRV(ID3D11ShaderResourceView * const normal, ID3D11DeviceContext* context = nullptr);

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;

		void Specular(const float4& specular_power, ID3D11DeviceContext* context = nullptr);
	public:
		static std::unique_ptr<EffectSkinGBuffer>& GetInstance(ID3D11Device* device = nullptr);
	};

}


#endif