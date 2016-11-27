/*! \file type_pun.hpp
\ingroup LBase
\brief 共享存储和直接转换。

*/
#ifndef LBase_type_pun_hpp
#define LBase_type_pun_hpp 1


#include "LBase/placement.hpp" // for "type_traits.hpp", bool_constant, lalignof,
//	and_, is_trivial, enable_if_t, is_object_pointer, remove_pointer_t,
//	aligned_storage_t, is_reference, remove_reference_t, exclude_self_t,
//	decay_t;
#include <new> // for placement ::operator new from standard library;

#ifdef LB_IMPL_MSCPP
#pragma warning(push)
#pragma warning(disable:4554)
#endif

namespace leo
{
	using stdex::byte;
	/*!	\defgroup aligned_type_traits Aligned Type Traits
	\ingroup binary_type_traits
	\brief 对齐类型特征。
	\note 第一参数类型比第二参数类型对齐要求更严格或相同。
	*/
	//@{
	/*!
	\brief 判断是否对齐兼容。
	\since build 1.4
	*/
	template<typename _type, typename _type2>
	struct is_aligned_compatible : bool_constant<!(lalignof(_type)
		< lalignof(_type2)) & lalignof(_type) % lalignof(_type2) == 0>
	{};


	/*!
	\brief 判断是否可原地对齐存储。
	\since build 1.4
	*/
	template<typename _type, typename _tDst>
	struct is_aligned_placeable : bool_constant<sizeof(_type)
		== sizeof(_tDst) && is_aligned_compatible<_type, _tDst>::value>
	{};


	/*!
	\brief 判断是否可对齐存储。
	\since build 1.4
	*/
	template<typename _type, typename _tDst>
	struct is_aligned_storable : bool_constant<!(sizeof(_type) < sizeof(_tDst))
		&& is_aligned_compatible<_type, _tDst>::value>
	{};


	/*!
	\brief 判断是否可替换对齐存储。
	\since build 1.4
	*/
	template<typename _type, typename _tDst>
	struct is_aligned_replaceable : and_<is_aligned_storable<_type, _tDst>,
		is_aligned_storable<_tDst, _type>>
	{};


	/*!
	\brief 判断是否可平凡替换存储。
	\todo 使用 \c is_trivially_copyable 代替 is_trivial 。
	*/
	template<typename _type, typename _tDst>
	struct is_trivially_replaceable : and_<is_trivial<_type>, is_trivial<_tDst>,
		is_aligned_replaceable<_type, _tDst>>
	{};
	//@}


	//! \since build 1.4
	//@{
	/*!
	\ingroup metafunctions
	\brief 选择和特定类型可替换字符类类型的特定重载以避免冲突。
	*/
	template<typename _tSrc, typename _tDst, typename _type = void>
	using enable_if_replaceable_t
		= enable_if_t<is_trivially_replaceable<_tSrc, _tDst>::value, _type>;


	/*!
	\ingroup transformation_traits
	\brief 显式转换的替代对齐存储对齐存储类型。
	*/
	template<typename _type, size_t _vAlign = lalignof(_type)>
	using pun_storage_t = aligned_storage_t<sizeof(_type), _vAlign>;


	/*!
	\ingroup cast
	\pre 目标类型是对象指针或引用。
	\note 使用 \c reinterpret_cast 且保证目标类型的对齐要求不比源类型更严格以保证可逆。
	*/
	//@{
	/*!
	\brief 保证对齐兼容的显式转换。
	\note 用于替代针对满足 is_aligned_compatible 要求对象的 \c reinterpret_cast 。
	*/
	//@{
	template<typename _pDst, typename _tSrc>
	inline limpl(enable_if_t) < and_<is_object_pointer<_pDst>,
		is_aligned_compatible<_tSrc, remove_pointer_t<_pDst>>>::value, _pDst >
		aligned_cast(_tSrc* v) lnothrow
	{
		return reinterpret_cast<_pDst>(v);
	}
	template<typename _rDst, typename _tSrc, limpl(typename = enable_if_t<
		and_<is_reference<_rDst>, is_aligned_compatible<remove_reference_t<_tSrc>,
		remove_reference_t<_rDst>>>::value>)>
		inline auto
		aligned_cast(_tSrc&& v) lnothrow
		-> decltype(lforward(reinterpret_cast<_rDst>(v)))
	{
		return lforward(reinterpret_cast<_rDst>(v));
	}
	//@}


