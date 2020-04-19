#pragma once

#include <LBase/lmathtype.hpp>
#include "ShaderParametersMetadata.h"

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
			static constexpr ShaderParamType ShaderType = SPT_float;

			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 4;

			using DeclType = float;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float2>
		{
			static constexpr ShaderParamType ShaderType = SPT_float2;

			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = Boundary >= 8 ? 4 : 16;

			using DeclType = leo::math::float2;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float3>
		{
			static constexpr ShaderParamType ShaderType = SPT_float3;

			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = Boundary >= 12 ? 4 : 16;

			using DeclType = leo::math::float3;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float4>
		{
			static constexpr ShaderParamType ShaderType = SPT_float4;

			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 16;

			using DeclType = leo::math::float4;
		};
	}
}

#define INTERNAL_LOCAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName) \
	static ShaderParametersMetadata StaticStructMetadata(\
		StructTypeName::zzGetMembers()); \
	return &StaticStructMetadata;

#define INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName,GetStructMetadataScope) \
	MS_ALIGN(platform::Render::VariableBoundary) class StructTypeName \
	{ \
	public: \
		struct TypeInfo{\
			static inline const ShaderParametersMetadata* GetStructMetadata() { GetStructMetadataScope } \
		};\
	private: \
		typedef StructTypeName zzThisStruct; \
		struct zzFirstMemberId { enum { Boundary = 16 }; }; \
		typedef void* zzFuncPtr; \
		typedef zzFuncPtr(*zzMemberFunc)(zzFirstMemberId, std::vector<ShaderParametersMetadata::Member>*); \
		static zzFuncPtr zzAppendMemberGetPrev(zzFirstMemberId, std::vector<ShaderParametersMetadata::Member>*) \
		{ \
			return nullptr; \
		} \
		typedef zzFirstMemberId

#define INTERNAL_SHADER_PARAMETER_EXPLICIT(BaseType,TypeInfo,MemberType,MemberName) \
	zzMemberId##MemberName; \
	public: \
		alignas(TypeInfo::Alignement<zzMemberId##MemberName::Boundary>) TypeInfo::DeclType MemberName; \
	private: \
		struct zzNextMemberId##MemberName \
			{enum { Boundary =zzMemberId##MemberName::Boundary <sizeof(MemberName) ? \
				platform::Render::VariableBoundary:\
				zzMemberId##MemberName::Boundary-sizeof(MemberName) }; \
			}; \
		static zzFuncPtr zzAppendMemberGetPrev(zzNextMemberId##MemberName, std::vector<ShaderParametersMetadata::Member>* Members) \
		{ \
			Members->emplace_back(\
				#MemberName,\
				static_cast<leo::uint32>(loffsetof(zzThisStruct,MemberName)),\
				BaseType \
			);\
			zzFuncPtr(*PrevFunc)(zzMemberId##MemberName, std::vector<ShaderParametersMetadata::Member>*); \
			PrevFunc = zzAppendMemberGetPrev; \
			return (zzFuncPtr)PrevFunc; \
		}\
		typedef zzNextMemberId##MemberName

#define END_SHADER_PARAMETER_STRUCT() \
	zzLastMemberId; \
	static std::vector<ShaderParametersMetadata::Member> zzGetMembers() { \
			std::vector<ShaderParametersMetadata::Member> Members; \
			zzFuncPtr(*LastFunc)(zzLastMemberId, std::vector<ShaderParametersMetadata::Member>*); \
			LastFunc = zzAppendMemberGetPrev; \
			zzFuncPtr Ptr = (zzFuncPtr)LastFunc; \
			do \
			{ \
				Ptr = reinterpret_cast<zzMemberFunc>(Ptr)(zzFirstMemberId(), &Members); \
			} while (Ptr); \
			return Members; \
		} \
	}

#define BEGIN_SHADER_PARAMETER_STRUCT(StructTypeName) \
		INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName,INTERNAL_LOCAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName))

#define SHADER_PARAMETER(MemberType,MemberName) \
		SHADER_PARAMETER_EX(MemberType,MemberName)

#define SHADER_PARAMETER_EX(MemberType,MemberName) \
		INTERNAL_SHADER_PARAMETER_EXPLICIT(platform::Render::TShaderParameterTypeInfo<MemberType>::ShaderType, platform::Render::TShaderParameterTypeInfo<MemberType>,MemberType,MemberName)