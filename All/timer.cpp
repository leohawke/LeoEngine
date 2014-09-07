#include "timer.hpp"
#include "exception.hpp"
#include <Windows.h>

namespace leo
{
	namespace win
	{
		timer::timer()
		{
			HANDLE thread = GetCurrentThread();
			win32call(SetThreadAffinityMask(thread, 1) != 0);
			LARGE_INTEGER largeint;
			win32call(QueryPerformanceFrequency(&largeint));
			freq = largeint.QuadPart;

			win32call(QueryPerformanceCounter(&largeint));
			start = largeint.QuadPart;
			elapse = 0.;
		}
		void timer::update()
		{
			LARGE_INTEGER largeint;
			win32call(QueryPerformanceCounter(&largeint));
			std::int64_t curr = largeint.QuadPart - start;
			decltype(delta) curr_mill = curr*1000.f / freq;
			delta = curr_mill - elapse;
			elapse = curr_mill;
		}
	}
}