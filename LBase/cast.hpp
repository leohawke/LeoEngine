#ifndef LBase_cast_hpp
#define LBase_cast_hpp 1

#include "LBase/type_op.hpp"
#include "LBase/exception.h"
#include "LBase/cassert.h"
#include <limits>
#include <typeinfo>

namespace leo {

	/*!	\defgroup cast Cast
	\brief 显式类型转换。
	*/
	//@{
	/*!
	\pre 源类型和目标类型退化后相同。
	\note 指定源类型优先，可自动推导目标类型。
	\sa as_const
	*/
	//! \brief 允许添加限定符转换。
	template<typename _tSrc, typename _tDst = const decay_t<_tSrc>&&>
	lconstfn _tDst
		qualify(_tSrc&& arg) lnothrow
	{
		static_assert(is_same<decay_t<_tSrc>, decay_t<_tDst>>(),
			"Non-qualification conversion found.");

		return static_cast<_tDst>(arg);
	}


	//@{
	//! \ingroup binary_type_traits
	//@{
	template<typename _tDst, typename _tSrc>
	using is_narrowing_from_floating_to_integer
		= and_<is_integral<_tDst>, is_floating_point<_tSrc>>;

	template<typename _tDst, typename _tSrc>
	using is_narrowing_from_floating_to_floating
		= and_<is_floating_point<_tDst>, is_floating_point<_tSrc>,
		is_same<common_type_t<_tDst, _tSrc>, _tSrc>>;

	template<typename _tDst, typename _tSrc>
	using is_narrowing_from_floating
		= or_<is_narrowing_from_floating_to_integer<_tDst, _tSrc>,
		is_narrowing_from_floating_to_floating<_tDst, _tSrc>>;
	//@}

	namespace details
	{

		template<typename _tDst>
		struct narrow_test
		{
			_tDst i, j;

			lconstfn false_
				get() const lnothrow
			{
				return{};
			}
		};

		template<typename _tDst, typename _tSrc>
		lconstfn auto
			test_narrow(_tSrc v) lnothrow -> decltype(narrow_test<_tDst>{v}.get())
		{
			return narrow_test<_tDst>{v};
		}
		template<typename>
		lconstfn true_
			test_narrow(...) lnothrow
		{
			return{};
		}

	} // namespace details;
	  //@}

	  //@{
	  //! \brief 可能缩小数值范围的显式转换。
	template<typename _tDst, typename _tSrc>
	lconstfn _tDst
		narrow_cast(_tSrc v) lnothrow
	{
		static_assert(is_arithmetic<_tDst>(), "Invalid destination type found.");
		static_assert(is_arithmetic<_tSrc>(), "Invalid source type found.");
		static_assert(is_narrowing_from_floating<_tDst, _tSrc>()
			|| details::test_narrow<_tDst>(std::numeric_limits<_tSrc>::max())
			|| details::test_narrow<_tDst>(std::numeric_limits<_tSrc>::min()),
			"Invalid types found.");

		return static_cast<_tDst>(v);
	}

	//! \note 使用 ADL narrow_cast 。
	template<typename _tDst, typename _tSrc>
	inline _tDst
		narrow(_tSrc v)
	{
		const auto d(narrow_cast<_tDst>(v));

		if (static_cast<_tSrc>(d) == v && ((is_signed<_tDst>()
			== is_signed<_tSrc>()) || (d < _tDst()) == (v < _tSrc())))
			return d;
		throw narrowing_error();
	}

	namespace details
	{
		template<typename _tDst, typename _tSrc>
		lconstfn _tDst
			not_widen_cast(_tSrc v, true_) lnothrow
		{
			return narrow_cast<_tDst>(v);
		}
		template<typename _tDst, typename _tSrc>
		lconstfn _tDst
			not_widen_cast(_tSrc v, false_) lnothrow
		{
			static_assert(is_arithmetic<_tDst>(), "Invalid destination type found.");
			static_assert(is_arithmetic<_tSrc>(), "Invalid source type found.");
			static_assert(std::numeric_limits<_tSrc>::max() == std::numeric_limits<
				_tDst>::max() || std::numeric_limits<_tSrc>::min()
				== std::numeric_limits<_tDst>::min(), "Invalid types found.");
			return v;
		}

	} // namespace details;


	/*!
	\brief 可能缩小数值范围或数值范围最大和最小值保持不变的转换。
	\note 使用 ADL narrow_cast 。
	*/
	template<typename _tDst, typename _tSrc>
	lconstfn _tDst
		not_widen_cast(_tSrc v) lnothrow
	{
		return details::not_widen_cast<_tDst>(v,
			bool_<is_narrowing_from_floating<_tDst, _tSrc>()
			|| details::test_narrow<_tDst>(std::numeric_limits<_tSrc>::max())
			|| details::test_narrow<_tDst>(std::numeric_limits<_tSrc>::min())>());
	}

