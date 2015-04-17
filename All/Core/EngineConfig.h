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

enum D3D11_SHADER_TYPE;
struct D3D11_RASTERIZER_DESC;
struct D3D11_DEPTH_STENCIL_DESC;
struct D3D11_BLEND_DESC;
struct D3D11_SAMPLER_DESC;
namespace leo{
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

	};
}


#endif


