#include "sexp.hpp"
#include <cstring>
namespace leo
{
	namespace sexp
	{
		namespace ops
		{
			sexp_t * find_sexp(const char *name, sexp_t * start)
			{
				sexp_t *temp;

				if (start == nullptr)
					return nullptr;

				if (start->ty == sexp_list)
				{
					temp = find_sexp(name, start->list);
					if (temp == nullptr)
						return find_sexp(name, start->next);
					else
						return temp;
				}
				else
				{
					if (start->val != nullptr && strcmp(start->val, name) == 0)
						return start;
					else
						return find_sexp(name, start->next);
				}

				return nullptr;			/* shouldn't get here */
			}

			/**
			* Breadth first search - look at ->next before ->list when seeing list
			* elements of an expression.
			*/
			sexp_t *bfs_find_sexp(const char *str, sexp_t *sx)
			{
				sexp_t *t = sx;
				sexp_t *rt;

				if (sx == nullptr) return nullptr;

				while (t != nullptr) {
					if (t->ty == sexp_value) {
						if (t->val != nullptr) {
							if (strcmp(t->val, str) == 0) {
								return t;
							}
						}
					}

					t = t->next;
				}

				t = sx;
				while (t != nullptr) {
					if (t->ty == sexp_list) {
						rt = bfs_find_sexp(str, t->list);
						if (rt != nullptr) return rt;
					}

					t = t->next;
				}

				return nullptr;
			}

			/**
			* Give the length of a s-expression list.
			*/
			int sexp_list_length(const sexp_t *sx)
			{
				int len = 0;
				const sexp_t *t;

				if (sx == nullptr) return 0;

				if (sx->ty == sexp_value) return 1;

				t = sx->list;

				while (t != nullptr) {
					len++;
					t = t->next;
				}
				return len;
			}

			/**
			* Copy an s-expression.
			*/
			sexp_t *copy_sexp(const sexp_t *s)
			{
				sexp_t *s_new;

				if (s == nullptr) return nullptr;

				s_new = sexp_t_allocate();
				if (s_new == nullptr) {
					sexp_errno = sexp_err_memory;
					return nullptr;
				}

				/* initialize fields to null and zero, and fill in only those necessary. */
				s_new->val_allocated = s_new->val_used = 0;
				s_new->val = nullptr;
				s_new->list = s_new->next = nullptr;
				s_new->bindata = nullptr;
				s_new->binlength = 0;

				/* now start copying in data and setting appropriate fields. */
				s_new->ty = s->ty;

				/* values */
				if (s_new->ty == sexp_value) {
					s_new->aty = s->aty;

					/* binary */
					if (s_new->aty == sexp_binary) {
						if (s->bindata == nullptr && s->binlength > 0) {
							sexp_errno = sexp_err_badcontent;
							sexp_t_deallocate(s_new);
							return nullptr;
						}

						s_new->binlength = s->binlength;

						if (s->bindata == nullptr) {
							s_new->bindata = nullptr;
						}
						else {
							/** allocate space **/
							s_new->bindata = new char[s->binlength];
						}

						if (s_new->bindata == nullptr) {
							sexp_errno = sexp_err_memory;
							sexp_t_deallocate(s_new);
							return nullptr;
						}

						memcpy(s_new->bindata, s->bindata, s->binlength*sizeof(char));

						/* non-binary */
					}
					else {
						if (s->val == nullptr && (s->val_used > 0 || s->val_allocated > 0)) {
							sexp_errno = sexp_err_badcontent;
							sexp_t_deallocate(s_new);
							return nullptr;
						}

						s_new->val_used = s->val_used;
						s_new->val_allocated = s->val_allocated;

						if (s->val == nullptr) {
							s_new->val = nullptr;
						}
						else {
							/** allocate space **/
							s_new->val = new char[1 * s->val_allocated];

							if (s_new->val == nullptr) {
								sexp_errno = sexp_err_memory;
								sexp_t_deallocate(s_new);
								return nullptr;
							}

							memcpy(s_new->val, s->val, sizeof(char)*s->val_used);
						}
					}
				}
				else {
					s_new->list = copy_sexp(s->list);
				}

				s_new->next = copy_sexp(s->next);

				return s_new;
			}
		}
	}
}