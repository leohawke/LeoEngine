#include "faststack.hpp"
#include "sexp.hpp"

namespace leo
{
	namespace sexp
	{
		static std::size_t sexp_val_start_size = 256;
		static std::size_t sexp_val_grow_size = 64;

		void set_parser_buffer_params(std::size_t ss, std::size_t gs)
		{
			try
			{
				if (ss > 0)
					sexp_val_start_size = ss;
				else
					Raise_Sexp_Exception(sexp_err_bad_param, "参数错误: sexp_val 开始分配大小应大于0");

				if (ss > 0)
					sexp_val_grow_size = gs;
				else
					Raise_Sexp_Exception(sexp_err_bad_param, "参数错误: sexp_val 增长大小应大于0");
			}
			Catch_Sexp_Exception
		}
		
		faststack<parse_data_t> pd_cache;

		faststack_t sexp_t_cache {};

		sexp_t * sexp_t_allocate(void)
		{
			sexp_t * sx = nullptr;
			try
			{
				if (sexp_t_cache.empty())
				{
					try{
						sx = new sexp_t;
					}
					catch (...)
					{
						Raise_Sexp_Exception(sexp_err_memory, "分配失败");
					}
					sx->next = sx->list = nullptr;
				}
				else
				{
					sx = sexp_t_cache.pop()->data;
				}
			}
			Catch_Sexp_Exception;
			return sx;
		}

		void sexp_t_deallcate(sexp_t *s)
		{
			if (!s) return;
			s->list = s->next = nullptr;

			if (s->ty == sexp_value && s->val)
				delete[] s->val;

			s->val = nullptr;
			
			sexp_t_cache.push(s);
		}

		void sexp_cleanup(void)
		{

			auto l = pd_cache.top;
			while (l)
			{
				delete l->data;
				l = l->below;
			}
			empty(pd_cache);

			auto ls = sexp_t_cache.top;
			while (ls)
			{
				delete ls->data;
				ls = ls->below;
			}

			empty(sexp_t_cache);
		}

		parse_data_t* pd_allocate(void)
		{
			parse_data_t * px = nullptr;
			try
			{
				if (pd_cache.empty())
				{
					try{
						px = new parse_data_t;
					}
					catch (...)
					{
						Raise_Sexp_Exception(sexp_err_memory, "分配失败");
					}
				}
				else
				{
					px = pd_cache.pop()->data;
				}
			}
			Catch_Sexp_Exception;
			return px;
		}

		void pd_deallocate(parse_data_t *p)
		{
			if (!p) return;
			pd_cache.push(p);
		}

		void print_pcont(pcont_t * pc, char * buf, size_t buflen) {
			char *cur = buf;
			int loc = 0;
			int n;
			parse_data_t *pdata;
			sexp_t *sx;

			/* return if either the buffer or continuation are null */
			if (!buf) return;
			if (!pc) return;

			/* if continuation has no stack, return */
			if (!pc->stack) return;

			/* start at the bottom of the stack */
			auto lvl = pc->stack->bottom;

			/* go until we either run out of buffer space or we hit the
			top of the stack */
			while (loc < buflen - 1 && lvl) {
				/* get the data at the current stack level */
				pdata = lvl->data;

				/* if this is null, we're at a level with nothing added yet */
				if (!pdata) break;

				/* get first fully parsed sexpr for this level. this could be
				any sub-expression, like an atom or a full s-expression */
				sx = pdata->fst;

				/* spin through all of the s-expressions at this level */
				while (sx) {

					/* if we have a list that has no contents, just add the open
					paren.  this means we haven't finished this expression and the
					stack contains it's partial contents.  Just print the open paren
					and break out so we can pop up the stack. */
					if (sx->ty == sexp_list && !sx->list) {
						cur[0] = '(';
						cur++;
						loc++;
						break;
					}
					else {
						/* print the fully parsed sub-expression */
						n = print_sexp(cur, buflen - loc, sx);

						/* add a space between this and the next expression.  note that
						this may induce spaces that were not part of the original
						expression.  */
						cur[n] = ' ';

						/* increment n to compensate for the space we added */
						n++;

						/* push the pointer into the output buffer forward by n */
						cur += n;

						/* increment counter for location in buffer by n */
						loc += n;
					}

					/* go to next s-expr */
					sx = sx->next;
				}

				/* go up to next level in stack */
				lvl = lvl->above;
			}

			/* at this point, all that may remain is a partially parsed string
			that hasn't been turned into a sexpr yet.  attach it to the
			output string. */
			if (pc->val_used < (buflen - loc) - 1) {
				strncpy(cur, pc->val, pc->val_used);
				cur += pc->val_used;
			}
			else {
				/* don't bother if we're so close to the end of the buffer that
				we can't attach our null terminator. */
				if (buflen - loc > 2) {
					strncpy(cur, pc->val, (buflen - loc) - 2);
					cur += (buflen - loc) - 2;
				}
			}

			/* add null terminator */
			cur[0] = '\0';
		}

