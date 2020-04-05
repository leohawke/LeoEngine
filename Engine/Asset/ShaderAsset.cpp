#include "ShaderAsset.h"

using namespace asset;
using namespace platform::Render::ShaderCore;

void asset::AssetName::SetName(const std::string& Name)
{
	name = Name;
	hash = leo::constfn_hash(name);
}

constexpr int MaxSpace = 4;

class ResourceRegisterSlotAllocator
{
public:
	enum ResourceType
	{
		t,// for shader resource views (SRV)
		s,// for samplers
		u,//for unordered access views (UAV)
		b //for constant buffer views (CBV)
	};

	struct SpaceRegisters
	{
		std::unordered_map<ResourceType, std::vector<bool>> registers;


		void SetUse(std::vector<bool>& c, int index)
		{
			c.resize(index + 1);
			c[index] = true;
		}

		void SetUse(ResourceType type, int index)
		{
			SetUse(registers[type], index);
		}

		int FindEmpty(std::vector<bool>& c)
		{
			for (int i = 0; i != c.size(); ++i)
			{
				if (!c[i])
				{
					c[i] = true;
					return i;
				}
			}

			auto index = c.size();

			c.resize(c.size() + 1);
			c[index] = true;
			return static_cast<int>(index);
		}

		int FindEmpty(ResourceType type)
		{
			return FindEmpty(registers[type]);
		}
	};

	SpaceRegisters Spaces[MaxSpace];

	std::pair<ResourceType, int> AllocatorIndex(const BindDesc& desc, ShaderParamType type)
	{
		auto space = desc.GetSpace() == BindDesc::Any ? 0 : desc.GetSpace();

		lconstraint(space < MaxSpace);

		auto register_type = to_type(type);

		if (desc.GetIndex() == BindDesc::Any)
		{
			return { register_type,Spaces[space].FindEmpty(register_type) };
		}
		else
		{
			Spaces[space].SetUse(register_type, desc.GetIndex());

			return { register_type,desc.GetIndex() };
		}
	}

	void PreAllocator(const BindDesc& desc, ShaderParamType type)
	{
		auto space = desc.GetSpace() == BindDesc::Any ? 0 : desc.GetSpace();

		lconstraint(space < MaxSpace);

		auto register_type = to_type(type);

		if (desc.GetIndex() != BindDesc::Any)
		{
			Spaces[space].SetUse(register_type, desc.GetIndex());
		}
	}

	static char to_char(ResourceType type)
	{
		switch (type)
		{
		case ResourceRegisterSlotAllocator::t:
			return 't';
			break;
		case ResourceRegisterSlotAllocator::s:
			return 's';
			break;
		case ResourceRegisterSlotAllocator::u:
			return 'u';
			break;
		case ResourceRegisterSlotAllocator::b:
			return 'b';
			break;
		default:
			lconstraint(false);
		}
	}

	static ResourceType to_type(ShaderParamType type)
	{
		if (type == SPT_ConstatnBuffer)
			return b;
		if (type == SPT_sampler)
			return s;
		switch (type)
		{
		case SPT_rwbuffer:
		case SPT_rwstructured_buffer:
		case SPT_rwtexture1D:
		case SPT_rwtexture2D:
		case SPT_rwtexture3D:
		case SPT_rwtexture1DArray:
		case SPT_rwtexture2DArray:
		case SPT_AppendStructuredBuffer:
		case SPT_rwbyteAddressBuffer:
			return u;
		default:
			return t;
		}
	}
};

std::string FormatBindDesc(ResourceRegisterSlotAllocator& allocaotr, const BindDesc& desc, ShaderParamType type)
{
	if (desc.GetIndex() == BindDesc::Any && desc.GetSpace() == BindDesc::Any)
		return {};

	auto type_index = allocaotr.AllocatorIndex(desc, type);

	auto empty_space = desc.GetSpace() == BindDesc::Any;

	std::string hlsl = ":register(";

	hlsl += ResourceRegisterSlotAllocator::to_char(type_index.first);

	hlsl += std::to_string(type_index.second);

	if (!empty_space)
	{
		hlsl += ",space";
		hlsl += std::to_string(desc.GetSpace());
	}

	hlsl += ")";

	return hlsl;
}

