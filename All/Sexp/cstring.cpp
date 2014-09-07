#include "cstring.hpp"

namespace leo
{
	namespace sexp
	{
		cstring::cstring(size_t s)
			:base(std::make_unique<char []>(s)), cap(s), len(0)
		{
			base[len] = 0;
		}

		cstring& cstring::add(char * a)
		{
			if (a == nullptr)
				return *this;
			auto alen = wcslen(a);
			if (len + alen >= cap)
			{
				try
				{
					auto  newbase = std::make_unique<char[]>(cap + cstring_growsize + alen);
					std::copy_n(base.get(), len, newbase.get());
					base = std::move(newbase);
					cap += cstring_growsize + alen;
				}
				catch (...)
				{
					return *this;
				}
			}

			memcpy(&base[len], a, alen);
			len += alen;
			base[len] = 0;

			return *this;
		}

		cstring& cstring::add(char a)
		{
			if (len + 1 >= cap)
			{
				try
				{
					auto  newbase = std::make_unique<char[]>(cap + cstring_growsize + 1);
					std::copy_n(base.get(), len, newbase.get());
					base = std::move(newbase);
					cap += cstring_growsize + 1;
				}
				catch (...)
				{
					return *this;
				}
			}

			base[len] = a;
			++len;
			base[len] = 0;
			return *this;
		}

		cstring& cstring::trim()
		{
			if (cap == len + 1)
				return *this;
			try
			{
				auto newbase = std::make_unique < char[]>(len + 1);
				std::copy_n(base.get(), len, newbase.get());
				base = std::move(newbase);
			}
			catch (...)
			{
				return *this;
			}
			base[len] = 0;
			cap = len + 1;
			return *this;
		}
	}
}