		void destroy_continuation(pcont_t * pc)
		{

				if (!pc) return; /* return if null passed in */

				if (pc->stack) {
					auto lvl = pc->stack->top;

					/*
					* note that destroy_stack() does not free the data hanging off of the
					* stack.  we have to walk down the stack and do that here.
					*/

					while (lvl) {
						auto lvl_data = lvl->data;

						/**
						* Seems to have fixed bug with destroying partially parsed
						* expression continuations with the short three lines below.
						*/
						if (lvl_data) {
							lvl_data->lst = nullptr;
							destroy_sexp(lvl_data->fst);
							lvl_data->fst = nullptr;

							pd_deallocate(lvl_data);
							lvl->data = lvl_data = nullptr;
						}

						lvl = lvl->below;
					}

					/*
					* stack has no data on it anymore, so we can free it.
					*/
					delete pc->stack;
					pc->stack = nullptr;
				}

				/*
				* free up data used for INLINE_BINARY mode
				*/
				if (pc->bindata) {
					delete [] pc->bindata;
					pc->bindata = nullptr;
				}

				if (pc->val) {
					delete [] pc->val;
					pc->val = nullptr;
				}

				delete pc;
		}

		sexp_t * parse_sexp(char *s, size_t len)
		{
				pcont_t *pc = nullptr;
				sexp_t *sx = nullptr;

				if (len < 1 || !s ) return nullptr; /* empty string - return */

				pc = cparse_sexp(s, len, pc);
				if (pc == nullptr)  return nullptr; /* assume that cparse_sexp set sexp_errno */
				sx = pc->last_sexp;

				destroy_continuation(pc);

				return sx;
		}

		pcont_t *init_continuation(char *str)
		{
			pcont_t* cc = nullptr;
			try
			{
				try
				{
					cc = new pcont_t;
					/* allocate atom buffer */
					cc->val = new char[sexp_val_start_size];

					cc->stack = new faststack<parse_data_t>;
				}
				catch (...)
				{
					delete cc->stack;
					delete [] cc->val;
					delete cc;
					Raise_Sexp_Exception(sexp_err_memory, "分配失败");
				}

				/* by default we assume a normal parser */
				//cc->mode = PARSER_NORMAL;

				cc->val_allocated = sexp_val_start_size;
				//cc->val_used = 0;

				//cc->bindata = NULL;
				//cc->binread = cc->binexpected = 0;

				/* allocate stack */
				//cc->esc = 0;

				cc->sbuffer = str;
				//cc->lastPos = NULL;
				cc->state = 1;
				cc->vcur = cc->val;
				//cc->depth = 0;
				//cc->qdepth = 0;
				//cc->squoted = 0;
				//cc->event_handlers = NULL;
			}
			Catch_Sexp_Exception;

			return cc;
		}
	}
}