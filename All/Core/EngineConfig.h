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

#include "..\IndePlatform\leoint.hpp"

#include <utility>
#include <vector>
#include <string>

namespace leo{
	class EngineConfig{
		static void Read(const std::wstring& configScheme = L"config.scheme");
		static void Write(const std::wstring& configScheme = L"config.scheme");

		static const std::pair<uint16, uint16>& ClientSize();
		static const std::vector<std::wstring>& SearchDirectors();
	};
}


#endif


