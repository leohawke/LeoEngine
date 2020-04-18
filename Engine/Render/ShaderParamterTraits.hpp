#pragma once

#include <LBase/lmathtype.hpp>

namespace platform::Render
{
	inline namespace Shader
	{
		constexpr std::size_t VariableBoundary = 16;
	}


#define MS_ALIGN(n) __declspec(align(n))
	inline namespace Shader
	{
		template<typename TypeParameter>
		struct TShaderParameterTypeInfo;

		template<>
		struct TShaderParameterTypeInfo<float>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 4;

			using DeclType = float;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float2>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = Boundary >= 8 ? 4 : 16;

			using DeclType = leo::math::float2;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float3>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = Boundary >= 12 ? 4 : 16;

			using DeclType = leo::math::float3;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float4>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 16;

			using DeclType = leo::math::float4;
		};
	}
}

#define INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName) \
	MS_ALIGN(platform::Render::VariableBoundary) class StructTypeName \
	{ \
	public: \
	private: \
		typedef StructTypeName zzThisStruct; \
		struct zzFirstMemberId { enum { Boundary = 16 }; }; \
		typedef zzFirstMemberId

#define INTERNAL_SHADER_PARAMETER_EXPLICIT(TypeInfo,MemberType,MemberName) \
	zzMemberId##MemberName; \
	public: \
		alignas(TypeInfo::Alignement<zzMemberId##MemberName::Boundary>) TypeInfo::DeclType MemberName; \
	private: \
		struct zzNextMemberId##MemberName \
			{enum { Boundary =zzMemberId##MemberName::Boundary <sizeof(MemberName) ? \
				platform::Render::VariableBoundary:\
				zzMemberId##MemberName::Boundary-sizeof(MemberName) }; \
			}; \
		typedef zzNextMemberId##MemberName

#define END_SHADER_PARAMETER_STRUCT() \
	zzLastMemberId; \
	}

#define BEGIN_SHADER_PARAMETER_STRUCT(StructTypeName) \
		INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName)

#define SHADER_PARAMETER(MemberType,MemberName) \
		SHADER_PARAMETER_EX(MemberType,MemberName)

#define SHADER_PARAMETER_EX(MemberType,MemberName) \
		INTERNAL_SHADER_PARAMETER_EXPLICIT(platform::Render::TShaderParameterTypeInfo<MemberType>,MemberType,MemberName)