	//! \note 使用 ADL not_widen_cast 。
	template<typename _tDst, typename _tSrc>
	inline _tDst
		not_widen(_tSrc v)
	{
		const auto d(not_widen_cast<_tDst>(v));

		if (static_cast<_tSrc>(d) == v && ((is_signed<_tDst>()
			== is_signed<_tSrc>()) || (d < _tDst()) == (v < _tSrc())))
			return d;
		throw narrowing_error();
	}
	//@}


	/*!
	\brief 多态类指针类型转换。
	\tparam _tSrc 源类型。
	\tparam _pDst 目标类型。
	\pre 静态断言： _tSrc 是多态类。
	\pre 静态断言： _pDst 是内建指针。
	\throw std::bad_cast dynamic_cast 失败。
	*/
	template<typename _pDst, class _tSrc>
	inline _pDst
		polymorphic_cast(_tSrc* v)
	{
		static_assert(is_polymorphic<_tSrc>(), "Non-polymorphic class found.");
		static_assert(is_pointer<_pDst>(), "Non-pointer destination found.");

		if (const auto p = dynamic_cast<_pDst>(v))
			return p;
		throw std::bad_cast();
	}

	/*!
	\brief 多态类指针向派生类指针转换。
	*/
	//@{
	/*!
	\tparam _tSrc 源类型。
	\tparam _pDst 目标类型。
	\pre 静态断言： _tSrc 是多态类。
	\pre 静态断言： _pDst 是内建指针。
	\pre 静态断言： _tSrc 是 _pDst 指向的类去除修饰符后的基类。
	\pre 断言： dynamic_cast 成功。
	*/
	template<typename _pDst, class _tSrc>
	inline _pDst
		polymorphic_downcast(_tSrc* v) lnothrowv
	{
		static_assert(is_polymorphic<_tSrc>(), "Non-polymorphic class found.");
		static_assert(is_pointer<_pDst>(), "Non-pointer destination found.");
		static_assert(is_base_of<_tSrc, remove_cv_t<
			remove_pointer_t<_pDst>>>(), "Wrong destination type found.");

		lassume(dynamic_cast<_pDst>(v) == v);
		return static_cast<_pDst>(v);
	}
	/*!
	\tparam _tSrc 源类型。
	\tparam _rDst 目标类型。
	\pre 静态断言： _rDst 是左值引用。
	*/
	template<typename _rDst, class _tSrc>
	lconstfn limpl(enable_if_t)<is_lvalue_reference<_rDst>::value, _rDst>
		polymorphic_downcast(_tSrc& v) lnothrowv
	{
		return *polymorphic_downcast<remove_reference_t<_rDst>*>(
			std::addressof(v));
	}
	/*!
	\tparam _tSrc 源类型。
	\tparam _rDst 目标类型。
	\pre 静态断言： _rDst 是右值引用。
	*/
	template<typename _rDst, class _tSrc>
	lconstfn limpl(enable_if_t)<is_rvalue_reference<_rDst>::value
		&& !is_reference<_tSrc>::value, _rDst>
		polymorphic_downcast(_tSrc&& v) lnothrowv
	{
		return std::move(polymorphic_downcast<_rDst&>(v));
	}
	/*!
	\tparam _tSrc 源的元素类型。
	\tparam _tDst 目标的元素类型。
	\tparam _tDeleter 删除器类型。
	\pre _tDst 可通过 \c release() 取得的指针和删除器右值构造。
	\pre 断言：调用 polymorphic_downcast 转换 \c release() 取得的指针保证无异常抛出。
	\note 使用 ADL 调用 polymorphic_downcast 转换 \c release() 取得的指针。
	*/
	template<class _tDst, typename _tSrc, typename _tDeleter>
	inline limpl(enable_if_t)<!is_reference<_tDst>::value
		&& !is_array<_tDst>::value, std::unique_ptr<_tDst, _tDeleter>>
		polymorphic_downcast(std::unique_ptr<_tSrc, _tDeleter>&& v) lnothrowv
	{
		using dst_type = std::unique_ptr<_tDst, _tDeleter>;
		using pointer = typename dst_type::pointer;
		auto ptr(v.release());
		lnoexcept_assert("Invalid cast found.", polymorphic_downcast<pointer>(ptr));

		lassume(bool(ptr));
		return dst_type(polymorphic_downcast<pointer>(ptr),
			std::move(v.get_deleter()));
	}
	/*!
	\tparam _tSrc 源的元素类型。
	\tparam _tDst 目标的元素类型。
	\pre _tDst 可通过 \c get() 取得的指针和删除器右值构造。
	*/
	template<class _tDst, typename _tSrc>
	inline limpl(enable_if_t)<!is_reference<_tDst>::value
		&& !is_array<_tDst>::value, std::shared_ptr<_tDst>>
		polymorphic_downcast(const std::unique_ptr<_tSrc>& v) lnothrowv
	{
		lassume(dynamic_cast<_tDst*>(v.get()) == v.get());

		return std::static_pointer_cast<_tDst>(v);
	}
	//@}

