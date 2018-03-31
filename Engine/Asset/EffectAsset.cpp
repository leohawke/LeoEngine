#include "EffectAsset.h"
#include <LBase/string.hpp>
#include <LBase/sutility.h>

void asset::EffectNodeAsset::SetName(const std::string & Name)
{
	name = Name;
	hash = leo::constfn_hash(name);
}

//TODO string_view
class type_define
{
public:
	static type_define& Instance()
	{
		static type_define instance;
		return instance;
	}

	uint32_t type_code(std::string const & name) const
	{
		auto lowername = name;
		std::transform(lowername.begin(), lowername.end(), lowername.begin(), std::tolower);
		auto const name_hash = leo::constfn_hash(lowername);
		for (uint32_t i = 0; i < hashs.size(); ++i)
		{
			if (hashs[i] == name_hash)
			{
				return i;
			}
		}
		throw leo::unsupported();
	}

	std::string const & type_name(uint32_t code) const
	{
		if (code < types.size())
		{
			return types[code];
		}
		throw leo::unsupported();
	}

	type_define()
	{
		types.emplace_back("Texture1D");
		types.emplace_back("Texture2D");
		types.emplace_back("Texture3D");
		types.emplace_back("TextureCUBE");
		types.emplace_back("Texture1DArray");
		types.emplace_back("Texture2DArray");
		types.emplace_back("Texture3DArray");
		types.emplace_back("TextureCUBEArray");
		types.emplace_back("Buffer");
		types.emplace_back("StructuredBuffer");
		types.emplace_back("RWBuffer");
		types.emplace_back("RWStructuredBuffer");
		types.emplace_back("RWTexture1D");
		types.emplace_back("RWTexture2D");
		types.emplace_back("RWTexture3D");
		types.emplace_back("RWTexture1DArray");
		types.emplace_back("RWTexture2DArray");
		types.emplace_back("AppendStructuredBuffer");
		types.emplace_back("ConsumeStructuredBuffer");
		types.emplace_back("ByteAddressBuffer");
		types.emplace_back("RWByteAddressBuffer");
		types.emplace_back("sampler");
		types.emplace_back("shader");
		types.emplace_back("bool");
		types.emplace_back("string");
		types.emplace_back("uint");
		types.emplace_back("uint2");
		types.emplace_back("uint3");
		types.emplace_back("uint4");
		types.emplace_back("int");
		types.emplace_back("int2");
		types.emplace_back("int3");
		types.emplace_back("int4");
		types.emplace_back("float");
		types.emplace_back("float2");
		types.emplace_back("float2x2");
		types.emplace_back("float2x3");
		types.emplace_back("float2x4");
		types.emplace_back("float3");
		types.emplace_back("float3x2");
		types.emplace_back("float3x3");
		types.emplace_back("float3x4");
		types.emplace_back("float4");
		types.emplace_back("float4x2");
		types.emplace_back("float4x3");
		types.emplace_back("float4x4");

		for (auto type : types) {
			std::transform(type.begin(), type.end(), type.begin(), std::tolower);
			hashs.emplace_back(leo::constfn_hash(type));
		}
	}

private:
	std::vector<std::string> types;
	std::vector<size_t> hashs;
};


template<>
std::optional<platform::Render::ShaderInfo::ConstantBufferInfo> asset::EffectAsset::GetInfo<platform::Render::ShaderInfo::ConstantBufferInfo>(const std::string_view& name) const {
	using platform::Render::ShaderCompose;
	using platform::Render::ShaderInfo;
	auto hash = leo::constfn_hash(name);
	for (auto & pair : blobs) {
		auto& blob = pair.second;
		auto& Info = blob.GetInfo();

		for (auto& info : Info.ConstantBufferInfos) {
			if (info.name_hash == hash)
				return info;
		}
	}
	return {};
}

template<>
std::optional<platform::Render::ShaderInfo::ConstantBufferInfo::VariableInfo> asset::EffectAsset::GetInfo<platform::Render::ShaderInfo::ConstantBufferInfo::VariableInfo>(const std::string_view& name) const {
	using platform::Render::ShaderCompose;
	using platform::Render::ShaderInfo;
	auto hash = leo::constfn_hash(name);
	for (auto & pair : blobs) {
		auto& blob = pair.second;
		auto& Info = blob.GetInfo();

		for (auto& info : Info.ConstantBufferInfos) {
			for (auto& varinfo : info.var_desc) {
				if (varinfo.name == name)
					return varinfo;
			}
		}
	}
	return {};
}

template std::optional<platform::Render::ShaderInfo::ConstantBufferInfo> asset::EffectAsset::GetInfo(const std::string_view& name) const;
template std::optional<platform::Render::ShaderInfo::ConstantBufferInfo::VariableInfo> asset::EffectAsset::GetInfo(const std::string_view& name) const;

std::string asset::EffectAsset::GetTypeName(EffectParamType type) {
	return type_define::Instance().type_name(type);
}

asset::EffectParamType asset::EffectAsset::GetType(const std::string & name)
{
	return (EffectParamType)type_define::Instance().type_code(name);
}


std::string asset::EffectAsset::GenHLSLShader() const
{
	using std::endl;

	std::stringstream ss;
	for (auto & name_value : GetMacros())
	{
		ss << leo::sfmt("#define %s %s", name_value.first.c_str(), name_value.second.c_str()) << endl;
	}
	ss << endl;
	std::set<std::size_t> except_set;
	for (auto & cbuffer : GetCBuffers())
	{
		ss << leo::sfmt("cbuffer %s", cbuffer.GetName().c_str()) << endl;
		ss << '{' << endl;

		for (auto paramindex : cbuffer.GetParamIndices())
		{
			const auto & param = GetParams()[paramindex];
			if (param.GetType() >= EPT_bool) {
				ss << leo::sfmt("%s %s", GetTypeName(param.GetType()).c_str(), param.GetName().c_str());
				if (param.GetArraySize())
					ss << leo::sfmt("[%s]", std::to_string(param.GetArraySize()).c_str());
				ss << ';' << endl;
			}
			except_set.insert(param.GetNameHash());
		}

		ss << "};" << endl;
	}

	for (auto & param : GetParams())
	{
		if (except_set.count(param.GetNameHash()) > 0)
			continue;
		std::string elem_type;
		if (param.GetType() <= EPT_ConsumeStructuredBuffer && param.GetElemType() != EPT_ElemEmpty) {
			elem_type = GetTypeName(param.GetElemType());
			ss << leo::sfmt("%s<%s> %s", GetTypeName(param.GetType()).c_str(),
				elem_type.c_str(), param.GetName().c_str())
				<< ';'
				<< endl;
		}
		else {
			ss << leo::sfmt("%s %s", GetTypeName(param.GetType()).c_str(),
				param.GetName().c_str())
				<< ';'
				<< endl;
		}
	}
	ss << endl;

	for (auto & fragment : GetFragments())
		ss << fragment.GetFragment() << endl;

	return ss.str();
}
