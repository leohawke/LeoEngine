/*!\file variadic.hpp
\ingroup LBase
\brief C++ 变长参数相关操作。
*/

#ifndef LBase_variadic_hpp
#define LBase_variadic_hpp 1

#include "LBase/type_traits.hpp" // for "type_traits.hpp", _t, size_t, empty_base,
//	false_type, is_same, true_type, are_same, integral_constant, conditional_t,
//	cond_t;

namespace leo
{
	/*!	\defgroup vseq_operations Variadic Sequence Operations
	\ingroup meta_operations
	\brief 变长参数标记的序列相关的元操作。
	\since build 1.4

	形式为模板类名声明和特化的相关操作，被操作的序列是类类型。
	除此处的特化外，可有其它类类型的特化。
	特化至少需保证具有表示和此处特化意义相同的 type 类型成员，并可能有其它成员。
	对非类型元素，成员 value 表示结果，成员 type 表示对应的序列类型。
	声明的元操作具有最后一个 void 默认模板参数时，其中的类模板允许被库的用户代码特化，
	最后一个参数用于 SFINAE 。
	*/

	//!\since build 1.4
	namespace vseq
	{

		/*!
		\ingroup vseq_operations
		\since build 1.4
		*/
		//@{
		/*!
		\brief 元函数应用。

		要求被应用的函数是具有可作为元函数调用的嵌套成员 apply 的类型。
		*/
		//@{
		template<class _func, typename... _tParams>
		using apply = typename _func::template apply<_tParams...>;

		template<class _func, typename... _tParams>
		using apply_t = _t<apply<_func, _tParams...>>;
		//@}


#define LB_Impl_Variadic_SeqOp(_n, _tparams, _targs) \
	template<_tparams, typename = void> \
	struct _n : _n<_targs, when<true>> \
	{}; \
	\
	template<_tparams, typename _tEnabler = void> \
	using _n##_t = _t<_n<_targs, _tEnabler>>;
#define LB_Impl_Variadic_SeqOpU(_n) \
	LB_Impl_Variadic_SeqOp(_n, class _tSeq, _tSeq)
#define LB_Impl_Variadic_SeqOpB(_n) LB_Impl_Variadic_SeqOp(_n, class _tSeq1 \
	LPP_Comma class _tSeq2, _tSeq1 LPP_Comma _tSeq2)
#define LB_Impl_Variadic_SeqOpI(_n) LB_Impl_Variadic_SeqOp(_n, class _tSeq \
	LPP_Comma size_t _vIdx, _tSeq LPP_Comma _vIdx)