	//! \brief 多态类指针交叉转换。
	//@{
	/*!
	\tparam _tSrc 源类型。
	\tparam _pDst 目标类型。
	\pre 静态断言： _tSrc 是多态类。
	\pre 静态断言： _pDst 是内建指针。
	\pre 断言： dynamic_cast 成功。
	\return 非空结果。
	\note 空指针作为参数一定失败。
	*/
	template<typename _pDst, class _tSrc>
	inline _pDst
		polymorphic_crosscast(_tSrc* v)
	{
		static_assert(is_polymorphic<_tSrc>(), "Non-polymorphic class found.");
		static_assert(is_pointer<_pDst>(), "Non-pointer destination found.");

		auto p(dynamic_cast<_pDst>(v));

		lassume(p);
		return p;
	}
	/*!
	\tparam _tSrc 源类型。
	\tparam _rDst 目标类型。
	\pre 静态断言： _rDst 是左值引用。
	*/
	template<typename _rDst, class _tSrc>
	lconstfn _rDst
		polymorphic_crosscast(_tSrc& v)
	{
		static_assert(is_lvalue_reference<_rDst>(),
			"Invalid destination type found.");

		return *polymorphic_crosscast<remove_reference_t<_rDst>*>(
			std::addressof(v));
	}
	/*!
	\tparam _tSrc 源类型。
	\tparam _rDst 目标类型。
	\pre 静态断言： _rDst 是右值引用。
	*/
	template<typename _rDst, class _tSrc>
	lconstfn enable_if_t<!is_reference<_tSrc>::value, _rDst>
		polymorphic_crosscast(_tSrc&& v)
	{
		static_assert(is_rvalue_reference<_rDst>(),
			"Invalid destination type found.");

		return std::move(polymorphic_crosscast<_rDst&>(v));
	}
	//@}


	namespace details
	{

		//@{
		template<typename _tFrom, typename _tTo, bool _bNonVirtualDownCast>
		struct general_polymorphic_cast_helper
		{
			static lconstfn _tTo
				cast(_tFrom v)
			{
				return polymorphic_downcast<_tTo>(v);
			}
		};

		template<typename _tFrom, typename _tTo>
		struct general_polymorphic_cast_helper<_tFrom, _tTo, false>
		{
			static lconstfn _tTo
				cast(_tFrom v)
			{
				return dynamic_cast<_tTo>(v);
			}
		};


		template<typename _tFrom, typename _tTo, bool _bUseStaticCast>
		struct general_cast_helper
		{
			static lconstfn _tTo
				cast(_tFrom v)
			{
				return _tTo(v);
			}
		};

		template<typename _tFrom, typename _tTo>
		struct general_cast_helper<_tFrom, _tTo, false>
		{
			static lconstfn _tTo
				cast(_tFrom v)
			{
				return general_polymorphic_cast_helper<_tFrom, _tTo, and_<is_base_of<
					_tFrom, _tTo>, not_<has_common_nonempty_virtual_base<
					_t<remove_rp<_tFrom>>, _t<remove_rp<_tTo>>>>>::value>::cast(v);
			}
		};

		template<typename _type>
		struct general_cast_helper<_type, _type, true>
		{
			static _type
				cast(_type v)
			{
				return v;
			}
		};

		template<typename _type>
		struct general_cast_helper<_type, _type, false>
		{
			static lconstfn _type
				cast(_type v)
			{
				return v;
			}
		};

		template<typename _tFrom, typename _tTo>
		struct general_cast_type_helper : is_convertible<_tFrom, _tTo>
		{};
		//@}

	} // namespace details;

	  /*!
	  \brief 一般类型转换。
	  \tparam _tSrc 源类型。
	  \tparam _tDst 目标类型。
	  \todo 扩展接受右值引用参数。

	  能确保安全隐式转换时使用 static_cast ；
	  除此之外非虚基类向派生类转换使用 polymophic_downcast；
	  否则使用 dynamic_cast。
	  */
	  //@{
	template<typename _tDst, typename _tSrc>
	lconstfn _tDst
		general_cast(_tSrc* v)
	{
		return details::general_cast_helper<_tSrc*, _tDst,
			details::general_cast_type_helper<_tSrc*, _tDst>::value>::cast(v);
	}
	template<typename _tDst, typename _tSrc>
	lconstfn _tDst
		general_cast(_tSrc& v)
	{
		return details::general_cast_helper<_tSrc&, _tDst,
			details::general_cast_type_helper<_tSrc&, _tDst>::value>::cast(v);
	}
	template<typename _tDst, typename _tSrc>
	lconstfn const _tDst
		general_cast(const _tSrc& v)
	{
		return details::general_cast_helper<const _tSrc&, _tDst, details
			::general_cast_type_helper<const _tSrc&, const _tDst>::value>::cast(v);
	}
	//@}
	//@}
}

#endif
