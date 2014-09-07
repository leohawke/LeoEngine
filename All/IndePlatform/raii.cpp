#ifdef WIN32
#include <Windows.h>
#else
#include <stdlib.h>
#endif
#include "raii.hpp"

#ifdef WIN32
static void * const invalid_void_pointer = reinterpret_cast<void*>(-1);

namespace leo
{
	HandleCloser::value_type HandleCloser::operator()() const
	{
		return invalid_void_pointer;
	}

	void HandleCloser::operator()(HandleCloser::value_type value) const
	{
		CloseHandle(value);
	}
}

#else
static void * const invalid_void_pointer = nullptr;

namespace leo
{
	HandleCloser::value_type HandleCloser::operator()() const
	{
		return invalid_void_pointer;
	}

	void HandleCloser::operator()(HandleCloser::value_type value) const
	{
		free(value);
	}
}
#endif