/*! \file future.hpp
\ingroup LBase
\brief ±ê×¼¿â\c \<future\> À©Õ¹¡£

*/
#ifndef LBase_future_hpp
#define LBase_future_hpp 1

#include "LBase/type_traits.hpp" // for false_type,true_type,decay_t,result_t
#include <future> //for std::future, std::packaged_task_t;
#include <memory> //for std::shared_ptr;

namespace leo
{
	//@{
	template<typename>
	struct is_future_type : false_type
	{};

	template<typename _type>
	struct is_future_type<std::future<_type>> : true_type
	{};


#if !__GLIBCXX__ || (defined(_GLIBCXX_HAS_GTHREADS) \
	&& defined(_GLIBCXX_USE_C99_STDINT_TR1) && (ATOMIC_INT_LOCK_FREE > 1))
	template<typename _type>
	std::future<decay_t<_type>>
		make_ready_future(_type&& val)
	{
		using val_t = decay_t<_type>;
		std::promise<val_t> pm;

		pm.set_value(std::forward<val_t>(val));
		return pm.get_future();
	}
	inline
		std::future<void>
		make_ready_future()
	{
		std::promise<void> pm;

		pm.set_value();
		return pm.get_future();
	}
	template<typename _type>
	std::future<_type>
		make_ready_future(std::exception_ptr p)
	{
		std::promise<_type> pm;
		pm.set_exception(p);
		return pm.get_future();
	}
	//@}
#endif


	template<typename _fCallable, typename... _tParams>
	using future_result_t
		= std::future<invoke_result_t<_fCallable && (_tParams&&...)>>;

	template<typename _fCallable, typename... _tParams>
	using packed_task_t
		= std::packaged_task<invoke_result_t<_fCallable && (_tParams&&...)>()>;


	//@{
	template<typename _fCallable, typename... _tParams>
	packed_task_t<_fCallable&&, _tParams&&...>
		pack_task(_fCallable&& f, _tParams&&... args)
	{
		return packed_task_t<_fCallable&&, _tParams&&...>(
			std::bind(lforward(f), lforward(args)...));
	}

	template<typename _fCallable, typename... _tParams>
	std::shared_ptr<packed_task_t<_fCallable&&, _tParams&&...>>
		pack_shared_task(_fCallable&& f, _tParams&&... args)
	{
		return std::make_shared<packed_task_t<_fCallable&&,
			_tParams&&...>>(std::bind(lforward(f), lforward(args)...));
	}
	//@}

} // namespace leo;

#endif