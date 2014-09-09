#pragma once
#include <string>
#include <cstdint>
namespace leo
{
	namespace yin
	{
		class Util
		{
		public:
			static std::wstring readfile(const std::wstring& path);
			static void msg(const std::wstring& m);

			template<typename Collection>
			static std::wstring joinwithseq(const Collection& coll, const std::wstring& seq)
			{
				std::wstring s;
				bool i = true;
				for (auto& c : coll)
				{
					i? i = false :s.append(seq);
					s.append(c.tostring());
				}
				return s;
			}

			static std::wstring unifypath(const std::wstring& filename);

			static void abort(const std::wstring& msg);
			static void abort(const wchar_t* msg);
			/*
			template<typename... Args>
			static void abort(const std::wstring& msg,const Args&... args)
			{
				abort(msg);
				abort(args);
			}
			template<typename... Args>
			static void abort(const wchar_t* msg,const Args&... args)
			{
				abort(msg);
				abort(args);
			}
			*/
		};
	}
}