bool asset::RequireStructElemType(ShaderParamType type)
{
	switch (type)
	{
	case SPT_AppendStructuredBuffer:
	case SPT_ConsumeStructuredBuffer:
	case SPT_StructuredBuffer:
	case SPT_rwstructured_buffer:
	case SPT_ConstatnBuffer:
		return true;
	}

	return false;
}


bool asset::RequireElemType(ShaderParamType type)
{
	if (type >= SPT_rwtexture1D && type <= SPT_rwtexture2DArray)
		return true;

	if (RequireStructElemType(type))
		return true;

	if (type == SPT_buffer || type == SPT_rwbuffer)
		return true;

	return false;
}

std::string asset::ShadersAsset::GenHLSLShader() const
{
	using std::endl;

	auto& macros = GetMacros();
	auto& cbuffers = GetCBuffers();
	auto& params = GetParams();
	auto& fragments = GetFragments();

	ResourceRegisterSlotAllocator allocator;
	//pre allocator
	for (auto& gen_index : gen_indices) {
		auto local_index = std::get<1>(gen_index);
		switch (std::get<0>(gen_index)) {
			case	CBUFFER:
			{
				auto& cbuffer = cbuffers[local_index];
				allocator.PreAllocator(cbuffer, SPT_ConstatnBuffer);
			}
			case	PARAM:
			{
				auto& param = params[local_index];
				allocator.PreAllocator(param, param.GetType());
			}
		}
	}

	std::stringstream ss;

	for (auto& gen_index : gen_indices) {
		auto local_index = std::get<1>(gen_index);
		switch (std::get<0>(gen_index)) {
		case	MACRO:
		{
			auto& name_value = macros[local_index];
			ss << leo::sfmt("#define %s %s", name_value.first.c_str(), name_value.second.c_str()) << endl;
			break;
		}
		case	CBUFFER:
		{
			auto& cbuffer = cbuffers[local_index];

			bool template_synatx = !cbuffer.GetElemInfo().empty();

			lconstraint(!template_synatx || !cbuffer.GetElemInfo().empty());

			if (!template_synatx)
				ss << leo::sfmt("cbuffer %s", cbuffer.GetName().c_str()) << endl;
			else
				ss << leo::sfmt("ConstantBuffer<%s> %s", cbuffer.GetElemInfo().c_str(), cbuffer.GetName().c_str()) << endl;

			ss << FormatBindDesc(allocator, cbuffer, SPT_ConstatnBuffer);

			if (!template_synatx)
			{
				ss << '{' << endl;

				for (auto paramindex : cbuffer.GetParamIndices())
				{
					const auto& param = GetParams()[paramindex];
					if (param.GetType() >= SPT_bool) {
						ss << leo::sfmt("%s %s", GetTypeName(param.GetType()).c_str(), param.GetName().c_str());
						if (param.GetArraySize())
							ss << leo::sfmt("[%s]", std::to_string(param.GetArraySize()).c_str());
						ss << ';' << endl;
					}
				}

				ss << "};" << endl;
			}
			else
			{
				ss << ";" << endl;
			}
			break;
		}
		case	PARAM:
		{
			auto& param = params[local_index];
			std::string elem_type = [&] {
				if (auto pElemType = std::get_if<ShaderParamType>(&param.GetElemInfo())) {
					if (*pElemType != SPT_ElemEmpty)
						return GetTypeName(param.GetElemType());
					else
						return std::string();
				}
				else {
					return param.GetElemUserType();
				}
			}();

			lconstraint(!RequireElemType(param.GetType())|| !elem_type.empty());

			if (param.GetType() <= SPT_ConsumeStructuredBuffer && !elem_type.empty()) {
				ss << leo::sfmt("%s<%s> %s", GetTypeName(param.GetType()).c_str(),
					elem_type.c_str(), param.GetName().c_str())
					<< FormatBindDesc(allocator, param, param.GetType())
					<< ';'
					<< endl;
			}
			else {
				ss << leo::sfmt("%s %s", GetTypeName(param.GetType()).c_str(),
					param.GetName().c_str())
					<< FormatBindDesc(allocator, param, param.GetType())
					<< ';'
					<< endl;
			}
			break;
		}
		case	FRAGMENT:
		{
			auto& fragment = fragments[local_index];
			ss << fragment.GetFragment() << endl;
			break;
		}
		}
	}

	return ss.str();
}


