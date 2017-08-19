/*!	\file mixin.hpp
\ingroup LBase
\brief ������̳еĻ���ӿڡ�
*/

#ifndef LBase_mixin_Hpp
#define LBase_mixin_Hpp 1

#include <LBase/tuple.hpp>
#include <LBase/utility.hpp>

namespace leo {

	namespace details {
		//@{
		/*!
		\brief �������
		\warning ����ʽ���������Ƿ�Ϊ��̬��ȡ���ڲ�����
		*/
		template<size_t N, class... _tBases>
		class mixin : public _tBases...
		{
		public:
			using tuple_type = std::tuple<_tBases...>;

			lconstfn
				mixin() = default;
			template<typename _tParam,
				limpl(typename = exclude_self_t<mixin, _tParam>)>
				lconstfn
				mixin(_tParam&& arg)
				: _tBases(lforward(arg))...
			{}
			template<typename _tParam1, typename _tParam2, typename... _tParams>
			lconstfn
				mixin(_tParam1&& arg1, _tParam2&& arg2, _tParams&&... args)
				: mixin(std::forward_as_tuple(lforward(arg1), lforward(arg2),
					lforward(args)...))
			{}
			template<typename... _tParams>
			lconstfn
				mixin(const std::tuple<_tParams...>& tp,
					limpl(enable_if_t<(sizeof...(_tBases) > 1)>* = {}))
				: mixin(index_sequence_for<_tParams...>(), tp)
			{}
			template<typename... _tParams>
			lconstfn
				mixin(std::tuple<_tParams...>&& tp,
					limpl(enable_if_t<(sizeof...(_tBases) > 1)>* = {}))
				: mixin(index_sequence_for<_tParams...>(), std::move(tp))
			{}
			template<size_t... _vSeq, typename... _tParams>
			lconstfn
				mixin(index_sequence<_vSeq...>, const std::tuple<_tParams...>& tp)
				: _tBases(lforward(std::get<_vSeq>(tp)))...
			{}
			template<size_t... _vSeq, typename... _tParams>
			lconstfn
				mixin(index_sequence<_vSeq...>, std::tuple<_tParams...>&& tp)
				: _tBases(lforward(std::get<_vSeq>(tp)))...
			{}
			lconstfn
				mixin(const mixin&) = default;
			lconstfn
				mixin(mixin&&) = default;

			tuple_type
				to_tuple() const
			{
				return this->template to_tuple(index_sequence_for<_tBases...>());
			}
			template<size_t... _vSeq>
			tuple_type
				to_tuple(index_sequence<_vSeq...>) const
			{
				return tuple_type(
					static_cast<const tuple_element_t<_vSeq, tuple_type>&>(*this)...);
			}
		};

		template<class... _tBases>
		class mixin<1, _tBases...> : public _tBases...
		{
		public:
			using tuple_type = std::tuple<_tBases...>;

			lconstfn
				mixin() = default;
			template<typename _tParam,
				limpl(typename = exclude_self_t<mixin, _tParam>)>
				lconstfn
				mixin(_tParam&& arg)
				: _tBases(lforward(arg))...
			{}
			template<typename _tParam1, typename _tParam2, typename... _tParams>
			lconstfn
				mixin(_tParam1&& arg1, _tParam2&& arg2, _tParams&&... args)
				: mixin(std::forward_as_tuple(lforward(arg1), lforward(arg2),
					lforward(args)...))
			{}
			template<typename... _tParams>
			lconstfn
				mixin(const std::tuple<_tParams...>& tp,
					limpl(enable_if_t<(sizeof...(_tBases) == 1)>* = {}))
				: mixin(std::get<0>(tp))
			{}
			template<typename... _tParams>
			lconstfn
				mixin(std::tuple<_tParams...>&& tp,
					limpl(enable_if_t<(sizeof...(_tBases) == 1)>* = {}))
				: mixin(std::get<0>(std::move(tp)))
			{}

			lconstfn
				mixin(const mixin&) = default;
			lconstfn
				mixin(mixin&&) = default;

			tuple_type
				to_tuple() const
			{
				return this->template to_tuple(index_sequence_for<_tBases...>());
			}
			template<size_t... _vSeq>
			tuple_type
				to_tuple(index_sequence<_vSeq...>) const
			{
				return tuple_type(
					static_cast<const tuple_element_t<_vSeq, tuple_type>&>(*this)...);
			}
		};
	}

	template<typename... _types>
	using mixin = details::mixin<sizeof...(_types), _types...>;

	/*!
	\ingroup metafunctions
	\brief ��װΪ�����ࡣ
	\note ʹ�� classify_value_t �Է������Ͱ�װΪ boxed_value ʵ����
	*/
	template<typename... _types>
	using wrap_mixin_t = mixin<classify_value_t<_types>...>;
	//@}
}

#endif