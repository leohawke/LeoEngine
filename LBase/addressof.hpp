/*! \file addressof.hpp
\ingroup LBase

提供和一元操作符 & 相关的接口：
检查是否重载 operator& 的特征；
以及非重载时能有 constexpr 的 leo::addressof 替代 constfn_addressof 。
\note 防止引入<memory>
*/

#ifndef LBase_addressof_hpp
#define LBase_addressof_hpp 1

#include "LBase/type_traits.hpp"

namespace leo {
		namespace details {
			// TEMPLATE FUNCTION addressof
			template<class _Ty> inline
				_Ty *_Addressof(_Ty& _Val, true_type) lnoexcept
			{	// return address of function _Val
				return (_Val);
			}

			template<class _Ty> inline
				_Ty *_Addressof(_Ty& _Val, false_type) lnoexcept
			{	// return address of object _Val
				return (reinterpret_cast<_Ty *>(
					&const_cast<char&>(
						reinterpret_cast<const volatile char&>(_Val))));
			}
		}

		template<class _Ty> inline
			_Ty *addressof(_Ty& _Val) lnoexcept
		{	// return address of _Val
			return (details::_Addressof(_Val, is_function<_Ty>()));
		}

		namespace details {
			//@{
			template<typename _type>
			using addressof_mem_t = decltype(std::declval<const _type&>().operator&());

			template<typename _type>
			using addressof_free_t = decltype(operator&(std::declval<const _type&>()));
			//@}
		}

		/*!
		\ingroup unary_type_traits
		\brief 判断是否存在合式重载 & 操作符接受指定类型的表达式。
		\pre 参数不为不完整类型。
		\since build 1.4
		*/
		template<typename _type>
		struct has_overloaded_addressof
			: or_<is_detected<details::addressof_mem_t, _type>,
			is_detected<details::addressof_free_t, _type >>
		{};


		/*!
		\brief 尝试对非重载 operator& 提供 constexpr 的 addressof 替代。
		\note 当前 ISO C++ 的 std::addressof 无 constexpr 。
		\since build 1.4
		\see http://wg21.cmeerw.net/lwg/issue2296 。
		*/
		//@{
		template<typename _type>
		lconstfn limpl(enable_if_t)<!has_overloaded_addressof<_type>::value, _type*>
			constfn_addressof(_type& r)
		{
			return &r;
		}
		//! \note 因为可移植性需要，不能直接提供 constexpr 。
		template<typename _type,
			limpl(typename = enable_if_t<has_overloaded_addressof<_type>::value>)>
			inline _type*
			constfn_addressof(_type& r)
		{
			return addressof(r);
		}
		//@}


		/*!
		\brief 取 \c void* 类型的指针。
		\note 适合 std::fprintf 等的 \c %p 转换规格。
		\since build 1.4
		*/
		//@{
		template<typename _type>
		lconstfn limpl(enable_if_t)<or_<is_object<_type>, is_void<_type>>::value, void*>
			pvoid(_type* p) lnothrow
		{
			return const_cast<void*>(static_cast<const volatile void*>(p));
		}
		//! \note ISO C++11 指定函数指针 \c reinterpret_cast 为对象指针有条件支持。
		template<typename _type>
		inline limpl(enable_if_t)<is_function<_type>::value, void*>
			pvoid(_type* p) lnothrow
		{
			return const_cast<void*>(reinterpret_cast<const volatile void*>(p));
		}

		/*!
		\return 非空指针。
		\since build 1.4
		*/
		template<typename _type>
		lconstfn void*
			pvoid_ref(_type&& ref)
		{
			return pvoid(addressof(ref));
		}
		//@}
}
#endif
