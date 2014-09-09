#pragma once

#include <string>
namespace leo
{
	namespace yin
	{
		class Value;
		class Interpreter
		{
			std::wstring file;
		public:
			Interpreter(const std::wstring& file)
				:file(file)
			{}
		public:
			Value interp(const std::wstring& file);
			Value interp();
		};
	}
}