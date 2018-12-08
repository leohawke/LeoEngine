/*! \file Engine\Asset\EffectAsset.h
\ingroup Engine
\brief EFFECT Infomation ...
*/
#ifndef LE_ASSET_EFFECT_ASSET_H
#define LE_ASSET_EFFECT_ASSET_H 1

#include <LBase/lmacro.h>
#include <LBase/linttype.hpp>
#include <LBase/any.h>

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <variant>

#include "../Render/PipleState.h"
#include "../Render/Effect/Effect.hpp"

namespace asset {


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

			DefGetter(const lnothrow, const EffectParamType&, ElemType,std::get<EffectParamType>(elem_info))
			DefGetter(lnothrow, EffectParamType&, ElemTypeRef,std::get<EffectParamType>(elem_info))

			DefGetter(const lnothrow, const std::string&, ElemUserType, std::get<std::string>(elem_info))
			DefGetter(lnothrow, std::string&, ElemUserTypeRef, std::get<std::string>(elem_info))

			DefGetter(const lnothrow, const std::variant<EffectParamType LPP_Comma std::string>&, ElemInfo, elem_info)
			DefGetter(lnothrow, std::variant<EffectParamType LPP_Comma std::string>&, ElemInfoRef, elem_info)
	private:
		EffectParamType type;
		leo::uint32 array_size = 0;

		std::variant<EffectParamType,std::string>  elem_info = EPT_ElemEmpty;
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

	class EffectAsset : leo::noncopyable,public  EffectNodeAsset {
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

		template<typename T>
		std::optional<T> GetInfo(const std::string_view& name) const;

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

		enum GenSlot {
			MACRO,
			PARAM,
			CBUFFER,
			FRAGMENT
		};

		void EmplaceShaderGenInfo(GenSlot genslot, leo::uint32 localindex, leo::uint32 genindex) {
			gen_indices.emplace_back(genslot, localindex, genindex);
		}

		void PrepareShaderGen() {
			std::sort(gen_indices.begin(), gen_indices.end(), [](const std::tuple<GenSlot, leo::uint32, leo::uint32>& lhs,const std::tuple<GenSlot, leo::uint32, leo::uint32>& rhs) {
				return std::get<2>(lhs) < std::get<2>(rhs);
			});
		}

	private:
		std::vector<std::tuple<GenSlot, leo::uint32, leo::uint32>> gen_indices;
#endif
	};
}

#endif