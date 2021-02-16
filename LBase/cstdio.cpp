#include "LBase/cstdio.h"

namespace leo
{
	ifile_iterator&
		ifile_iterator::operator++()
	{
		lassume(stream);

		const auto val(std::fgetc(stream));

		if LB_UNLIKELY(val == EOF)
			stream = {};
		else
		{
			lassume(byte(val) == val);
			value = byte(val);
		}
		return *this;
	}

	const char*
		openmode_conv(std::ios_base::openmode mode) lnothrow
	{
		using namespace std;

		switch (unsigned((mode &= ~ios_base::ate) & ~ios_base::binary))
		{
		case ios_base::out:
		case ios_base::out | ios_base::trunc:
			return mode & ios_base::binary ? "wb" : "w";
		case ios_base::out | ios_base::app:
		case ios_base::app:
			return mode & ios_base::binary ? "ab" : "a";
		case ios_base::in:
			return mode & ios_base::binary ? "rb" : "r";
		case ios_base::in | ios_base::out:
			return mode & ios_base::binary ? "r+b" : "r+";
		case ios_base::in | ios_base::out | ios_base::trunc:
			return mode & ios_base::binary ? "w+b" : "w+";
		case ios_base::in | ios_base::out | ios_base::app:
		case ios_base::in | ios_base::app:
			return mode & ios_base::binary ? "a+b" : "a+";
		default:
			break;
		}
		return {};
	}
	std::ios_base::openmode
		openmode_conv(const char* str) lnothrow
	{
		using namespace std;

		if (str)
		{
			ios_base::openmode mode;

			switch (*str)
			{
			case 'w':
				mode = ios_base::out | ios_base::trunc;
				break;
			case 'r':
				mode = ios_base::in;
				break;
			case 'a':
				mode = ios_base::app;
				break;
			default:
				goto invalid;
			}
			if (str[1] != char())
			{
				auto l(char_traits<char>::length(str));

				if (str[l - 1] == 'x')
				{
					if (mode & ios_base::out)
						mode &= ~ios_base::out;
					else
						goto invalid;
					--l;
				}

				bool b(str[1] == 'b'), p(str[1] == '+');

				switch (l)
				{
				case 2:
					if (b != p)
						break;
					goto invalid;
				case 3:
					lunseq(b = b != (str[2] == 'b'), p = p != (str[2] == '+'));
					if (b && p)
						break;
				default:
					goto invalid;
				}
				if (p)
					mode |= *str == 'r' ? ios::out : ios::in;
				if (b)
					mode |= ios::binary;
			}
			return mode;
		}
	invalid:
		return ios_base::openmode();
	}
}