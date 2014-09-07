#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "sexp.hpp"
#include "faststack.hpp"

namespace leo
{
	namespace sexp
	{
		sexp_errcode_t sexp_errno = sexp_err_ok;
	
		void reset_sexp_errno()
		{
			sexp_errno = sexp_err_ok;
		}

		sexp_exception::sexp_exception(error_code_type ec, const std::string& s, level_type l)
			_NOEXCEPT
			: logged_event([&]{
			try
			{
				return s + ": " + formatmessage(ec);
			}
			catch (...)
			{
			}
			return std::string(s);
		}(), l),
			errcode(ec)
		{
			assert(ec != 0);
		}

		std::string
			sexp_exception::formatmessage(error_code_type ec) _NOEXCEPT
		{
			try
			{
				switch (ec)
				{
				default:
					break;
				}
				return std::string("sexp exception");
			}
			catch (...)
			{
			}
			return{};
		}


		void destroy_sexp(sexp_t * s)
		{
				if (!s)
					return;

				if (s->ty == sexp_list) {
					destroy_sexp(s->list);
				}
				else if (s->ty == sexp_value) {
					if (s->aty == sexp_binary && s->bindata) {
						delete [] s->bindata;
					}
					else if (!s->val) {
						delete [] s->val;
					}
				}

				s->val = nullptr;
				s->bindata = nullptr;

				destroy_sexp(s->next);

				s->next = s->list = nullptr;

				sexp_t_deallocate(s);
		}

		int print_sexp(char * buf, std::size_t size, const sexp_t * sx)
		{
			char *b = buf, *tc;
			std::size_t left = size;
			int depth = 0;
			size_t sz;
			if (!sx)
			{
				buf[0] = 0;
				return 0;
			}

			auto tmp = *sx;
			tmp.next = tmp.list = nullptr;

			auto fakehead = ops::copy_sexp(&tmp);

			if (!fakehead)
			{
				sexp_errno = sexp_err_memory;
				return -1;
			}

			fakehead->list = sx->list;
			fakehead->next = nullptr;

			auto stack = new faststack_t;
			if (!stack)
			{
				sexp_errno = sexp_err_memory;
				sexp_t_deallocate(fakehead);
				return -1;
			}

			stack->push(fakehead);

			auto top = stack->top;
			sexp_t * tdata = nullptr;
			while (stack->top)
			{
				top = stack->top;
				tdata = (sexp_t*)top->data;

				if (!tdata)
				{
					stack->pop();
					if (depth > 0)
					{
						b[0] = ')';
						++b;
						--left;
						--depth;
						if (left == 0)
						{
							sexp_errno = sexp_err_buffer_full;
							break;
						}
					}

					if (!stack->top)
						break;

					top = stack->top;
					top->data = ((sexp_t*)top->data)->next;
					if (top->data)
					{
						b[0] = ' ';
						++b;
						--left;
						if (left == 0)
						{
							sexp_errno = sexp_err_buffer_full;
							break;
						}
					}
				}
				else if (tdata->aty == sexp_value)
				{
					if (tdata->aty == sexp_dquote)
					{
						b[0] = '\"';
						++b;
						--left;
					}
					else if (tdata->aty == sexp_squote)
					{
						b[0] = '\'';
						++b;
						--left;
					}

					if (tdata->aty != sexp_binary && tdata->val_used > 0)
					{
						auto tc = tdata->val;
						while (tc[0] != 0 && left > 0)
						{
							if (tc[0] == '\"' || tc[0] == '\\' 
								&& tdata->aty == sexp_dquote)
							{
								b[0] = '\\';
								++b;
								--left;
								if (left == 0) break;
							}

							b[0] = tc[0];
							++b; ++tc;
							--left;
							if (left == 0) break;
						}
					}
					else
					{
						if (left > 3)
						{
							b[0] = '#'; b[1] = 'b'; b[2] = '#';
							b += 3;
							left -= 3;

							if ((std::size_t)(sz = _snprintf(b, left, "%1u#", tdata->binlength)) >= left){
								left = 0;
								break;
							}

							b += sz;
							left -= sz;

							if (left < tdata->binlength){
								left = 0;
								break;
							}

							if (tdata->binlength > 0)
							{
								memcpy(b, tdata->bindata, tdata->binlength);
								left -= tdata->binlength;
								b += tdata->binlength;
							}

							b[0] = ' ';
							--left;
						}else{ 
							left = 0;
							break;
						}
					}

					if (tdata->aty == sexp_dquote && left > 0)
					{
						b[0] = '\"';
						b++;
						left--;
					}

					if (left == 0)
					{
						sexp_errno = sexp_err_buffer_full;
						break;
					}

					top->data = ((sexp_t *)top->data)->next;

					if (top->data)
					{
						b[0] = ' ';
						++b;
						--left;
						if (left == 0)
						{
							sexp_errno = sexp_err_buffer_full;
							break;
						}
					}
				}
				else if (tdata->ty == sexp_list)
				{
					++depth;
					b[0] = '(';
					++b;
					--left;
					if (left == 0)
					{
						sexp_errno = sexp_err_buffer_full;
						break;
					}
					stack->push(tdata->list);
				}
				else
				{
					sexp_errno = sexp_errr_badcontent;
					delete stack;
					sexp_t_deallocate(fakehead);
					return -1;
				}
			}

			while (depth != 0)
			{
				b[0] = ')';
				b++;
				left--;
				depth--;
				if (left == 0)
				{
					sexp_errno = sexp_err_buffer_full;
					break;
				}
			}

			int retval = 0;
			if (left != 0){
				b[0] = 0;
				retval = (int)(size - left);
			}else {
				--b;
				b[0] = 0;
				retval = -1;
			}

			delete stack;
			sexp_t_deallocate(fakehead);

			return retval;
		}

		sexp_t*  make_sexp_List(sexp_t * l);

		sexp_t*	make_sexp_binary_atom(char *data, std::size_t binlength);

		sexp_t*	make_sexp_atom(const char *buf, std::size_t bs, atom_t aty = sexp_basic);
	}
}