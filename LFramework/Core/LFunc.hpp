/*!	\file LFunc.hpp
\ingroup LFrameWork/Core
\brief 函数调用和仿函数封装。
*/

#ifndef LFrameWork_Core_LFunc_Hpp
#define LFrameWork_Core_LFunc_Hpp 1

#include "LFramework/Core/LShellDefinition.h"

namespace leo
{
	/*!
	\brief 递归调用上下文。
	\note 用于构建依赖项。
	*/
	template<typename _tKey, typename _tValue>
	class GRecursiveCallContext
	{
	public:
		using KeyType = _tKey;
		using ValueType = _tValue;
		using CallerType = std::function<ValueType(const GRecursiveCallContext&)>;

	private:
		std::function<CallerType(const KeyType&)> build;

	public:
		template<typename _func, limpl(typename
			= exclude_self_t<GRecursiveCallContext, _func>)>
			GRecursiveCallContext(_func f)
			: build(f)
		{}
		DefDeCopyMoveCtorAssignment(GRecursiveCallContext)

			PDefHOp(ValueType, (), const KeyType& key) const
			ImplRet(build(key)(*this))
	};


	/*!
	\brief 注册处理器抽象模板：供派生类加载一个或多个键和指定类型关联的例程。
	\note 加载的键的数量和类型的数量需要保持一致。
	\warning 非虚析构。
	*/
	template<class _tDerived, typename _tKey, typename _fHandler>
	struct GHandlerRegisterBase
	{
	private:
		unordered_map<_tKey, _fHandler> registered_map{};

	public:
		PDefH(bool, Contains, const _tKey& key) const
			ImplRet(registered_map.count(key) != 0)

			DeclSEntry(template<_type, _fHandler> _fHandler GetRegister() const)

			template<typename... _tParams>
		auto
			Call(const _tKey& key, _tParams&&... args)
			-> result_of_t<_fHandler&(_tParams&&...)>
		{
			if (const auto f = registered_map[key])
			{
				// TODO: Do right trace.
				//	TraceDe(Notice, "Found registered handler: %s.",
				//		to_string(key).c_str());

				return f(lforward(args)...);
			}
			return result_of_t<_fHandler&(_tParams&&...)>();
		}

		template<class _type>
		void
			Register(const _tKey& key)
		{
			registered_map.emplace(key, static_cast<_tDerived*>(this)
				->template GetRegister<_type, _fHandler>(key));
		}
		template<typename _tIn, class _type, class _tTuple>
		void
			Register(_tIn first, _tIn last)
		{
			LAssert(first != last && std::distance(first, last)
				== decltype(std::distance(first, last))(
					std::tuple_size<_tTuple>::value + 1), "Wrong range found.");
			Register<_type>(*first);
			++first;
			LAssert((first == last) == (std::tuple_size<_tTuple>::value == 0),
				"Wrong number of parameters found.");
			//	static_if(std::tuple_size<_tTuple>::value != 0)
			//		RegisterTail<_tIn, std::tuple_element<0, _tTuple>,
			//			typename tuple_split<_tTuple>::tail>(first, last);
			RegisterTail<_tIn>(identity<_tTuple>(), first, last);
		}
		template<class _type, class... _types>
		void
			Register(std::initializer_list<string> il)
		{
			LAssert(il.size() == sizeof...(_types)+1,
				"Wrong size of initializer list found.");
			Register<std::initializer_list<string>::const_iterator, _type,
				tuple<_types...>>(il.begin(), il.end());
		}

	private:
		template<typename _tIn>
		void
			RegisterTail(identity<tuple<>>, _tIn first, _tIn last)
		{
			LAssert(first == last, "Wrong size of initializer list found.");
			lunused(first),
				lunused(last);
		}
		template<typename _tIn, class _type, class... _types>
		void
			RegisterTail(identity<tuple<_type, _types...>>, _tIn first,
				_tIn last)
		{
			Register<_tIn, _type, tuple<_types...>>(first, last);
		}
	};

} // namespace leo;


#endif