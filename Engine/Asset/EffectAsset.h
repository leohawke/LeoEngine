/*! \file Engine\Asset\EffectAsset.h
\ingroup Engine
\brief EFFECT Infomation ...
*/
#ifndef LE_ASSET_EFFECT_ASSET_H
#define LE_ASSET_EFFECT_ASSET_H 1

#include <string>
#include <vector>
#include <utility>
#include <LBase/lmacro.h>
#include <LBase/linttype.hpp>

namespace asset {

	enum EffectParamType
	{
		EPT_texture1D,
		EPT_texture2D,
		EPT_texture3D,
		EPT_textureCUBE,
		EPT_texture1DArray,
		EPT_texture2DArray,
		EPT_texture3DArray,
		EPT_textureCUBEArray,
		EPT_buffer,
		EPT_structured_buffer,
		EPT_rw_buffer,
		EPT_rw_structured_buffer,
		EPT_rw_texture1D,
		EPT_rw_texture2D,
		EPT_rw_texture3D,
		EPT_rw_texture1DArray,
		EPT_rw_texture2DArray,
		EPT_append_structured_buffer,
		EPT_consume_structured_buffer,
		EPT_byte_address_buffer,
		EPT_rw_byte_address_buffer,
		EPT_sampler,
		EPT_shader,
		EPT_bool,
		EPT_string,
		EPT_uint,
		EPT_uint2,
		EPT_uint3,
		EPT_uint4,
		EPT_int,
		EPT_int2,
		EPT_int3,
		EPT_int4,
		EPT_float,
		EPT_float2,
		EPT_float2x2,
		EPT_float2x3,
		EPT_float2x4,
		EPT_float3,
		EPT_float3x2,
		EPT_float3x3,
		EPT_float3x4,
		EPT_float4,
		EPT_float4x2,
		EPT_float4x3,
		EPT_float4x4,
	};

	class EffectNodeAsset {
	public:
		DefGetter(const lnothrow, const std::string&, Name, name)
			DefGetter(const lnothrow, std::size_t, NameHash, hash)
			void SetName(const std::string&);
	private:
		std::string name;
		std::size_t hash;
	};

	class EffectConstantBufferAsset : public EffectNodeAsset{
	public:
		DefGetter(const lnothrow, const std::vector<leo::uint32>&, ParamIndices, indices)
			DefGetter(lnothrow, std::vector<leo::uint32>&, ParamIndicesRef, indices)
	private:
		std::vector<leo::uint32> indices;
	};

	class EffectParameterAsset : public EffectNodeAsset {
	public:
		DefGetter(const lnothrow,const EffectParamType& ,Type,type)
			DefGetter( lnothrow,  EffectParamType&, TypeRef, type)
			DefGetter(const lnothrow,leo::uint32,ArraySize,array_size)
			DefGetter(lnothrow, leo::uint32&, ArraySizeRef, array_size)

			DefGetter(const lnothrow, const EffectParamType&, ElemType, elem_type)
			DefGetter(lnothrow, EffectParamType&, ElemTypeRef, elem_type)
	private:
		EffectParamType type;
		leo::uint32 array_size = 0;

		EffectParamType elem_type;
	};

	class ShaderFragmentAsset {
	public:
		DefGetter(const lnothrow,const std::string&,Fragment,fragment)
			DefGetter(lnothrow,std::string&, FragmentRef, fragment)

	private:
		std::string fragment;
	};

	class EffectAsset {
	public:
		using macro_pair = std::pair<std::string, std::string>;

		DefGetter(const lnothrow, const std::vector<macro_pair>&, Macros, macros)
			DefGetter(lnothrow, std::vector<macro_pair>&, MacrosRef, macros)
			DefGetter(const lnothrow, const std::vector<EffectConstantBufferAsset>&, CBuffers, cbuffers)
			DefGetter(lnothrow, std::vector<EffectConstantBufferAsset>&, CBuffersRef, cbuffers)

			DefGetter(const lnothrow, const std::vector<EffectParameterAsset>&, Params, params)
			DefGetter(lnothrow, std::vector<EffectParameterAsset>&, ParamsRef, params)

			DefGetter(const lnothrow, const std::vector<ShaderFragmentAsset>&, Fragments, fragements)
			DefGetter(lnothrow, std::vector<ShaderFragmentAsset>&, FragmentsRef, fragements)


		static	std::string GetTypeName(EffectParamType type);
		static EffectParamType GetType(const std::string&);
	private:
		std::vector<macro_pair> macros;
		std::vector<EffectConstantBufferAsset> cbuffers;
		std::vector<EffectParameterAsset> params;
		std::vector<ShaderFragmentAsset> fragements;
#ifdef ENGINE_TOOL
	public:
		std::string GenHLSLShader() const;
#endif
	};
}

#endif