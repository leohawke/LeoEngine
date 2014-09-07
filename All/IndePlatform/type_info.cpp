#include "EndianSwap.hpp"

namespace leo
{
	void SwapEndian(const type_info& info, std::size_t check, void * data, std::size_t nCount, bool bWriting)
	{
		switch (check)
		{
		case 1:
			break;
		case 2:
			SwapEndianBase((uint16*)data, nCount);
			break;
		case 4:
			SwapEndianBase((uint32*)data, nCount);
			break;
		case 8:
			SwapEndianBase((uint64*)data, nCount);
			break;
		default:
			assert(0);
		}
	}
}