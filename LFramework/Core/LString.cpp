#include "LString.h"

namespace leo
{
	namespace Text
	{
		String&
			String::operator*=(size_t n)
		{
			switch (n)
			{
			case 0:
				clear();
			case 1:
				break;
			default:
				reserve(length() * n);
				leo::concat(*this, n);
			}
			return *this;
		}
	} // namespace Text;
} // namespace leo;