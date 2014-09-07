#pragma once
#include "debug.hpp"
namespace leo
{
	namespace win
	{
		template<typename COM>
		void ReleaseCOM(COM* &com)
		{
			if (com)
				com->Release();
			com = nullptr;
		}
	}
}