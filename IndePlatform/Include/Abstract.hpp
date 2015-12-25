#ifndef IndePlatform_Abstract_hpp
#define IndePlatform_Abstract_hpp

#include "ldef.h"
#include "LAssert.h" //AbstractBase depend lassume

namespace leo
{
	namespace details
	{
		class AbstractBase {
		protected:
			virtual ~AbstractBase() lnothrow = default;

		private:
			virtual void LEO_PureAbstract_() lnothrow = 0;
		};

		template<typename RealBase>
		class ConcreteBase : public RealBase {
		protected:
			template<typename ...BaseParams>
			explicit ConcreteBase(BaseParams &&...vBaseParams)
				lnoexcept(std::is_nothrow_constructible<RealBase, BaseParams &&...>::value)
				: RealBase(std::forward<BaseParams>(vBaseParams)...)
			{
			}

		private:
			void LEO_PureAbstract_() lnothrow override{
			}
		};

	}
}

#define ABSTRACT					private ::leo::details::AbstractBase
#define CONCRETE(base)				public ::leo::details::ConcreteBase<base>

#define CONCRETE_INIT(base, ...)	::leo::details::ConcreteBase<base>(__VA_ARGS__)


#endif