template<>
std::optional<platform::Render::ShaderInfo::ConstantBufferInfo> asset::ShadersAsset::GetInfo<platform::Render::ShaderInfo::ConstantBufferInfo>(const std::string_view& name) const {
	using platform::Render::ShaderCompose;
	using platform::Render::ShaderInfo;
	auto hash = leo::constfn_hash(name);
	for (auto& pair : blobs) {
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
std::optional<platform::Render::ShaderInfo::ConstantBufferInfo::VariableInfo> asset::ShadersAsset::GetInfo<platform::Render::ShaderInfo::ConstantBufferInfo::VariableInfo>(const std::string_view& name) const {
	using platform::Render::ShaderCompose;
	using platform::Render::ShaderInfo;
	auto hash = leo::constfn_hash(name);
	for (auto& pair : blobs) {
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

template std::optional<platform::Render::ShaderInfo::ConstantBufferInfo> asset::ShadersAsset::GetInfo(const std::string_view& name) const;
template std::optional<platform::Render::ShaderInfo::ConstantBufferInfo::VariableInfo> asset::ShadersAsset::GetInfo(const std::string_view& name) const;

class type_define
{
public:
	static type_define& Instance()
	{
		static type_define instance;
		return instance;
	}

	uint32_t type_code(std::string const& name) const
	{
		auto lowername = name;
		leo::to_lower(lowername);
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

	std::string const& type_name(uint32_t code) const
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
		types.emplace_back("ConstantBuffer");
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
		types.emplace_back("RaytracingAccelerationStructure");
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
			leo::to_lower(type);
			hashs.emplace_back(leo::constfn_hash(type));
		}
	}

private:
	std::vector<std::string> types;
	std::vector<size_t> hashs;
};




std::string asset::ShadersAsset::GetTypeName(ShaderParamType type) {
	return type_define::Instance().type_name(type);
}

ShaderParamType asset::ShadersAsset::GetType(const std::string& name)
{
	return (ShaderParamType)type_define::Instance().type_code(name);
}

#include "../Core/AssetResourceScheduler.h"
#include "LFramework/Helper/ShellHelper.h"
#include "ShaderLoadingDesc.h"
using namespace platform::X;

using namespace platform;
using namespace asset;
using namespace platform::Render::ShaderCore;
using namespace leo;

struct HLSLAsset :public asset::ShadersAsset, public asset::AssetName
{
	std::string Code;
};

class HLSLLoadingDesc : public asset::AssetLoading<HLSLAsset>, public platform::X::ShaderLoadingDesc<HLSLAsset> {
private:
	using Super = platform::X::ShaderLoadingDesc<HLSLAsset>;
public:
	explicit HLSLLoadingDesc(platform::X::path const& shaderpath)
		:Super(shaderpath)
	{
	}

	std::size_t Type() const override {
		return leo::type_id<HLSLLoadingDesc>().hash_code();
	}

	std::size_t Hash() const override {
		return leo::hash_combine_seq(Type(), Super::Hash());
	}

	const asset::path& Path() const override {
		return Super::Path();
	}

	std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() override {
		co_yield PreCreate();
		co_yield LoadNode();
		co_yield ParseNode();
		co_yield CreateAsset();
	}
private:
	std::shared_ptr<AssetType> PreCreate()
	{
		Super::PreCreate();
		return nullptr;
	}

	std::shared_ptr<AssetType> LoadNode()
	{
		Super::LoadNode();
		return  nullptr;
	}

	std::shared_ptr<AssetType> ParseNode()
	{
		Super::ParseNode();
		return nullptr;
	}

	std::shared_ptr<AssetType> CreateAsset()
	{
		GetAsset()->Code = GetCode();
		return ReturnValue();
	}
};


std::string platform::X::GenHlslShader(const path& filepath)
{
	auto pAsset = AssetResourceScheduler::Instance().SyncLoad<HLSLLoadingDesc>(filepath);

	return pAsset->Code;;
}