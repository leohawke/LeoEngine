#include "MappingEx.h"

namespace CHRLib
{

	using namespace CharSet;

	namespace
	{

		template<byte*& _vCPMapPtr, size_t _vMax>
		ucs2_t
			dbcs_lkp(byte seq0, byte seq1)
		{
			const auto idx(size_t(seq0 << 8U | seq1));

			return idx < 0xFF7E
				? reinterpret_cast<const ucs2_t*>(_vCPMapPtr + 0x0100)[idx] : 0;
		}

	} // unnamed namespacce;

#if !CHRLIB_NODYNAMIC_MAPPING

#if 0
	byte* cp17;
#endif
	byte* cp113;
#if 0
	byte* cp2026;
#endif

#endif

	ucs2_t(*cp113_lkp)(byte, byte) = dbcs_lkp<CHRLib::cp113, 0xFF7E>;

} // namespace CHRLib;