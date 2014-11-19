#include "platform.h"
#include "string.hpp"

namespace leo
{
#ifdef WIN32
	std::wstring towstring(const char* c_str, std::size_t len)
	{
		const int w_Len(::MultiByteToWideChar(CP_ACP, 0, c_str, len, {}, 0));
		std::wstring wstr(w_Len, wchar_t());
		wchar_t * w_str = &wstr[0];
		::MultiByteToWideChar(CP_ACP, 0, c_str, len, w_str, w_Len);
		return wstr;
	}
	std::wstring towstring(const std::string& string)
	{
		const int w_Len(::MultiByteToWideChar(CP_ACP, 0, string.c_str(), string.length(), {}, 0));
		std::wstring wstr(w_Len, wchar_t());
		wchar_t * w_str = &wstr[0];
		::MultiByteToWideChar(CP_ACP, 0, string.c_str(), string.length(), w_str, w_Len);
		return wstr;
	}

	std::string tostring(const wchar_t* c_str, std::size_t len){
		char defaultChar = ' ';
		BOOL defaultBool = FALSE;
		const int w_Len(::WideCharToMultiByte(CP_ACP, 0, c_str, len, {}, 0,&defaultChar,&defaultBool));
		std::string wstr(w_Len, wchar_t());
		char * w_str = &wstr[0];
		::WideCharToMultiByte(CP_ACP, 0, c_str, len, w_str, w_Len, &defaultChar, &defaultBool);
		return wstr;
	}
	std::string tostring(const std::wstring& string){
		char defaultChar = ' ';
		BOOL defaultBool = FALSE;
		const int w_Len(::WideCharToMultiByte(CP_ACP, 0,string.c_str(),string.length(), {}, 0, &defaultChar, &defaultBool));
		std::string wstr(w_Len, wchar_t());
		char * w_str = &wstr[0];
		::WideCharToMultiByte(CP_ACP, 0, string.c_str(), string.length(), w_str, w_Len, &defaultChar, &defaultBool);
		return wstr;
	}
#endif
}