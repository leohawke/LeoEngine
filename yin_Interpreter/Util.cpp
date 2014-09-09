#include "Win.hpp"
#include "Util.hpp"
#include "..\All\file.hpp"
#include "..\All\exception.hpp"
namespace leo
{
	namespace yin
	{
		void Util::abort(const std::wstring& msg)
		{
			OutputDebugStringW(msg.c_str());
			std::abort();
		}
		void Util::abort(const wchar_t* msg)
		{
			OutputDebugStringW(msg);
			std::abort();
		}

		std::wstring Util::readfile(const std::wstring& path)
		{
			std::wstring string;
			leo::win::File file(path.c_str(), true, false, false);
#pragma region UTF-8
			{
				if (file.GetSize() < 3)
					goto UTF16;
				char a[3];
				file.Read(a, 0, 3);
				if (a[0] != 0xEF || a[1] != 0xBB || a[2] != 0xBF)
					goto UTF16;

				auto filesize = file.GetSize() - 3;
				auto buffer = std::make_unique<char[]>(filesize);
				file.Read(buffer.get(), 3, static_cast<std::uint32_t>(filesize));
				auto stringsize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, buffer.get(), filesize, nullptr, 0);
				if (!stringsize)
					return string;
				string.resize(stringsize, wchar_t());
				MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, buffer.get(), filesize, &string[0], stringsize);
				return string;
			}
#pragma endregion
#pragma region UTF-16
		UTF16:
		{

				if (file.GetSize() < 2)
					goto ANSI;
				char a[2];
				file.Read(a, 0, 2);
				const char c = 0xff, d = 0xfe;
				if ((a[0] != c || a[1] != d) && (a[0] != d || a[1] != c))
					goto ANSI;
				auto filesize = file.GetSize() - 2;
				string.resize(filesize);
				file.Read(&string[0], 2, static_cast<std::uint32_t>(filesize));
#ifndef BIG_ENDIAN
				if (a[0] == d && a[1] == c)
#else
				if (a[1] == 0xFE && a[0] == 0xFF)
#endif
				{
					for (auto & c : string)
					{
						auto p = reinterpret_cast<char*>(&c);
						std::swap(p[0], p[1]);
					}
				}
				return string;
		}
#pragma endregion
#pragma region ANSI
		ANSI:
		{
			auto filesize = file.GetSize();
			auto buffer = std::make_unique<char[]>(filesize);
			file.Read(buffer.get(),0, static_cast<std::uint32_t>(filesize));
			auto stringsize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, buffer.get(), filesize, nullptr, 0);
			if (!stringsize)
				return string;
			string.resize(stringsize, wchar_t());
			MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, buffer.get(), filesize, &string[0], stringsize);
			return string;
		}
#pragma endregion
		}
		void Util::msg(const std::wstring& m)
		{
			OutputDebugStringW(m.c_str());
			OutputDebugStringW(L"\n");
		}
	
		std::wstring Util::unifypath(const std::wstring& filename)
		{
			static wchar_t buffer[4096 / sizeof(wchar_t)];
			int retval= GetFullPathNameW(filename.c_str(), 4096 / sizeof(wchar_t), buffer, nullptr);
			if (retval == 0){
				Raise_Win32_Exception("获取完整路径失败");
				return std::wstring();
			}
			else
			{
				buffer[retval] = wchar_t();
				return std::wstring(buffer);
			}
		}
	}
}