	/*!
	\brief 保证对齐存储的显式转换。
	\note 用于替代针对满足 is_aligned_storable 要求对象的 \c reinterpret_cast 。
	*/
	//@{
	template<typename _pDst, typename _tSrc>
	inline limpl(enable_if_t) < and_<is_object_pointer<_pDst>,
		is_aligned_storable<_tSrc, remove_pointer_t<_pDst>>>::value, _pDst >
		aligned_store_cast(_tSrc* v) lnothrow
	{
		return reinterpret_cast<_pDst>(v);
	}
	template<typename _rDst, typename _tSrc, limpl(typename = enable_if_t<
		and_<is_reference<_rDst>, is_aligned_storable<remove_reference_t<_tSrc>,
		remove_reference_t<_rDst>>>::value>)>
		inline auto
		aligned_store_cast(_tSrc&& v) lnothrow
		-> decltype(lforward(reinterpret_cast<_rDst>(v)))
	{
		return lforward(reinterpret_cast<_rDst>(v));
	}
	//@}


	/*!
	\brief 保证对齐替换存储的显式转换。
	\note 用于替代针对满足 is_aligned_replaceable 要求对象的 \c reinterpret_cast 。
	*/
	//@{
	template<typename _pDst, typename _tSrc>
	inline limpl(enable_if_t) < and_<is_object_pointer<_pDst>,
		is_aligned_replaceable<_tSrc, remove_pointer_t<_pDst>>>::value, _pDst >
		aligned_replace_cast(_tSrc* v) lnothrow
	{
		return reinterpret_cast<_pDst>(v);
	}
	template<typename _rDst, typename _tSrc, limpl(typename = enable_if_t<
		and_<is_reference<_rDst>, is_aligned_replaceable<remove_reference_t<_tSrc>,
		remove_reference_t<_rDst>>>::value>)>
		inline auto
		aligned_replace_cast(_tSrc&& v) lnothrow
		-> decltype(lforward(reinterpret_cast<_rDst>(v)))
	{
		return lforward(reinterpret_cast<_rDst>(v));
	}
	//@}


	/*!
	\brief 保证平凡替换存储的显式转换。
	\note 用于替代针对满足 is_trivially_replaceable 要求对象的 \c reinterpret_cast 。
	\since build 1.4
	*/
	//@{
	template<typename _pDst, typename _tSrc>
	inline limpl(enable_if_t) < and_<is_object_pointer<_pDst>,
		is_trivially_replaceable<_tSrc, remove_pointer_t<_pDst>>>::value, _pDst >
		replace_cast(_tSrc* v) lnothrow
	{
		return reinterpret_cast<_pDst>(v);
	}
	template<typename _rDst, typename _tSrc, limpl(typename = enable_if_t<
		and_<is_reference<_rDst>, is_trivially_replaceable<remove_reference_t<_tSrc>,
		remove_reference_t<_rDst>>>::value>)>
		inline auto
		replace_cast(_tSrc&& v) lnothrow
		-> decltype(lforward(reinterpret_cast<_rDst>(v)))
	{
		return lforward(reinterpret_cast<_rDst>(v));
	}
	//@}
	//@}
	//@}

	/*!
	\brief 避免严格别名分析限制的缓冲引用。
	\tparam _type 完整或不完整的对象类型。
	*/
	template<typename _type>
	class pun_ref
	{
		static_assert(is_object<_type>::value, "Invalid type found.");

	private:
		//! \invariant \c alias 。
		_type& alias;

	public:
		//! \pre 实例化时对象类型完整。
		//@{
		/*!
		\pre 指针参数非空。
		*/
		//@{
		pun_ref(decltype(default_init),void* p)
			: alias(*::new(p) _type)
		{}
		template<typename... _tParams>
		pun_ref(void* p, _tParams&&... args)
			: alias(*::new(p) _type(lforward(args)...))
		{}
		//@}
		~pun_ref()
		{
			alias.~_type();
		}
		//@}

		_type&
			get() const lnothrow
		{
			return alias;
		}
	};

	//@{
	//! \brief 避免严格别名分析限制的缓冲独占所有权指针。
	template<typename _type>
	using pun_ptr = placement_ptr<decay_t<_type>>;


