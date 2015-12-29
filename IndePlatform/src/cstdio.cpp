#include "cstdio.h"

namespace stdex
{
	ifile_iterator&
		ifile_iterator::operator++()
	{
		lassume(stream);

		const auto val(std::fgetc(stream));

		if (LB_UNLIKELY(val == EOF))
			stream = {};
		else
		{
			lassume(byte(val) == val);
			value = byte(val);
		}
		return *this;
	}
}