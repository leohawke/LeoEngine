#pragma once

#include <LBase/lmacro.h>
#include <LBase/linttype.hpp>
#include <LBase/any.h>

#include <string>
#include <vector>
#include <utility>
#include <variant>

#include "../Render/PipleState.h"
#include "../Render/Effect/Effect.hpp"

namespace asset
{
	using platform::Render::ShaderParamType;
	using platform::Render::ShaderMacro;

	class AssetName {
	public:
		DefGetter(const lnothrow, const std::string&, Name, name)
			DefGetter(const lnothrow, std::size_t, NameHash, hash)
			void SetName(const std::string&);
	private:
		std::string name;
		std::size_t hash;
	};

	class BindDesc
	{
	public:
		enum {Any = -1};

		int GetIndex() const
		{
			return register_index;
		}

		int GetSpace() const
		{
			return register_space;
		}

		void SetIndex(int index)
		{
			register_index =index;
		}

		void SetSpace(int space)
		{
			register_space = space;
		}
	protected:
		int register_index =Any;
		int register_space =Any;
	};

	class ShaderConstantBufferAsset : public AssetName,public BindDesc {
	public:
		DefGetter(const lnothrow, const std::vector<leo::uint32>&, ParamIndices, indices)
			DefGetter(lnothrow, std::vector<leo::uint32>&, ParamIndicesRef, indices)

		DefGetter(const lnothrow, const std::string&, ElemInfo, elem_info)
		DefGetter(lnothrow, std::string&, ElemInfoRef, elem_info)
	private:
		std::vector<leo::uint32> indices;

		std::string elem_info;
	};

	class ShaderParameterAsset : public AssetName, public BindDesc {
	public:
		DefGetter(const lnothrow, const ShaderParamType&, Type, type)
			DefGetter(lnothrow, ShaderParamType&, TypeRef, type)
			DefGetter(const lnothrow, leo::uint32, ArraySize, array_size)
			DefGetter(lnothrow, leo::uint32&, ArraySizeRef, array_size)

			DefGetter(const lnothrow, const ShaderParamType&, ElemType, std::get<ShaderParamType>(elem_info))
			DefGetter(lnothrow, ShaderParamType&, ElemTypeRef, std::get<ShaderParamType>(elem_info))

			DefGetter(const lnothrow, const std::string&, ElemUserType, std::get<std::string>(elem_info))
			DefGetter(lnothrow, std::string&, ElemUserTypeRef, std::get<std::string>(elem_info))

			DefGetter(const lnothrow, const std::variant<ShaderParamType LPP_Comma std::string>&, ElemInfo, elem_info)
			DefGetter(lnothrow, std::variant<ShaderParamType LPP_Comma std::string>&, ElemInfoRef, elem_info)
	private:
		ShaderParamType type;
		leo::uint32 array_size = 0;

		std::variant<ShaderParamType, std::string>  elem_info =platform::Render::SPT_ElemEmpty;

		
	};

	class ShaderFragmentAsset {
	public:
		DefGetter(const lnothrow, const std::string&, Fragment, fragment)
			DefGetter(lnothrow, std::string&, FragmentRef, fragment)

	private:
		std::string fragment;
	};

	class ShaderBlobAsset :leo::noncopyable {
	public:
		using Type = platform::Render::ShaderType;
		using ShaderBlob = platform::Render::ShaderBlob;

		explicit ShaderBlobAsset(Type _type, ShaderBlob&& _blob, platform::Render::ShaderInfo* pinfo)
			:type(_type), blob(lforward(_blob)), pInfo(pinfo)
		{}

		ShaderBlobAsset() = default;


		DefGetter(const lnothrow, const Type&, ShaderType, type)

			DefGetter(const lnothrow, const ShaderBlob&, Blob, blob)

			DefGetter(const lnothrow, const platform::Render::ShaderInfo&, Info, *pInfo)
	private:
		Type type;
		ShaderBlob blob;
		std::unique_ptr<platform::Render::ShaderInfo> pInfo;
	};



	class ShadersAsset
	{
	public:
		DefGetter(const lnothrow, const std::vector<ShaderMacro>&, Macros, macros)
			DefGetter(lnothrow, std::vector<ShaderMacro>&, MacrosRef, macros)

		DefGetter(const lnothrow, const std::vector<ShaderConstantBufferAsset>&, CBuffers, cbuffers)
			DefGetter(lnothrow, std::vector<ShaderConstantBufferAsset>&, CBuffersRef, cbuffers)

			DefGetter(const lnothrow, const std::vector<ShaderParameterAsset>&, Params, params)
			DefGetter(lnothrow, std::vector<ShaderParameterAsset>&, ParamsRef, params)

			DefGetter(const lnothrow, const std::vector<ShaderFragmentAsset>&, Fragments, fragements)
			DefGetter(lnothrow, std::vector<ShaderFragmentAsset>&, FragmentsRef, fragements)


		const ShaderBlobAsset& GetBlob(size_t blob_index) const {
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
			auto iter = std::find_if(bind_values.begin(), bind_values.end(), [&](const std::pair<size_t, leo::any>& pair) {
				return pair.first == param_index;
				});
			if (iter != bind_values.end())
				return iter->second;
			return std::nullopt;
		}

		template<typename T>
		std::optional<T> GetInfo(const std::string_view& name) const;

		static std::string GetTypeName(ShaderParamType type);
		static ShaderParamType GetType(const std::string&);
		
	private:
		std::vector<ShaderMacro> macros;

		std::vector<ShaderConstantBufferAsset> cbuffers;
		std::vector<ShaderParameterAsset> params;
		std::vector<std::pair<size_t, leo::any>> bind_values;
		std::vector<ShaderFragmentAsset> fragements;
		std::unordered_map<size_t, ShaderBlobAsset> blobs;
	public:
		std::string GenHLSLShader() const;


		enum GenSlot {
			MACRO,
			PARAM,
			CBUFFER,
			FRAGMENT
		};

		void EmplaceShaderGenInfo(GenSlot genslot, std::size_t localindex, std::size_t genindex) {
			gen_indices.emplace_back(genslot, static_cast<leo::uint32>(localindex), static_cast<leo::uint32>(genindex));
		}

		void PrepareShaderGen() {
			std::sort(gen_indices.begin(), gen_indices.end(), [](const std::tuple<GenSlot, leo::uint32, leo::uint32>& lhs, const std::tuple<GenSlot, leo::uint32, leo::uint32>& rhs) {
				if (std::get<2>(lhs) == std::get<2>(rhs))
					return std::get<1>(lhs) < std::get<1>(rhs);
				return std::get<2>(lhs) < std::get<2>(rhs);
				});
		}
	private:
		std::vector<std::tuple<GenSlot, leo::uint32, leo::uint32>> gen_indices;
	};

	bool RequireElemType(ShaderParamType type);

	bool RequireStructElemType(ShaderParamType type);
}
