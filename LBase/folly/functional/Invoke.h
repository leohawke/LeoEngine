#pragma once

#include <functional>
#include <type_traits>

#include <folly/CppAttributes.h>
#include <folly/Portability.h>
#include <folly/Preprocessor.h>
#include <folly/Traits.h>
#include <folly/Utility.h>

namespace folly
{
	using std::invoke;
	using std::invoke_result;
	using std::invoke_result_t;
	using std::is_invocable;
	using std::is_invocable_v;
	using std::is_invocable_r;
	using std::is_invocable_r_v;
	using std::is_nothrow_invocable;
	using std::is_nothrow_invocable_v;
	using std::is_nothrow_invocable_r;
	using std::is_nothrow_invocable_r_v;

#define FOLLY_CREATE_MEMBER_INVOKER(classname, membername)                 \
  struct classname {                                                       \
    template <typename O, typename... Args>                                \
    FOLLY_MAYBE_UNUSED FOLLY_ERASE_HACK_GCC constexpr auto operator()(     \
        O&& o, Args&&... args) const                                       \
        noexcept(noexcept(                                                 \
            static_cast<O&&>(o).membername(static_cast<Args&&>(args)...))) \
            -> decltype(static_cast<O&&>(o).membername(                    \
                static_cast<Args&&>(args)...)) {                           \
      return static_cast<O&&>(o).membername(static_cast<Args&&>(args)...); \
    }                                                                      \
  }
}