#include "LBase/cwctype.h"

namespace stdex {

	bool iswprint(wchar_t wc)
	{
		// TODO: See https://www.sourceware.org/cgi-bin/cvsweb.cgi/src/newlib/libc/ctype/iswprint.c?cvsroot=src .
		// NOTE: See http://git.musl-libc.org/cgit/musl/tree/src/ctype/iswprint.c .
		const auto c = unsigned(wc);

		if (c < 0xFFU)
			return ((c + 1) & 0x7F) >= 0x21;
		if (c < 0x2028U || c - 0x202AU < 0xD800 - 0x202A
			|| c - 0xE000U < 0xFFF9 - 0xE000)
			return true;
		return !(c - 0xFFFCU > 0x10FFFF - 0xFFFC || (c & 0xFFFE) == 0xFFFE);
	}

	bool
		iswspace(wchar_t wc)
	{
		// NOTE: See https://www.sourceware.org/cgi-bin/cvsweb.cgi/src/newlib/libc/ctype/iswprint.c?cvsroot=src .
		return (wc >= 0x0009 && wc <= 0x000D) || wc == 0x0020 || wc == 0x1680
			|| wc == 0x180e || (wc >= 0x2000 && wc <= 0x200A && wc != 2007)
			|| wc == 0x2028 || wc == 0x2029 || wc == 0x205F || wc == 0x3000;
	}

}
