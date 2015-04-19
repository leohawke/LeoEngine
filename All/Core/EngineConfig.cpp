#include "platform.h"


#include "EngineConfig.h"
#include "FileSearch.h"

#include "..\d3dx11.hpp"
#include "..\exception.hpp"
#include "..\DeviceMgr.h"
#include "..\scheme_helper.h"
#include "string.hpp"
#include <fstream>

using namespace leo;
using namespace std::experimental::string_view_literals;
	static leo::scheme::sexp::sexp_list read_config_sexp = nullptr;
	static leo::scheme::sexp::sexp_list write_config_sexp = nullptr;

	static scheme::sexp::sexp_list pack_effect(const std::wstring& shader);

	void EngineConfig::Read(const std::wstring& configScheme) {
		read_config_sexp = leo::parse_file(configScheme);

		auto window_sexp = leo::scheme::sexp::ops::find_sexp("window"sv, read_config_sexp);

		auto width = static_cast<leo::uint16>(leo::expack_long(window_sexp,"width"sv));
		auto height = static_cast<leo::uint16>(leo::expack_long(window_sexp,"height"sv));

		global::globalClientSize.first = width;
		global::globalClientSize.second = height;

		auto dirs_sexp = leo::scheme::sexp::ops::find_sexp("search-dirs"sv, read_config_sexp);
		auto dirs_num = leo::scheme::sexp::ops::sexp_list_length(dirs_sexp)-1;

		auto dirs_iter = dirs_sexp->mNext;
		while (dirs_iter)
		{
			auto dir_sexp = leo::scheme::sexp::ops::find_sexp("dir"sv, dirs_iter);
			FileSearch::PushSearchDir(to_wstring(dir_sexp->mNext->mValue.cast_atom<scheme::sexp::sexp_string>()));
			dirs_iter = dirs_iter->mNext;
		}

		write_config_sexp = scheme::sexp::ops::make_copy(read_config_sexp);
	}
	void EngineConfig::Write(const std::wstring& configScheme) {
		using namespace scheme;
		auto config_sexp = sexp::make_sexp_word(S("config"));

		auto window_sexp = sexp::make_sexp_word(S("window"));
		auto width_sexp = pack_key_value(S("width"), ClientSize().first);
		auto height_sexp = pack_key_value(S("height"), ClientSize().second);
		window_sexp->mNext = sexp::make_sexp(sexp::sexp_list(width_sexp));
		window_sexp->mNext->mNext = sexp::make_sexp(sexp::sexp_list(height_sexp));



		config_sexp->mNext = sexp::make_sexp(sexp::sexp_list(window_sexp));

		auto dirs_sexp = sexp::make_sexp_word(S("search-dirs"));
		auto prev = dirs_sexp;
		for (auto & dir : SearchDirectors()) {
			auto dir_sexp = pack_key_value(S("dir"), to_string(dir));
			prev->mNext = sexp::make_sexp(sexp::sexp_list(dir_sexp));
			prev = prev->mNext;
		}
		config_sexp->mNext->mNext = sexp::make_sexp(std::shared_ptr<sexp::sexp>(dirs_sexp));

		auto effects_sexp = sexp::make_sexp_word(S("effects"));
		prev = effects_sexp;
		for (auto & shader : ShaderConfig::GetAllShaderName()) {
			auto effect_sexp = pack_effect(shader);
			prev->mNext = sexp::make_sexp(sexp::sexp_list(effect_sexp));
			prev = prev->mNext;
		}

		//end
		//config_sexp->mNext->mNext->mNext = effects_sexp;
		config_sexp->mNext->mNext->mNext = sexp::make_sexp(std::shared_ptr<sexp::sexp>(effects_sexp));

		//auto config_string = scheme::sexp::ops::print_sexp(config_sexp);
		auto config_string = scheme::sexp::ops::print_sexp(write_config_sexp);
		std::ofstream fout(configScheme);
		const unsigned char BOM[] = { 0xEFu, 0XBBu,0xBFu };
		fout.write((char*)BOM, 3);
		fout.write(config_string.c_str(), config_string.size());
	}

	const std::pair<uint16, uint16>& EngineConfig::ClientSize() {
		return global::globalClientSize;
	}
	const std::vector<std::wstring>& EngineConfig::SearchDirectors() {
		return FileSearch::SearchDirectors();

	}

	struct ShaderFileName {
		std::wstring mFileName[6];
	};

	static std::vector<ShaderFileName> mShaderFileNames;
	static std::vector<std::wstring> mShaderNames;
	static std::vector<std::wstring> mSamplNames;
	static std::vector<std::wstring> mDepthNames;
	static std::vector<std::wstring> mBlendNames;
	static std::vector<std::wstring> mRasteNames;

	static std::vector<D3D11_RASTERIZER_DESC> mRasteDescs;
	static std::vector<D3D11_SAMPLER_DESC> mSamplDescs;
	static std::vector<D3D11_DEPTH_STENCIL_DESC> mDepthDescs;
	static std::vector<D3D11_BLEND_DESC> mBlendDescs;

	static void expack_effect(scheme::sexp::sexp_list effect_sexp) {
		using namespace scheme;
		auto name_sexp = sexp::ops::find_sexp("name"sv, effect_sexp);
		mShaderNames.push_back(
				to_wstring(
				name_sexp->mNext->mValue.cast_atom<sexp::sexp_string>()
				)
			);
		auto index = mShaderFileNames.size();
		mShaderFileNames.push_back(ShaderFileName());
		auto shader_sexp = sexp::ops::find_sexp("shader"sv, effect_sexp);
		auto shader_iter = shader_sexp->mNext;
		while (shader_iter)
		{
			auto value_sexp = shader_iter->mValue.cast_list();
			auto type = value_sexp->mValue.cast_atom<sexp::sexp_string>()[0];
			auto file =to_wstring(value_sexp->mNext->mValue.cast_atom <sexp::sexp_string>());
			switch (type)
			{
			case 'v':
			case 'V':
				mShaderFileNames[index].mFileName[D3D11_VERTEX_SHADER - 1] = file;
				break;
			case 'p':
			case 'P':
				mShaderFileNames[index].mFileName[D3D11_PIXEL_SHADER - 1] = file;
				break;
			case 'g':
			case 'G':
				mShaderFileNames[index].mFileName[D3D11_GEOMETRY_SHADER - 1] = file;
				break;
			case 'c':
			case 'C':
				mShaderFileNames[index].mFileName[D3D11_COMPUTE_SHADER - 1] = file;
				break;
			case 'h':
			case 'H':
				mShaderFileNames[index].mFileName[D3D11_HULL_SHADER - 1] = file;
				break;
			case 'd':
			case 'D':
				mShaderFileNames[index].mFileName[D3D11_DOMAIN_SHADER - 1] = file;
				break;
			default:
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "读取\"" + to_string(mSamplNames[index]) + " \"Shader出现了无法识别的Shader类型");
			}
			shader_iter = shader_iter->mNext;
		}
	}

	void pack_shader(const std::wstring& shader,scheme::sexp::sexp_list& prev, D3D11_SHADER_TYPE s) {
		using namespace scheme;
		if (!EngineConfig::ShaderConfig::GetShaderFileName(shader, s).empty()) {
			std::string type;
			switch (s)
			{
			case D3D11_VERTEX_SHADER:
				type = "v";
				break;
			case D3D11_HULL_SHADER:
				type = "h";
				break;
			case D3D11_DOMAIN_SHADER:
				type = "d";
				break;
			case D3D11_GEOMETRY_SHADER:
				type = "g";
				break;
			case D3D11_PIXEL_SHADER:
				type = "p";
				break;
			case D3D11_COMPUTE_SHADER:
				type = "c";
				break;
			default:
				break;
			}
			prev->mNext = sexp::make_sexp(
				sexp::sexp_list(pack_key_value(
				type,
				to_string(EngineConfig::ShaderConfig::GetShaderFileName(shader, s)))
				));
			prev = prev->mNext;
		}
	}

	scheme::sexp::sexp_list pack_effect(const std::wstring& shader) {
		using namespace scheme;
		auto effect_sexp = sexp::make_sexp_word(S("effect"));

		auto name_sexp = pack_key_value("name", to_string(shader));
		effect_sexp->mNext = sexp::make_sexp(sexp::sexp_list(name_sexp));

		auto shader_sexp = sexp::make_sexp_word(S("shader"));
		auto prev = shader_sexp;
		pack_shader(shader, prev, D3D11_VERTEX_SHADER);
		pack_shader(shader, prev, D3D11_PIXEL_SHADER);
		pack_shader(shader, prev, D3D11_GEOMETRY_SHADER);
		pack_shader(shader, prev, D3D11_HULL_SHADER);
		pack_shader(shader, prev, D3D11_DOMAIN_SHADER);
		pack_shader(shader, prev, D3D11_COMPUTE_SHADER);
		//end
		effect_sexp->mNext->mNext = sexp::make_sexp(shader_sexp);

		return effect_sexp;
	}
	static void ShaderConfigInit() {
		using namespace scheme;
		static bool init = false;
		if (!init) {

			auto effects_sexp = sexp::ops::find_sexp("effects"sv, read_config_sexp);
			if (effects_sexp) {
				auto effects_num = sexp::ops::sexp_list_length(effects_sexp) - 1;

				auto effects_iter = effects_sexp->mNext;
				while (effects_iter)
				{
					auto effect_sexp = sexp::ops::find_sexp("effect"sv, effects_iter);
					expack_effect(effect_sexp);

					effects_iter = effects_iter->mNext;
				}
			}

			init = true;
		}
	}


	template<typename Index,typename Contain>
	const typename Contain::value_type& find_helper(const Contain& vals, const Index& indexs, const typename Index::value_type& index) {
		auto it = std::find(std::begin(indexs), std::end(indexs), index);
		if(it == std::end(indexs))
			Raise_Error_Exception(ERROR_INVALID_PARAMETER, "要查找的"+to_string(index)+"不存在");
		return vals[std::distance(std::begin(indexs), it)];
	}

	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllShaderName() {
		ShaderConfigInit();
		return mShaderNames;
	}
	const std::wstring& EngineConfig::ShaderConfig::GetShaderFileName(const std::wstring& shaderName, D3D11_SHADER_TYPE shaderType) {
		ShaderConfigInit();
		return find_helper(mShaderFileNames, mShaderNames, shaderName).mFileName[shaderType - 1];
	}

	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllSampleStateName() {
		ShaderConfigInit();
		return mSamplNames;
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllDepthStencilStateName() {
		ShaderConfigInit();
		return mDepthNames;
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllRasterizerStateName() {
		ShaderConfigInit();
		return mRasteNames;
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllBlendStateName() {
		ShaderConfigInit();
		return mBlendNames;
	}

	const D3D11_RASTERIZER_DESC& EngineConfig::ShaderConfig::GetRasterizerState(const std::wstring& rasName) {
		ShaderConfigInit();
		return find_helper(mRasteDescs, mRasteNames, rasName);
	}
	const D3D11_DEPTH_STENCIL_DESC& EngineConfig::ShaderConfig::GetDepthStencilState(const std::wstring& depName) {
		ShaderConfigInit();
		return find_helper(mDepthDescs, mDepthNames, depName);
	}
	const D3D11_BLEND_DESC& EngineConfig::ShaderConfig::GetBlendState(const std::wstring& bleName) {
		ShaderConfigInit();
		return find_helper(mBlendDescs, mBlendNames, bleName);
	}
	const D3D11_SAMPLER_DESC& EngineConfig::ShaderConfig::GetSampleState(const std::wstring& samName) {
		ShaderConfigInit();
		return find_helper(mSamplDescs, mSamplNames, samName);
	}

	scheme::sexp::sexp_list find_sexp(const std::experimental::basic_string_view<char>& word, const scheme::sexp::sexp_list & s)
	{
		using namespace scheme::sexp;
		auto iter = s;
		while (true)
		{
			if (!iter)
				return nullptr;
			if (iter->mValue.can_cast<sexp_list>()) {
				 auto next = iter->mValue.cast_list();
				//做一次向下查找
				if (next->mValue.can_cast<sexp_string>() && (car_to_string(next) == word))
					return next;
			}
			else
				if (iter->mValue.can_cast<sexp_string>() &&  car_to_string(iter) == word)
					return iter;
			iter = iter->mNext;
		}

		return nullptr;
	}

	scheme::sexp::sexp_list ParsePath(const std::string& path, std::string& subpath) {
		using namespace leo::scheme::sexp;

		auto iter_index = path.find_first_of('/');
		auto iter_end = path.find_last_of('/');
		sexp_list iter_sexp = write_config_sexp;
		sexp_list prev_sexp = iter_sexp;
		auto prev_index = iter_index;
		for (; iter_index != iter_end;) {
			prev_index = iter_index;
			auto iter_next = path.find_first_of('/', iter_index + 1);
			auto dir = to_string(path.substr(iter_index + 1, iter_next - iter_index - 1));
			iter_index = iter_next;
			prev_sexp = iter_sexp;
			iter_sexp = find_sexp(dir, iter_sexp);
			if (!iter_sexp)
				break;
		}

		if (!iter_sexp) {
			subpath = path.substr(prev_index, path.size()  - prev_index);
			return prev_sexp;
		}

		auto property_name = path.substr(iter_index + 1, path.size() - 1 - iter_index);

		prev_sexp = iter_sexp;
		iter_sexp = find_sexp(property_name, iter_sexp);

		if (!iter_sexp) {
			subpath = path.substr(iter_index, path.size()  - iter_index);
			return prev_sexp;
		}

		subpath = std::string();
		return iter_sexp;
	}

	template<typename T>
	scheme::sexp::sexp_list PathPack(const std::string& path, const T& value,bool data = false) {
		auto iter_index = path.find_last_of('/');

		auto str = path.substr(iter_index + 1, path.size() - 1 - iter_index);
		auto path_sexp =pack_key_value(
							path.substr(iter_index + 1, path.size() - 1 - iter_index), 
							value);
		if (data)
			path_sexp->mNext =path_sexp->mNext->mValue.mListValue;
		while (iter_index != 0) {
			auto iter_next = path.find_last_of('/', iter_index - 1);
			path_sexp = pack_key_value(path.substr(iter_next + 1, iter_index - 1 - iter_next), path_sexp);
			str = path.substr(iter_next + 1, iter_index - 1 - iter_next);
			iter_index = iter_next;
		}
		return path_sexp;
	}

	template<typename T>
	void write_sexp_type(const std::string& path,const T& value, bool data = false) {
		std::string subpath{};
		auto parent_sexp = ParsePath(path, subpath);
		auto path_sexp = PathPack(subpath, value,data);

		while (parent_sexp->mNext) {
			parent_sexp = parent_sexp->mNext;
		}

		parent_sexp->mNext = scheme::sexp::make_sexp(scheme::sexp::sexp_list(path_sexp));
	}


	void EngineConfig::Save(const std::string& path, bool value) {
		write_sexp_type(path, value);
	}
	void EngineConfig::Save(const std::string& path, char value) {
		write_sexp_type(path, value);
	}
	void EngineConfig::Save(const std::string& path, std::int64_t value) {
		write_sexp_type(path, value);
	}
	void EngineConfig::Save(const std::string& path, std::double_t value) {
		write_sexp_type(path, value);
	}
	void EngineConfig::Save(const std::string& path, const std::string& value) {
		write_sexp_type(path, value);
	}

	scheme::sexp::sexp_list ParsePath(const std::string& path) {
		using namespace leo::scheme::sexp;

		auto iter_index = path.find_first_of('/');
		auto iter_end = path.find_last_of('/');
		sexp_list iter_sexp = read_config_sexp;
		for (; iter_index != iter_end;) {
			auto iter_next = path.find_first_of('/', iter_index + 1);
			auto dir = to_string(path.substr(iter_index + 1, iter_next - iter_index - 1));
			iter_index = iter_next;
			iter_sexp = find_sexp(dir, iter_sexp);
		}

		auto property_name = path.substr(iter_index + 1, path.size() - 1 - iter_index);
		iter_sexp = find_sexp(property_name, iter_sexp);

		if (!iter_sexp)
			throw logged_event(property_name + ": this property doesn't exist(path=" + path + ")", record_level::Warning);
		return iter_sexp;
	}

	template<typename T>
	void CheckType(scheme::sexp::sexp_list property, const T&) {
#if defined(DEBUG)
		if (!property->mNext->mValue.can_cast<T>())
			throw logged_event(car_to_string(property) + ": this property isn't "+typeid(T).name(), record_level::Warning);
#endif
	}

	template<typename T>
	void read_sexp_type(const std::string& path, T& value) {
		try {
			auto property_sexp = ParsePath(path);
			CheckType(property_sexp, value);
			value = expack<T>(property_sexp);
		}
		catch (logged_event & e)
		{
			auto str = format_logged_event(e);
			DebugPrintf("%s", str.c_str());
		}
	}

	void EngineConfig::Read(const std::string& path, bool& value){
		read_sexp_type(path, value);
	}
	void EngineConfig::Read(const std::string& path, char& value) {
		read_sexp_type(path, value);
	}
	void EngineConfig::Read(const std::string& path, std::int64_t & value) {
		read_sexp_type(path, value);
	}
	void EngineConfig::Read(const std::string& path, std::double_t& value) {
		read_sexp_type(path, value);
	}
	void EngineConfig::Read(const std::string& path, std::string& value) {
		read_sexp_type(path, value);
	}

	void leo::EngineConfig::Save(const std::string & path, const std::vector<std::string>& value){
		assert( value.size() > 0 );
		if (value.size() == 1)
			return Save(path, value[0]);
		auto iter = value.rbegin();
		auto iter_end = value.rend();
		auto vector_sexp = cons(*(iter + 1), *iter);
		++iter;
		while (iter != iter_end) {
			if (++iter != iter_end)
				vector_sexp = cons(*iter, vector_sexp);
		}
		write_sexp_type(path, vector_sexp,true);
	}

	void leo::EngineConfig::Read(const std::string & path, std::vector<std::string>& value){
		try {
			auto property_sexp = ParsePath(path);
			while (property_sexp->mNext) {
				CheckType(property_sexp, std::string());
				value.push_back(expack<std::string>(property_sexp));
				property_sexp = property_sexp->mNext;
			}
		}
		catch (logged_event & e)
		{
			auto str = format_logged_event(e);
			DebugPrintf("%s", str.c_str());
		}
	}

	void leo::EngineConfig::Save(const std::string & path, const scheme::sexp::sexp_list & value){
		write_sexp_type(path, value);
	}

	void leo::EngineConfig::Read(const std::string & path, scheme::sexp::sexp_list & value,bool copy){
		auto val = value;
		read_sexp_type(path, val);
		if (copy)
			value = scheme::sexp::ops::make_copy(val);
		else
			value = val;
	}


	template<size_t multi>
	void read_multifloat_type(const std::string& path,data_storage<float,multi>& value) {
		try {
			auto property_sexp = ParsePath(path);
			auto value_iter = value.begin();
			auto end_iter = value.end();
			while (property_sexp->mNext && (value_iter != end_iter)) {
				CheckType(property_sexp,scheme::sexp::sexp_real());
				*value_iter =float(expack<scheme::sexp::sexp_real>(property_sexp));
				property_sexp = property_sexp->mNext;
			}
		}
		catch (logged_event & e)
		{
			auto str = format_logged_event(e);
			DebugPrintf("%s", str.c_str());
		}
	}

	void leo::EngineConfig::Save(const std::string & path, const float2 & value){
		auto float2_sexp = cons(value.x,value.y);
		write_sexp_type(path, float2_sexp,true);
	}

	void leo::EngineConfig::Read(const std::string & path, float2 & value){
		read_multifloat_type(path, value);
	}

	void leo::EngineConfig::Save(const std::string & path, const float3 & value){
		auto float3_sexp = list(value.x, value.y, value.z);
		write_sexp_type(path, float3_sexp,true);
	}

	void leo::EngineConfig::Read(const std::string & path, float3 & value){
		read_multifloat_type(path, value);
	}

	void leo::EngineConfig::Save(const std::string & path, const float4 & value){
		auto float4_sexp = list(value.x, value.y, value.z,value.w);
		write_sexp_type(path, float4_sexp,true);
	}

	void leo::EngineConfig::Read(const std::string & path, float4 & value){
		read_multifloat_type(path, value);
	}

	template<size_t multi>
	void read_multihalf_type(const std::string& path, data_storage<half, multi>& value) {
		try {
			auto property_sexp = ParsePath(path);
			auto value_iter = value.begin();
			auto end_iter = value.end();
			while (property_sexp->mNext && (value_iter != end_iter)) {
				CheckType(property_sexp, scheme::sexp::sexp_int());
				value_iter->data = decltype(half::data)(expack<scheme::sexp::sexp_int>(property_sexp));
				property_sexp = property_sexp->mNext;
			}
		}
		catch (logged_event & e)
		{
			auto str = format_logged_event(e);
			DebugPrintf("%s", str.c_str());
		}
	}

	void leo::EngineConfig::Save(const std::string & path, const half2 & value)
	{
		using scheme::sexp::sexp_int;
		auto half2_sexp = list(sexp_int(value.x.data), sexp_int(value.y.data));
		write_sexp_type(path, half2_sexp, true);

	}

	void leo::EngineConfig::Read(const std::string & path, half2 & value){
		read_multihalf_type(path, value);
	}

	void leo::EngineConfig::Save(const std::string & path, const half3 & value)
	{
		using scheme::sexp::sexp_int;
		auto half3_sexp = list(sexp_int(value.x.data), sexp_int(value.y.data), sexp_int(value.z.data));
		write_sexp_type(path, half3_sexp, true);
	}

	void leo::EngineConfig::Read(const std::string & path, half3 & value) {
		read_multihalf_type(path, value);
	}

	void leo::EngineConfig::Save(const std::string & path, const half4 & value)
	{
		using scheme::sexp::sexp_int;
		auto half4_sexp = list(sexp_int(value.x.data), sexp_int(value.y.data), sexp_int(value.z.data), sexp_int(value.w.data));
		write_sexp_type(path, half4_sexp, true);
	}

	void leo::EngineConfig::Read(const std::string & path, half4 & value) {
		read_multihalf_type(path, value);
	}

