#include "EffectAsset.h"
#include <LBase/string.hpp>

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
		size_t const name_hash = std::hash<std::string>()(name);
		for (uint32_t i = 0; i < hashs.size(); ++i)
		{
			if (hashs[i] == name_hash)
			{
				return i;
			}
		}
		LAssert(false,"Non't Support Name");
		return 0xFFFFFFFF;
	}

	std::string const & type_name(uint32_t code) const
	{
		if (code < types.size())
		{
			return types[code];
		}
		LAssert(false, "Non't Support Code");
		return "";
	}

	type_define()
	{
		types.emplace_back("texture1D");
		types.emplace_back("texture2D");
		types.emplace_back("texture3D");
		types.emplace_back("textureCUBE");
		types.emplace_back("texture1DArray");
		types.emplace_back("texture2DArray");
		types.emplace_back("texture3DArray");
		types.emplace_back("textureCUBEArray");
		types.emplace_back("buffer");
		types.emplace_back("structured_buffer");
		types.emplace_back("rw_buffer");
		types.emplace_back("rw_structured_buffer");
		types.emplace_back("rw_texture1D");
		types.emplace_back("rw_texture2D");
		types.emplace_back("rw_texture3D");
		types.emplace_back("rw_texture1DArray");
		types.emplace_back("rw_texture2DArray");
		types.emplace_back("append_structured_buffer");
		types.emplace_back("consume_structured_buffer");
		types.emplace_back("byte_address_buffer");
		types.emplace_back("rw_byte_address_buffer");
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
		
		for (auto type : types)
			hashs.emplace_back(std::hash<decltype(type)>()(type));
	}

private:
	std::vector<std::string> types;
	std::vector<size_t> hashs;
};

std::string asset::EffectAsset::GetTypeName(EffectParamType type) const {
	return type_define::Instance().type_name(type);
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
		}

		ss << "};" << endl;
	}

	for (auto & param : GetParams())
	{
		std::string elem_type;
		if (param.GetType() <= EPT_consume_structured_buffer) {
			elem_type = GetTypeName(param.GetElemType());
			ss << leo::sfmt("%s<%s> %s", GetTypeName(param.GetType()).c_str(),
				elem_type.c_str(), param.GetName().c_str())
				<< ';'
				<<endl;
		}
		else if (param.GetType() <= EPT_rw_byte_address_buffer) {
			ss << leo::sfmt("%s %s", GetTypeName(param.GetType()).c_str(),
				param.GetName().c_str())
				<< ';'
				<< endl;
		}
	}

	for (auto & fragment : GetFragments())
		ss << fragment.GetFragment() << endl;

	return ss.str();
}
