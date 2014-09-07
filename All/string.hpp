#pragma once
#include <string>
#include <vector>
#include <cstdint>
namespace leo
{
	inline void split(const std::wstring& str, std::vector<std::wstring>& parts, const std::wstring& delimiters = L"")
	{
		std::wstring::size_type lastpos = str.find_first_not_of(delimiters, 0);
		std::wstring::size_type pos = str.find_first_of(delimiters, lastpos);

		while (pos != std::wstring::npos || lastpos != std::wstring::npos)
		{
			parts.push_back(str.substr(lastpos, pos - lastpos));
			lastpos = str.find_first_not_of(delimiters, pos);
			pos = str.find_first_of(delimiters, lastpos);
		}
	}
	inline std::vector<std::wstring> split(const std::wstring& str, const std::wstring& delimiters = L"")
	{
		std::vector<std::wstring> parts;
		split(str, parts, delimiters);
		return parts;
	}

	std::wstring towstring(const char* c_str,std::size_t len);
	std::wstring towstring(const std::string& string);
}