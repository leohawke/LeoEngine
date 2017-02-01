/*! \file Engine\Asset\EffectAsset.h
\ingroup Engine
\brief EFFECT Infomation ...
*/
#ifndef LE_ASSET_EFFECT_ASSET_H
#define LE_ASSET_EFFECT_ASSET_H 1

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <LBase/lmacro.h>
#include <LBase/linttype.hpp>
#include <LBase/any.h>

#include "../Render/PipleState.h"
#include "../Render/Effect/Effect.hpp"

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
		EPT_StructuredBuffer,
		EPT_rwbuffer,
		EPT_rwstructured_buffer,
		EPT_rwtexture1D,
		EPT_rwtexture2D,
		EPT_rwtexture3D,
		EPT_rwtexture1DArray,
		EPT_rwtexture2DArray,
		EPT_AppendStructuredBuffer,
		EPT_ConsumeStructuredBuffer,
		EPT_byteAddressBuffer,
		EPT_rwbyteAddressBuffer,
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

		EPT_ElemEmpty,
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

	class EffectConstantBufferAsset : public EffectNodeAsset {
	public:
		DefGetter(const lnothrow, const std::vector<leo::uint32>&, ParamIndices, indices)
			DefGetter(lnothrow, std::vector<leo::uint32>&, ParamIndicesRef, indices)
	private:
		std::vector<leo::uint32> indices;
	};

	class EffectParameterAsset : public EffectNodeAsset {
	public:
		DefGetter(const lnothrow, const EffectParamType&, Type, type)
			DefGetter(lnothrow, EffectParamType&, TypeRef, type)
			DefGetter(const lnothrow, leo::uint32, ArraySize, array_size)
			DefGetter(lnothrow, leo::uint32&, ArraySizeRef, array_size)

			DefGetter(const lnothrow, const EffectParamType&, ElemType, elem_type)
			DefGetter(lnothrow, EffectParamType&, ElemTypeRef, elem_type)
	private:
		EffectParamType type;
		leo::uint32 array_size = 0;

		EffectParamType elem_type = EPT_ElemEmpty;
	};

	class ShaderFragmentAsset {
	public:
		DefGetter(const lnothrow, const std::string&, Fragment, fragment)
			DefGetter(lnothrow, std::string&, FragmentRef, fragment)

	private:
		std::string fragment;
	};

	using EffectMacro = std::pair<std::string, std::string>;


	class TechniquePassAsset :public EffectNodeAsset {
	public:
		using ShaderType = platform::Render::ShaderCompose::Type;
		DefGetter(const lnothrow, const std::vector<EffectMacro>&, Macros, macros)
			DefGetter(lnothrow, std::vector<EffectMacro>&, MacrosRef, macros)

			DefGetter(const lnothrow, const platform::Render::PipleState&, PipleState, piple_state)
			DefGetter(lnothrow, platform::Render::PipleState&, PipleStateRef, piple_state)

			void AssignOrInsertHash(ShaderType type, size_t  blobhash)
		{
			blobindexs.insert_or_assign(type, blobhash);
		}

		size_t GetBlobHash(ShaderType type) const {
			return blobindexs.find(type)->second;
		}

		const std::unordered_map<ShaderType, size_t>& GetBlobs() const lnothrow {
			return blobindexs;
		}
	private:
		platform::Render::PipleState piple_state;
		std::vector<EffectMacro> macros;
		std::unordered_map<ShaderType, size_t> blobindexs;
	};

	class EffectTechniqueAsset : public EffectNodeAsset {
	public:
		DefGetter(const lnothrow, const std::vector<EffectMacro>&, Macros, macros)
			DefGetter(lnothrow, std::vector<EffectMacro>&, MacrosRef, macros)

			DefGetter(const lnothrow, const std::vector<TechniquePassAsset>&, Passes, passes)
			DefGetter(lnothrow, std::vector<TechniquePassAsset>&, PassesRef, passes)
	private:
		std::vector<EffectMacro> macros;
		std::vector<TechniquePassAsset> passes;
	};

	class ShaderBlobAsset :leo::noncopyable {
	public:
		using Type = platform::Render::ShaderCompose::Type;
		using ShaderBlob = platform::Render::ShaderCompose::ShaderBlob;

		explicit ShaderBlobAsset(Type _type, ShaderBlob&& _blob, platform::Render::ShaderInfo* pinfo)
			:type(_type), blob(lforward(_blob)),pInfo(pinfo)
		{}

		ShaderBlobAsset() = default;


		DefGetter(const lnothrow, const Type&, ShaderType, type)

			DefGetter(const lnothrow, const ShaderBlob&, Blob, blob)

			DefGetter(const lnothrow,const platform::Render::ShaderInfo&,Info,*pInfo)
	private:
		Type type;
		ShaderBlob blob;
		std::unique_ptr<platform::Render::ShaderInfo> pInfo;
	};

	class EffectAsset : leo::noncopyable {
	public:
		EffectAsset() = default;


		DefGetter(const lnothrow, const std::vector<EffectMacro>&, Macros, macros)
			DefGetter(lnothrow, std::vector<EffectMacro>&, MacrosRef, macros)
			DefGetter(const lnothrow, const std::vector<EffectConstantBufferAsset>&, CBuffers, cbuffers)
			DefGetter(lnothrow, std::vector<EffectConstantBufferAsset>&, CBuffersRef, cbuffers)

			DefGetter(const lnothrow, const std::vector<EffectParameterAsset>&, Params, params)
			DefGetter(lnothrow, std::vector<EffectParameterAsset>&, ParamsRef, params)

			DefGetter(const lnothrow, const std::vector<ShaderFragmentAsset>&, Fragments, fragements)
			DefGetter(lnothrow, std::vector<ShaderFragmentAsset>&, FragmentsRef, fragements)

			DefGetter(const lnothrow, const std::vector<EffectTechniqueAsset>&, Techniques, techniques)
			DefGetter(lnothrow, std::vector<EffectTechniqueAsset>&, TechniquesRef, techniques)

			const ShaderBlobAsset & GetBlob(size_t blob_index) const {
				return blobs.find(blob_index)->second;
			}

		template<typename... Params>
		void EmplaceBlob(Params&&... params) {
			blobs.emplace(lforward(params)...);
		}

		template<typename T>
		void BindValue(size_t param_index, T&& value) {
			bind_values.emplace_back(param_index, lforward(value));
		}

		std::optional<leo::any> GetValue(size_t param_index) {
			auto iter =  std::find_if(bind_values.begin(), bind_values.end(), [&](const std::pair<size_t, leo::any>& pair) {
				return pair.first == param_index;
			});
			if (iter != bind_values.end())
				return iter->second;
			return std::nullopt;
		}

		std::optional<leo::any> GetInfo(const std::string& name) const;

		static	std::string GetTypeName(EffectParamType type);
		static EffectParamType GetType(const std::string&);
	private:
		std::vector<EffectMacro> macros;
		std::vector<EffectConstantBufferAsset> cbuffers;
		std::vector<EffectParameterAsset> params;
		std::vector<std::pair<size_t, leo::any>> bind_values;
		std::vector<ShaderFragmentAsset> fragements;
		std::vector<EffectTechniqueAsset> techniques;
		std::unordered_map<size_t, ShaderBlobAsset> blobs;
#ifdef ENGINE_TOOL
	public:
		std::string GenHLSLShader() const;
#endif
	};
}

#endif