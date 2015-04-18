//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/EngineConfig.h
//  Version:     v1.00
//  Created:     11/18/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供引擎配置文件逻辑,只提供获取逻辑,不提供保存逻辑
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_EngineConfig_H
#define Core_EngineConfig_H

#include "leoint.hpp"

#include <utility>
#include <vector>
#include <string>
#include <memory>

enum D3D11_SHADER_TYPE;
struct D3D11_RASTERIZER_DESC;
struct D3D11_DEPTH_STENCIL_DESC;
struct D3D11_BLEND_DESC;
struct D3D11_SAMPLER_DESC;
namespace leo{

	namespace scheme {
		namespace sexp {
			struct sexp;
			using sexp_list = std::shared_ptr < sexp >;
		}
	}

	struct float2;
	struct float3;
	struct float4;

	struct half2;
	struct half3;
	struct half4;

	class EngineConfig{
	public:
		static void Read(const std::wstring& configScheme = L"config.scheme");
		static void Write(const std::wstring& configScheme = L"config.scheme");

		static const std::pair<uint16, uint16>& ClientSize();
		static const std::vector<std::wstring>& SearchDirectors();

		class ShaderConfig {
		public:
			static const std::vector<std::wstring>& GetAllShaderName();
			static const std::wstring& GetShaderFileName(const std::wstring&, D3D11_SHADER_TYPE);

			static const std::vector<std::wstring>& GetAllSampleStateName();
			static const std::vector<std::wstring>& GetAllDepthStencilStateName();
			static const std::vector<std::wstring>& GetAllRasterizerStateName();
			static const std::vector<std::wstring>& GetAllBlendStateName();

			static const D3D11_RASTERIZER_DESC& GetRasterizerState(const std::wstring&);
			static const D3D11_DEPTH_STENCIL_DESC& GetDepthStencilState(const std::wstring&);
			static const D3D11_BLEND_DESC& GetBlendState(const std::wstring&);
			static const D3D11_SAMPLER_DESC& GetSampleState(const std::wstring&);
		};


		static void Save(const std::string& path, bool value);
		static void Save(const std::string& path, char value);
		static void Save(const std::string& path, std::int64_t value);
		static void Save(const std::string& path, std::double_t value);
		static void Save(const std::string& path, const std::string& value);

		static void Read(const std::string& path, bool& value);
		static void Read(const std::string& path, char& value);
		static void Read(const std::string& path, std::int64_t & value);
		static void Read(const std::string& path, std::double_t& value);
		static void Read(const std::string& path, std::string& value);

		static void Save(const std::string& path,const std::vector<std::string>& value);
		static void Read(const std::string& path, std::vector<std::string>& value);

		static void Save(const std::string& path, const scheme::sexp::sexp_list& value);
		static void Read(const std::string& path, scheme::sexp::sexp_list& value,bool copy = true);

		static void Save(const std::string& path, const float2& value);
		static void Read(const std::string& path, float2& value);

		static void Save(const std::string& path, const float3& value);
		static void Read(const std::string& path, float3& value);

		static void Save(const std::string& path, const float4& value);
		static void Read(const std::string& path, float4& value);

		static void Save(const std::string& path, const half2& value);
		static void Read(const std::string& path, half2& value);

		static void Save(const std::string& path, const half3& value);
		static void Read(const std::string& path, half3& value);

		static void Save(const std::string& path, const half4& value);
		static void Read(const std::string& path, half4& value);
	};
}


#endif


