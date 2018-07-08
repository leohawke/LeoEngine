/*! \file Core\LSLBuilder.h
\ingroup Engine
\brief LSL Builder Function¡£
*/
#ifndef LE_Core_LSLBuilder_H
#define LE_Core_LSLBuilder_H 1

#include "LSLEvaluator.h"

namespace platform::lsl::context {
	using namespace scheme;
	using namespace v1;

	LiteralPasses::HandlerType FetchNumberLiteral();
}

#endif