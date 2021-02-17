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

		template<ShaderParamType ShaderType,uint32 NumElements=0>
		struct ShaderTypeInfo
		{
			static constexpr ShaderParamType ShaderType = ShaderType;

			static constexpr ShaderBaseType BaseType = GetBaseType(ShaderType);
			static constexpr uint32 NumRows = GetNumRows(ShaderType);
			static constexpr uint32 NumColumns = GetNumColumns(ShaderType);
			static constexpr uint32 NumElements = NumElements;
		};

		template<>
		struct TShaderParameterTypeInfo<float> : ShaderTypeInfo<SPT_float>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 4;

			using DeclType = float;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float2> : ShaderTypeInfo<SPT_float2>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = Boundary >= 8 ? 4 : 16;

			using DeclType = leo::math::float2;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float3> : ShaderTypeInfo<SPT_float3>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = Boundary >= 12 ? 4 : 16;

			using DeclType = leo::math::float3;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float4> : ShaderTypeInfo<SPT_float4>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 16;

			using DeclType = leo::math::float4;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::float4x4> : ShaderTypeInfo<SPT_float4x4>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 16;

			using DeclType = leo::math::float4x4;
		};

		template<>
		struct TShaderParameterTypeInfo<int> : ShaderTypeInfo<SPT_int>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 4;

			using DeclType = leo::int32;
		};

		template<>
		struct TShaderParameterTypeInfo<unsigned int> : ShaderTypeInfo<SPT_uint>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = 4;

			using DeclType = leo::uint32;
		};

		template<>
		struct TShaderParameterTypeInfo<leo::math::uint2> : ShaderTypeInfo<SPT_uint2>
		{
			template<std::size_t Boundary = 0>
			static constexpr std::size_t Alignement = Boundary >= 8 ? 4 : 16;

			using DeclType = leo::math::uint2;
		};
		
		template<typename TypeParameter>
		struct TShaderTextureTypeInfo;
	}
}

#define INTERNAL_LOCAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName) \
	static platform::Render::ShaderParametersMetadata StaticStructMetadata(sizeof(StructTypeName),\
		StructTypeName::zzGetMembers()); \
	return &StaticStructMetadata;

#define INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName,GetStructMetadataScope) \
	MS_ALIGN(platform::Render::VariableBoundary) class StructTypeName \
	{ \
	public: \
		struct TypeInfo{\
			static constexpr leo::uint32 NumRows = 1;\
			static constexpr leo::uint32 NumColumns =1;\
			static constexpr leo::uint32 NumElements = 0;\
			template<std::size_t Boundary = 0>\
			static constexpr std::size_t Alignement = 16;\
			static inline const platform::Render::ShaderParametersMetadata* GetStructMetadata() { GetStructMetadataScope } \
		};\
	private: \
		typedef StructTypeName zzThisStruct; \
		struct zzFirstMemberId { enum { Boundary = 16 }; }; \
		typedef void* zzFuncPtr; \
		typedef zzFuncPtr(*zzMemberFunc)(zzFirstMemberId, std::vector<platform::Render::ShaderParametersMetadata::Member>*); \
		static zzFuncPtr zzAppendMemberGetPrev(zzFirstMemberId, std::vector<platform::Render::ShaderParametersMetadata::Member>*) \
		{ \
			return nullptr; \
		} \
		typedef zzFirstMemberId

#define INTERNAL_SHADER_PARAMETER_EXPLICIT(ShaderType,TypeInfo,MemberType,MemberName) \
	zzMemberId##MemberName; \
	public: \
		alignas(TypeInfo::Alignement<zzMemberId##MemberName::Boundary>) TypeInfo::DeclType MemberName {}; \
	private: \
		struct zzNextMemberId##MemberName \
			{enum { Boundary =zzMemberId##MemberName::Boundary <sizeof(MemberName) ? \
				platform::Render::VariableBoundary:\
				zzMemberId##MemberName::Boundary-sizeof(MemberName) }; \
			}; \
		static zzFuncPtr zzAppendMemberGetPrev(zzNextMemberId##MemberName, std::vector<platform::Render::ShaderParametersMetadata::Member>* Members) \
		{ \
			Members->emplace_back(\
				#MemberName,\
				static_cast<leo::uint32>(loffsetof(zzThisStruct,MemberName)),\
				ShaderType, \
				TypeInfo::BaseType,\
				TypeInfo::NumRows,\
				TypeInfo::NumColumns,\
				TypeInfo::NumElements\
			);\
			zzFuncPtr(*PrevFunc)(zzMemberId##MemberName, std::vector<platform::Render::ShaderParametersMetadata::Member>*); \
			PrevFunc = zzAppendMemberGetPrev; \
			return (zzFuncPtr)PrevFunc; \
		}\
		typedef zzNextMemberId##MemberName

#define END_SHADER_PARAMETER_STRUCT() \
	zzLastMemberId; \
	static std::vector<platform::Render::ShaderParametersMetadata::Member> zzGetMembers() { \
			std::vector<platform::Render::ShaderParametersMetadata::Member> Members; \
			zzFuncPtr(*LastFunc)(zzLastMemberId, std::vector<platform::Render::ShaderParametersMetadata::Member>*); \
			LastFunc = zzAppendMemberGetPrev; \
			zzFuncPtr Ptr = (zzFuncPtr)LastFunc; \
			do \
			{ \
				Ptr = reinterpret_cast<zzMemberFunc>(Ptr)(zzFirstMemberId(), &Members); \
			} while (Ptr); \
			return Members; \
		} \
	};

#define BEGIN_SHADER_PARAMETER_STRUCT(StructTypeName) \
		INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName,INTERNAL_LOCAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName))

/** Adds a constant-buffer stored value.
 *
 * Example:
 *	SHADER_PARAMETER(float, MyScalar)
 *	SHADER_PARAMETER(float4x4, MyMatrix)
 */
#define SHADER_PARAMETER(MemberType,MemberName) \
		SHADER_PARAMETER_EX(MemberType,MemberName)

#define SHADER_PARAMETER_EX(MemberType,MemberName) \
		INTERNAL_SHADER_PARAMETER_EXPLICIT(platform::Render::TShaderParameterTypeInfo<MemberType>::ShaderType, platform::Render::TShaderParameterTypeInfo<MemberType>,MemberType,MemberName)

 /** Adds a texture.
  *
  * Example:
  *	SHADER_PARAMETER_TEXTURE(Texture2D, MyTexture)
  */
#define SHADER_PARAMETER_TEXTURE(MemberType,MemberName) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(platform::Render::TShaderTextureTypeInfo<MemberType>::ShaderType, platform::Render::TShaderTextureTypeInfo<MemberType>, ShaderType*,MemberName)

  /** Adds a sampler.
   *
   * Example:
   *	SHADER_PARAMETER_SAMPLER(SamplerState, MySampler)
   */
#define SHADER_PARAMETER_SAMPLER(MemberType,MemberName) SHADER_PARAMETER(MemberType,MemberName)