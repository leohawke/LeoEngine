#ifndef LScheme_Inc_SXML_h
#define LScheme_Inc_SXML_h 1

#include "sdef.h"

namespace scheme {
	struct LS_API LTag {};

	using stdex::byte;

	namespace sxml {
		enum class ParseOption
		{
			Normal,
			Strict,
			String,
			Attribute
		};
	}
}

#endif
