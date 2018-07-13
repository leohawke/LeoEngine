/*! \file Core\LSLBuilder.h
\ingroup Engine
\brief LSL Builder Function¡£
*/
#ifndef LE_Core_LSLBuilder_H
#define LE_Core_LSLBuilder_H 1

#include "LSLEvaluator.h"

namespace platform::lsl {
	using namespace scheme;
	using namespace v1;

	namespace context {

		LiteralPasses::HandlerType FetchNumberLiteral();
	}

	namespace math {
		void RegisterTypeLiteralAction(REPLContext& context);
	}


	namespace access {
		namespace details {
			template<typename _target,typename _head, typename... tails>
			_target static_value_cast(TermNode & term) {
				if (pHead = leo::AccessPtr<_head>(term))
					return static_cast<_target>(*pHead);
				else
					return static_value_cast<_target, tails...>(term);
			}

			template<typename _target>
			_target static_value_cast(TermNode& term) {
				return leo::Access<_target>(term);
			}
		}


		template<typename _target,typename... _types>
		_target static_value_cast(TermNode & term) {
			return detatils::static_value_cast<_target, _types...>(term);
		}

		template<typename _target>
		_target static_value_cast(TermNode & term) {
			return leo::Access<_target>(term);
		}
	}
}

#endif