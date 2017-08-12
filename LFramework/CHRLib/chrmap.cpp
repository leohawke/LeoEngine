#include "CharacterMapping.h"

namespace CHRLib
{

	using namespace CharSet;

	size_t
		FetchFixedCharWidth(Encoding cp)
	{
		switch (cp)
		{
		case csASCII:
			return 1;
		case csUnicode:
		case csUTF16BE:
		case csUTF16LE:
		case csUTF16:
			return 2;
		case csUCS4:
		case csUTF32:
		case csUTF32BE:
		case csUTF32LE:
			return 4;
		default:
			return 0;
		}
	}

	size_t
		FetchMaxCharWidth(Encoding cp)
	{
		const auto r = FetchFixedCharWidth(cp);

		return r == 0 ? FetchMaxVariantCharWidth(cp) : r;
	}

	size_t
		FetchMaxVariantCharWidth(Encoding cp)
	{
		switch (cp)
		{
		case csGBK:
			return 2;
		case csGB18030:
		case csUTF8:
			return 4;
		default:
			return 0;
		}
	}

} // namespace CHRLib;