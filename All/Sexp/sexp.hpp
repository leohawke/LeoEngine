#pragma once
#include <cstdint>
#include <cstddef>
#include "faststack.hpp"
#include "..\exception.hpp"
#ifdef SMART_PTR
#include <memory>
#define SHARED_PTR(type) std::shared_ptr<type>
#define WEAK_PTR(type) std::weak_ptr<type>
#else
#include "..\declara.hpp"
#define SHARED_PTR(type) Macro_Cat(type,*)
#define WEAK_PTR(type) Macro_Cat(type,*)
#endif

namespace leo
{
	namespace sexp
	{
		enum sexp_errcode_t 
		{
			sexp_err_ok,
			sexp_err_memory,
			sexp_err_badform,sexp_err_badcontent,
			sexp_err_nullstring,
			sexp_err_io,sexp_err_io_empty,
			sexp_err_mem_limit,
			sexp_err_buffer_full,
			sexp_err_bad_param,sexp_err_bad_stack,
			sexp_err_unknown_state,sexp_err_incomplete,
			sexp_err_bad_constructor
		};

		class sexp_exception : public leo::logged_event
		{
		public:
			using error_code_type = sexp_errcode_t;
		private:
			error_code_type errcode;
		public:
			sexp_exception(error_code_type, const std::string& = "win32 exception", level_type = {}) _NOEXCEPT;
			Macro_DefGetter(const _NOEXCEPT, error_code_type, Error_Code, errcode);
			Macro_DefGetter(const _NOEXCEPT, std::string, Message, formatmessage(errcode));

			explicit Macro_DefCvt(const _NOEXCEPT, error_code_type, errcode);
			static std::string formatmessage(error_code_type) _NOEXCEPT;
		};

#define Raise_Sexp_Exception(err,...)\
		{\
		throw leo::sexp::sexp_exception(err,__VA_ARGS__);\
		}
#define Catch_Sexp_Exception \
	catch(leo::sexp::sexp_exception & e) \
		{\
			perror(e.what()) ;\
		}
		enum elt_t {sexp_value,sexp_list};

		enum atom_t {sexp_basic,sexp_squote,sexp_dquote,sexp_binary};

		struct elt
		{
			elt* next;

			elt_t ty;

			union
			{
				struct
				{
					char* val;//assert(ty == sexp_value)
					atom_t aty;
				};
				
				elt* list;//assert(ty == sexp_list)
			};

			std::size_t val_allocated;
			std::size_t val_used;
			//assert(val_used <= val_allocated)

			struct
			{
				char * bindata;//assert(aty == sexp_binary)
				std::size_t binlength;
			};
		};

		using sexp_t = elt;

		enum parsermode_t {parser_normal,parser_inline_binary,parser_evnets_only};

		struct parser_event_handlers
		{
			//std::funciton<void()> start
			void(*start_sexpr)();

			//std::function<void()> end
			void(*end_sexpr)();

			//std::function<void(const char*,std::size_t,atom_t)> chars;
			void(*chararcters)(const char* data, std::size_t len, atom_t aty);

			//std::function<void(const char*,std::size_t)> bina;
			void(*binary)(const char* data, std::size_t len);

		};

		using parser_event_handlers_t = parser_event_handlers;

		struct parse_data_t
		{
			sexp_t *fst = nullptr, *lst = nullptr;
		};

		struct pcont
		{
			faststack<parse_data_t> * stack = nullptr;

			sexp_t * last_sexp = nullptr;

			char * val = nullptr;

			std::size_t val_used = 0;

			std::size_t val_allocated = 0;

			char * vcur = 0;

			char * lastPos = nullptr;

			char * sbuffer = nullptr;

			unsigned int depth = 0;

			unsigned int qdepth = 0;

			unsigned int state = 0;

			unsigned int esc = 0;

			unsigned int squoted = 0;

			sexp_errcode_t error = sexp_err_ok;

			parsermode_t mode = parser_normal;

			std::size_t binexpected = 0;

			std::size_t binread = 0;

			char * bindata = nullptr;

			parser_event_handlers_t * event_handlers = nullptr;
		};

		using pcont_t = pcont;

		struct sexp_iowrap
		{
			pcont * cc;

			int fd;

			char buf[BUFSIZ];

			std::size_t cnt;
		};

		using sexp_iowrap_t = sexp_iowrap;

		extern sexp_errcode_t sexp_errno;

		void set_parser_buffer_params(std::size_t ss, std::size_t gs);

		sexp_t * sexp_t_allocate(void);

		void sexp_t_deallocate(sexp_t * s);

		void sexp_cleanup(void);

		int print_sexp(char * loc, std::size_t size, const sexp_t *e);

		int print_sexp_cstr(cstring ** s, const sexp_t *e, size_t ss);

		sexp_t * make_sexp_List(sexp_t * l);

		sexp_t * make_sexp_binary_atom(char *data, std::size_t binlength);

		sexp_t * make_sexp_atom(const char *buf, std::size_t bs, atom_t aty = sexp_basic);

		void destroy_continuation(pcont_t * pc);

		sexp_iowrap_t *init_iowrap(int fd);

		void destroy_iowrap(sexp_iowrap_t *iow);

		sexp_t *read_one_sexp(sexp_iowrap_t *iow);

		sexp_t *parse_sexp(char *s, size_t len);

		sexp_t *iparse_sexp(char *s, size_t len, pcont_t *cc);

		pcont_t *cparse_sexp(char *s, size_t len, pcont_t *pc);

		void destroy_sexp(sexp_t *s);

		void reset_sexp_errno();

		void print_pcont(pcont_t * pc, char * buf, size_t buflen);

		namespace ops
		{
			inline sexp_t * hd_sexp(sexp_t* s)
			{
				return s->list;
			}

			inline sexp_t * tl_sexp(sexp_t* s)
			{
				return s->list->next;
			}

			inline sexp_t * next_sexp(sexp_t* s)
			{
				return s->next;
			}

			inline void reset_pcont(pcont* c)
			{
				c->lastPos = nullptr;
			}

			sexp_t *find_sexp(const char *name, sexp_t *start);

			sexp_t *bfs_find_sexp(const char *name, sexp_t *start);

			int sexp_list_length(const sexp_t *sx);

			sexp_t *copy_sexp(const sexp_t *sx);
		}
	}
}