#define LB_Impl_Variadic_SeqOpN(_n) LB_Impl_Variadic_SeqOp(_n, size_t _vN \
	LPP_Comma class _tSeq, _vN LPP_Comma _tSeq)


		/*!
		\brief 延迟求值。
		\since build 1.4
		*/
		//@{
		LB_Impl_Variadic_SeqOp(defer, template<typename...> class _gOp LPP_Comma class
			_tSeq, _gOp LPP_Comma _tSeq)

			template<template<typename...> class _gOp, template<typename...> class _gSeq,
			typename... _types>
		struct defer<_gOp, _gSeq<_types...>, when_valid<_gOp<_types...>>>
		{
			using type = _gOp<_types...>;
		};


		LB_Impl_Variadic_SeqOp(defer_i, typename _type LPP_Comma template<_type...>
		class _gOp LPP_Comma class _tSeq, _type LPP_Comma _gOp LPP_Comma _tSeq)

			template<typename _tInt, template<_tInt...> class _gOp,
			template<typename, _tInt...> class _gSeq, _tInt... _vSeq>
		struct defer_i<_tInt, _gOp, _gSeq<_tInt, _vSeq...>, when_valid<_gOp<_vSeq...>>>
		{
			using type = _gOp<_vSeq...>;
		};
		//@}


		/*!
		\brief 可变参数类型列表延迟求值。
		\sa defer
		\since build 1.4
		*/
		template<template<typename...> class _gOp, typename... _types>
		struct vdefer : defer<_gOp, empty_base<_types...>>
		{};

		//! \since build 1.4
		template<class _func, class _tParamsList>
		using defer_apply_t = _t<defer_t<_func::template apply, _tParamsList>>;
		//@}


		/*!	\defgroup metafunction_composition Metafunction Compositions
		\ingroup metafunctions
		\brief 元函数组合操作。
		\sa vseq::apply
		\since build 1.4

		组合满足元函数应用条件的类型的模板。
		要求输入的元函数是满足元函数应用条件的类型。
		*/
		//@{
		//! \brief 部分应用。
		//@{
		template<class _func, typename... _tParams>
		struct bind_back
		{
			template<typename... _types>
			using apply = vseq::apply<_func, _types..., _tParams...>;
		};


		template<class _func, typename... _tParams>
		struct bind_front
		{
			template<typename... _types>
			using apply = vseq::apply<_func, _tParams..., _types...>;
		};
		//@}


		/*!
		\note vdefer 是必要的，否则别名模板作为元函数时无法推导参数。
		\sa vdefer
		\see http://wg21.cmeerw.net/lwg/issue1430 。
		*/
		//@{
		/*!
		\brief 构造具有 \c apply 成员的一阶元函数。
		\since build 1.4
		*/
		template<template<typename...> class _gOp>
		struct _a
		{
			template<typename... _types>
			using apply = vdefer<_gOp, _types...>;
		};


		/*!
		\brief 引用：延迟求值变换。
		\since build 1.4
		\todo 支持没有 \c apply 成员的非元函数。
		*/
		template<class _func>
		struct _q
		{
			// NOTE: Ideally, the template argument should be limited to enable check.
			//	However this is impossible since the arity of '_func::apply' is
			//	unknown.
			template<typename... _types>
			using apply = vdefer<apply, _func, _types...>;
		};
		//@}
		//@}


		//! \since build 1.4
		//@{
		/*!
		\note 类型匹配操作：默认实现不依赖其它序列操作。
		\note 除参数为类型参数的类模板外不提供默认实现，需对具体序列特化。
		*/
		//@{
		/*!
		\brief 取类型构造器：具有 apply 成员，结果的嵌套 type 类型为构造类型的元函数。
		\note 默认实现拆分类模板实例并移除类型参数。
		\sa _a
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(ctor_of)

			template<template<typename...> class _gSeq, typename... _types>
		struct ctor_of<_gSeq<_types...>>
		{
			using type = _a<_gSeq>;
		};
		//@}


		/*!
		\brief 取类型构造器参数。
		\note 默认实现拆分类模板实例并移除类型参数。
		\sa ctor_of
		\sa empty_base
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(params_of)

			template<template<typename...> class _gSeq, typename... _types>
		struct params_of<_gSeq<_types...>>
		{
			using type = empty_base<_types...>;
		};
		//@}
		//@}


		//! \brief 使用构造器应用指定列表参数。
		template<class _tSeq, class _tParamsList>
		using instance_apply_t = defer_apply_t<ctor_of_t<_tSeq>, _tParamsList>;


		/*!
		\brief 判断是否可匹配类型实例。
		\note 作为使用 apply 代替模板参数求值的 is_decomposable 的一般化。
		\sa apply_t
		\sa is_decomposable
		*/
		//@{
		template<class, typename = void>
		struct is_instance : false_type
		{};

		template<class _tSeq>
		struct is_instance<_tSeq, enable_if_t<is_same<_tSeq, _t<
			vdefer<instance_apply_t, _tSeq, params_of_t<_tSeq>> >> ::value >> : true_type
		{};
		//@}


		template<class... _tSeqs>
		using same_instance = are_same<ctor_of_t<_tSeqs>...>;


		/*!
		\sa enable_when
		\sa is_instance
		\sa same_instance
		*/
		template<typename... _tSeqs>
		using enable_for_instances = enable_when<and_<is_instance<_tSeqs>...,
			same_instance<_tSeqs... >> ::value>;
		//@}


		//! \note 基本序列操作：默认实现不依赖类型匹配、构造、取参数和应用外的其它元操作。
		//@{
		//! \brief 单位元：取空序列。
		//@{
		LB_Impl_Variadic_SeqOpU(clear)

			//! \since build 684
			template<class _tSeq>
		struct clear<_tSeq, when<true>>
		{
			using type = apply_t<ctor_of_t<_tSeq>>;
		};
		//@}


		/*!
		\brief 连接：合并序列。
		\note 默认实现合并类模板实例的类型参数。
		*/
		//@{
		LB_Impl_Variadic_SeqOpB(concat)

			//! \since build 1.4
			//@{
			template<typename... _types1, typename... _types2>
		struct concat<empty_base<_types1...>, empty_base<_types2...>>
		{
			using type = empty_base<_types1..., _types2...>;
		};

		template<class _tSeq1, class _tSeq2>
		struct concat<_tSeq1, _tSeq2, enable_for_instances<_tSeq1, _tSeq2>>
		{
			using type = instance_apply_t<_tSeq1,
				concat_t<params_of_t<_tSeq1>, params_of_t<_tSeq2>>>;
		};
		//@}


		/*!
		\brief 函子映射：使用指定类型构造器和序列参数构造新的类型。
		\since build 1.4
		*/
		//@{
		LB_Impl_Variadic_SeqOp(fmap, class _tCtor LPP_Comma class _tSeq,
			_tCtor LPP_Comma _tSeq)

			//! \since build 684
			template<class _tCtor, typename... _types>
		struct fmap<_tCtor, empty_base<_types...>>
		{
			using type = apply_t<_tCtor, _types...>;
		};

		//! \since build 1.4
		template<class _tCtor, class _tSeq>
		struct fmap<_tCtor, _tSeq, enable_for_instances<_tSeq>>
		{
			using type = defer_apply_t<_tCtor, params_of_t<_tSeq>>;
		};
		//@}


		/*!
		\brief 取序列第一个元素。
		\note 默认实现取类模板的第一个类型参数。
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(front)

			//! \since build 1.4
			template<typename _tHead, typename... _tTail>
		struct front<empty_base<_tHead, _tTail...>>
		{
			using type = _tHead;
		};

		//! \since build 1.4
		template<class _tSeq>
		struct front<_tSeq, enable_for_instances<_tSeq>>
		{
			using type = front_t<params_of_t<_tSeq>>;
		};
		//@}


		/*!
		\brief 取序列第一个元素以外的元素的序列。
		\note 默认实现移除类模板的第一个类型参数。
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(pop_front)

			//! \since build 1.4
			template<typename _tHead, typename... _tTail>
		struct pop_front<empty_base<_tHead, _tTail...>>
		{
			using type = empty_base<_tTail...>;
		};

		//! \since build 1.4
		template<class _tSeq>
		struct pop_front<_tSeq, enable_for_instances<_tSeq>>
		{
			using type = instance_apply_t<_tSeq, pop_front_t<params_of_t<_tSeq>>>;
		};
		//@}


		/*!
		\brief 取在序列末尾插入一个元素的序列。
		\note 默认实现在类模板的参数列表后添加类型参数。
		*/
		//@{
		LB_Impl_Variadic_SeqOp(push_back, class _tSeq LPP_Comma typename _tItem,
			_tSeq LPP_Comma _tItem)

			//! \since build 1.4
			template<typename... _types, typename _type>
		struct push_back<empty_base<_types...>, _type>
		{
			using type = empty_base<_types..., _type>;
		};

		//! \since build 1.4
		template<class _tSeq, typename _type>
		struct push_back<_tSeq, _type, enable_for_instances<_tSeq>>
		{
			using type
				= instance_apply_t<_tSeq, push_back_t<params_of_t<_tSeq>, _type>>;
		};
		//@}


		/*!
		\brief 取序列第一个元素以外的元素的序列。
		\note 默认实现在类模板的参数列表前添加类型参数。
		*/
		//@{
		//! \brief 取在序列起始插入一个元素的序列。
		LB_Impl_Variadic_SeqOp(push_front, class _tSeq LPP_Comma typename _tItem,
			_tSeq LPP_Comma _tItem)

			//! \since build 1.4
			template<typename... _types, typename _type>
		struct push_front<empty_base<_types...>, _type>
		{
			using type = empty_base<_type, _types...>;
		};

		//! \since build 1.4
		template<class _tSeq, typename _type>
		struct push_front<_tSeq, _type, enable_for_instances<_tSeq>>
		{
			using type
				= instance_apply_t<_tSeq, push_front_t<params_of_t<_tSeq>, _type>>;
		};
		//@}


		/*!
		\brief 取序列元素数。
		\note 默认实现拆分类模板实例并取类型参数的个数。
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(seq_size)

			//! \since build 1.4
			template<typename... _types>
		struct seq_size<empty_base<_types...>>
			: integral_constant<size_t, sizeof...(_types)>
		{};

		//! \since build 1.4
		template<class _tSeq>
		struct seq_size<_tSeq, enable_for_instances<_tSeq>>
			: seq_size_t<params_of_t<_tSeq>>
		{};
		//@}
		//@}


		/*!
		\brief 取指定位置的元素。
		\note 默认实现依赖 front 和 pop_front 。
		*/
		//@{
		LB_Impl_Variadic_SeqOpI(at)

			//! \since build 1.4
			template<class _tSeq, size_t _vIdx>
		struct at<_tSeq, _vIdx, when<true>>
			: at<pop_front_t<_tSeq>, _vIdx - 1>
		{};

		//! \since build 1.4
		template<class _tSeq>
		struct at<_tSeq, 0, when<true>> : front<_tSeq>
		{};
		//@}


		/*!
		\brief 取序列最后一个元素。
		\note 默认实现依赖 seq_size 和 at 。
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(back)

			//! \since build 1.4
			template<class _tSeq>
		struct back<_tSeq, when<true>> : at<_tSeq, seq_size<_tSeq>::value - 1>
		{};
		//@}


		/*!
		\brief 拆分序列前若干元素。
		\note 默认实现依赖 clear 、 front 、 push_front 、 pop_front 和 concat 。
		*/
		//@{
		LB_Impl_Variadic_SeqOpN(split_n)

			//! \since build 1.4
			//@{
			//! \note 使用二分实现减少递归实例化深度。
			template<size_t _vN, class _tSeq>
		struct split_n<_vN, _tSeq, when<true>>
		{
		private:
			using half = split_n<_vN / 2, _tSeq>;
			using last = split_n<_vN - _vN / 2, typename half::tail>;

		public:
			using type = concat_t<_t<half>, _t<last>>;
			using tail = typename last::tail;
		};

		template<class _tSeq>
		struct split_n<0, _tSeq, when<true>>
		{
			using type = clear_t<_tSeq>;
			using tail = _tSeq;
		};

		template<class _tSeq>
		struct split_n<1, _tSeq, when<true>>
		{
			using type = push_front_t<clear_t<_tSeq>, front_t<_tSeq>>;
			using tail = pop_front_t<_tSeq>;
		};
		//@}
		//@}


		/*!
		\brief 取序列最后元素以外的元素的序列。
		\note 默认实现依赖 seq_size 和 split 。
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(pop_back)

			//! \since build 1.4
			template<class _tSeq>
		struct pop_back<_tSeq, when<true>> : split_n<seq_size<_tSeq>::value - 1, _tSeq>
		{};
		//@}


		/*!
		\brief 删除指定位置的元素。
		\note 默认实现依赖 split 和 concat 。
		*/
		//@{
		LB_Impl_Variadic_SeqOp(erase, class _tSeq LPP_Comma size_t _vIdx LPP_Comma
			size_t _vEnd = _vIdx + 1, _tSeq LPP_Comma _vIdx LPP_Comma _vEnd)

			//! \since build 1.4
			template<class _tSeq, size_t _vIdx, size_t _vEnd>
		struct erase<_tSeq, _vIdx, _vEnd, when<true>>
		{
			static_assert(_vIdx <= _vEnd, "Invalid range found.");

		public:
			using type = concat_t<split_n_t<_vIdx, _tSeq>,
				typename split_n<_vEnd, _tSeq>::tail>;
		};
		//@}


		/*!
		\brief 查找元素。
		\note 默认实现依赖 front 和 pop_front 。
		*/
		//@{
		LB_Impl_Variadic_SeqOp(find, class _tSeq LPP_Comma typename _type,
			_tSeq LPP_Comma _type)

		namespace details
		{

			template<size_t _vN, class _tSeq, typename _type>
			struct find
			{
				static lconstexpr const size_t value = is_same<front_t<_tSeq>, _type>::value
					? 0 : find<_vN - 1, pop_front_t<_tSeq>, _type>::value + 1;
			};

			template<class _tSeq, typename _type>
			struct find<0, _tSeq, _type>
			{
				static lconstexpr const size_t value = 0;
			};

		} // namespace details;

		  //! \since build 1.4
		template<class _tSeq, typename _type>
		struct find<_tSeq, _type, when<true>> : integral_constant<size_t,
			details::find<seq_size<_tSeq>::value, _tSeq, _type>::value>
		{};
		//@}


		/*!
		\brief 取合并相同元素后的序列。
		\note 默认实现依赖 seq_size 、 pop_front 、 front 、 find 和 push_front 。
		\note 默认实现保留尾端元素。
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(deduplicate)

		namespace details
		{

			template<size_t, class _tSeq>
			struct deduplicate
			{
			private:
				using head = front_t<_tSeq>;
				using tail = pop_front_t<_tSeq>;
				using sub = deduplicate_t<tail>;

			public:
				using type = conditional_t<vseq::find<tail, head>::value
					== seq_size<tail>::value, push_front_t<sub, head>, sub>;
			};

			template<class _tSeq>
			struct deduplicate<0, _tSeq>
			{
				using type = _tSeq;
			};

			template<class _tSeq>
			struct deduplicate<1, _tSeq>
			{
				using type = _tSeq;
			};

		} // namespace details;

		  //! \since build 1.4
		template<class _tSeq>
		struct deduplicate<_tSeq, when<true>>
		{
			using type = _t<details::deduplicate<seq_size<_tSeq>::value, _tSeq>>;
		};
		//@}


		/*!
		\brief 重复连接序列元素。
		\note 默认实现依赖 clear 和 concat 。
		*/
		//@{
		LB_Impl_Variadic_SeqOpN(join_n)

			/*!
			\note 使用二分实现减少递归实例化深度。
			\since build 1.4
			*/
			template<size_t _vN, class _tSeq>
		struct join_n<_vN, _tSeq, when<true>>
		{
		private:
			using unit = _tSeq;
			using half = join_n_t<_vN / 2, unit>;

		public:
			using type = concat_t<concat_t<half, half>, join_n_t<_vN % 2, unit>>;
		};

		template<class _tSeq>
		struct join_n<0, _tSeq>
		{
			using type = clear_t<_tSeq>;
		};

		template<class _tSeq>
		struct join_n<1, _tSeq>
		{
			using type = _tSeq;
		};
		//@}


		/*!
		\brief 投影操作。
		\note 默认实现依赖 at 和 index_sequence ，参见模块 IntegerSequence 。
		*/
		LB_Impl_Variadic_SeqOp(project, class _tSeq LPP_Comma class _tIdxSeq,
			_tSeq LPP_Comma _tIdxSeq)


			/*!
			\brief 取逆序列。
			\note 默认实现依赖 seq_size 、 clear 和 pop_front 。
			*/
			//@{
			LB_Impl_Variadic_SeqOpU(reverse)

			//! \since build 1.4
			template<class _tSeq>
		struct reverse<_tSeq, when<true>>
		{
			using type = conditional_t<seq_size<_tSeq>::value == 0, clear_t<_tSeq>,
				concat_t<reverse_t<pop_front_t<_tSeq>>, front_t<_tSeq>>>;
		};
		//@}


		/*!
		\brief 取合并相邻相同元素后的序列。
		\note 默认实现依赖 seq_size 、 clear 、 pop_front 和 concat 。
		*/
		//@{
		LB_Impl_Variadic_SeqOpU(unique)

		namespace details
		{

			/*!
			\note 使用二分实现减少递归实例化深度。
			\since build 1.4
			*/
			//@{
			template<class _tSeq, size_t _vN = seq_size<_tSeq>::value>
			struct unique
			{
			private:
				using split = split_n<_vN / 2, _tSeq>;
				using half = _t<unique<typename split::type>>;
				using last = _t<unique<typename split::tail>>;
				using half_back = back_t<half>;
				using last_front = front_t<last>;

			public:
				using type = concat_t<half, cond_t<is_same<half_back, last_front>,
					pop_front_t<last>, last >> ;
			};

			template<class _tSeq>
			struct unique<_tSeq, 0>
			{
				using type = clear_t<_tSeq>;
			};

			template<class _tSeq>
			struct unique<_tSeq, 1>
			{
				using type = _tSeq;
			};
			//@}

		} // namespace details;

		  //! \since build 1.4
		template<class _tSeq>
		struct unique<_tSeq, when<true>>
		{
			using type = _t<details::unique<_tSeq>>;
		};


		/*!
		\brief 二元操作合并应用。
		\pre 二元操作符合交换律和结合律。
		\note 默认实现依赖 seq_size 、 front 和 split_n 。
		*/
		//@{
		LB_Impl_Variadic_SeqOp(fold, class _fBinary LPP_Comma typename _tState LPP_Comma
		class _tSeq, _fBinary LPP_Comma _tState LPP_Comma _tSeq)

			//! \since build 1.4
			//@{
		namespace details
		{

			//! \note 使用二分实现减少递归实例化深度。
			template<class _fBinary, typename _tState, typename _tSeq, size_t _vSize>
			struct fold
			{
			private:
				using parts = split_n<_vSize / 2, _tSeq>;
				using head = _t<parts>;
				using tail = typename parts::tail;

			public:
				using type = fold_t<_fBinary, fold_t<_fBinary, _tState, head>, tail>;
			};

			template<class _fBinary, typename _tState, typename _tSeq>
			struct fold<_fBinary, _tState, _tSeq, 0>
			{
				using type = _tState;
			};

			template<class _fBinary, typename _tState, typename _tSeq>
			struct fold<_fBinary, _tState, _tSeq, 1>
			{
				using type = apply_t<_fBinary, _tState, front_t<_tSeq>>;
			};

		} // namespace details;

		template<class _fBinary, typename _tState, typename _tSeq>
		struct fold<_fBinary, _tState, _tSeq, enable_for_instances<_tSeq>>
			: details::fold<_fBinary, _tState, _tSeq, seq_size_t<_tSeq>::value>
		{};
		//@}
		//@}


		//! \note 不提供默认实现，需对具体序列特化。
		//@{
		//! \brief 序列作为向量的加法。
		LB_Impl_Variadic_SeqOpB(vec_add)


			//! \brief 序列作为向量的减法。
			LB_Impl_Variadic_SeqOpB(vec_subtract)
			//@}

#undef LB_Impl_Variadic_SeqOpN
#undef LB_Impl_Variadic_SeqOpI
#undef LB_Impl_Variadic_SeqOpU
#undef LB_Impl_Variadic_SeqOp
			//@}

	} // namespace vseq;

	  //! \since build 1.4
	using vseq::vdefer;
}

#endif