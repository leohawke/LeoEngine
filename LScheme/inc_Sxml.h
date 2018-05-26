#ifndef LScheme_Inc_SXML_h
#define LScheme_Inc_SXML_h 1

#include "sdef.h"
#include <LFramework/Adaptor/LAdaptor.h>

namespace scheme {
	struct LS_API LTag {};

	using leo::byte;

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
