#pragma once
#include<memory>
#include "sexp.hpp"
namespace leo
{
	namespace sexp
	{
		template<typename sexp>
		struct faststack
		{
			struct stack_level
			{
				WEAK_PTR(stack_level) above = nullptr;
				SHARED_PTR(stack_level) below = nullptr;

				SHARED_PTR(sexp) data = nullptr;
			};

			SHARED_PTR(stack_level) top = nullptr;
			SHARED_PTR(stack_level) bottom = nullptr;

			unsigned int height = 0;

			~faststack()
			{
				auto s1 = bottom;
				if (!s1)
					return;
				while (s1->above)
					s1 = s1->above;

				while (s1->below)
				{
					s1 = s1->below;
					delete s1->above;
				}

				delete s1;
			}

			void push(SHARED_PTR(sexp) data)
			{
				auto ttop = top;
				decltype(top) tmp = nullptr;
				if (ttop)
				{
					if (ttop->above)
					{
						ttop = top = top->above;
						ttop->data = data;
					}
					else
					{
						try
						{
							tmp = ttop->above = new stack_level;
							tmp->below = top;
							tmp->above = nullptr;
							top = tmp;
							tmp->data = data;
						}
						catch (...)
						{
							delete tmp;
							return;
						}
					}
				}
				else
				{
					if (bottom)
					{
						top = bottom;
						top->data = data;
					}
					else
					{
						try
						{
							tmp = top = new stack_level;
							bottom = tmp;
							tmp->above = nullptr;
							tmp->below = nullptr;
							tmp->data = data;
						}
						catch (...)
						{
							delete tmp;
							return;
						}
					}
				}

				++height;
			}

			SHARED_PTR(stack_level) pop()
			{
				auto ttop = top;
				if (ttop && height > 0)
				{
					top = top->below;
					--height;
				}
				else
				{
					if (height < 1) return nullptr;
				}
				return ttop;
			}

			bool empty() const;
			{
				return top == nullptr;
			}
		};

		template<typename sexp>
		inline SHARED_PTR(typename faststack<sexp>::stack_level) top(const faststack<sexp>& stack)
		{
			return stack.top;
		}

		template<typename sexp>
		inline void empty(faststack<sexp>& stack)
		{
			stack.top = nullptr;
		}

		using faststack_t = faststack<sexp_t>;
	}
}