	//! \brief 构造避免严格别名分析限制的缓冲独占所有权指针指定的值初始化对象。
	template<typename _type, typename _tObj, typename... _tParams>
	inline pun_ptr<_type>
		make_pun(_tObj& obj, _tParams&&... args)
	{
		return pun_ptr<_type>(
			leo::construct_within<decay_t<_type>>(obj, lforward(args)...));
	}

	/*!
	\brief 构造避免严格别名分析限制的缓冲独占所有权指针指定的默认初始化对象。
	*/
	template<typename _type, typename _tObj>
	inline pun_ptr<_type>
		make_pun_default(_tObj& obj)
	{
		return
			pun_ptr<_type>(leo::construct_default_within<decay_t<_type>>(obj));
	}
	//@}

	/*!
	\brief 任意标准布局类型存储。
	\since build 1.4
	*/
	template<typename _tUnderlying = aligned_storage_t<sizeof(void*)>>
	struct standard_layout_storage
	{
		static_assert(is_standard_layout<_tUnderlying>::value,
			"Invalid underlying type found.");

		using underlying = _tUnderlying;

		underlying object;

		standard_layout_storage() = default;
		standard_layout_storage(const standard_layout_storage&) = default;
		//! \since build 1.4
		template<typename _type,
			limpl(typename = exclude_self_t<standard_layout_storage, _type>)>
			inline
			standard_layout_storage(_type&& x)
		{
			new(access()) decay_t<_type>(lforward(x));
		}

		standard_layout_storage&
			operator=(const standard_layout_storage&) = default;
		/*!
		\note 为避免类型错误，需要确定类型时应使用显式使用 access 指定类型赋值。
		\since build 1.4
		*/
		template<typename _type,
			limpl(typename = exclude_self_t<standard_layout_storage, _type>)>
			inline standard_layout_storage&
			operator=(_type&& x)
		{
			assign(lforward(x));
			return *this;
		}

		lconstfn_relaxed LB_PURE void*
			access() lnothrow
		{
			return &object;
		}
		lconstfn LB_PURE const void*
			access() const lnothrow
		{
			return &object;
		}
		template<typename _type>
		lconstfn_relaxed LB_PURE _type&
			access() lnothrow
		{
			static_assert(is_aligned_storable<standard_layout_storage, _type>(),
				"Invalid type found.");

			return *static_cast<_type*>(access());
		}
		template<typename _type>
		lconstfn LB_PURE const _type&
			access() const lnothrow
		{
			static_assert(is_aligned_storable<standard_layout_storage, _type>(),
				"Invalid type found.");

			return *static_cast<const _type*>(access());
		}

		//@{
		//! \pre 对象未通过 construct 或构造模板创建。
		template<typename _type, typename... _tParams>
		inline _type*
			construct(_tParams&&... args)
		{
			return leo::construct_within<decay_t<_type>>(object,
				lforward(args)...);
		}

		//! \pre 指定类型的对象已通过 construct 或构造模板创建。
		template<typename _type>
		inline void
			destroy()
		{
			leo::destruct_in(access<decay_t<_type>>());
		}

		//! \pre 对象未通过 construct 或构造模板创建。
		//@{
		template<typename _type, typename... _tParams>
		inline pun_ptr<_type>
			pun(_tParams&&... args)
		{
			return leo::make_pun<_type>(object, lforward(args)...);
		}

		template<typename _type, typename... _tParams>
		inline pun_ptr<_type>
			pun_default()
		{
			return leo::make_pun_default<_type>(object);
		}
		//@}
		//@}

		lconstfn_relaxed LB_PURE byte*
			data() lnothrow
		{
			return static_cast<byte*>(access());
		}
		lconstfn LB_PURE const byte*
			data() const lnothrow
		{
			return static_cast<const byte*>(access());
		}

		//! \since build 1.4
		template<typename _type>
		inline void
			assign(_type&& x)
		{
			access<decay_t<_type>>() = lforward(x);
		}
	};


	/*!
	\ingroup transformation_traits
	\brief 显式转换的替代标准布局替代存储类型。
	\since build 1.4
	*/
	template<typename _type, size_t _vAlign = lalignof(_type)>
	using replace_storage_t
		= standard_layout_storage<pun_storage_t<_type, _vAlign>>;

} // namespace leo;

#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif

#endif
