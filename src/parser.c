/** 

SFSEXP: Small, Fast S-Expression Library version 1.0
Written by Matthew Sottile (matt@lanl.gov)

Copyright (2004). The Regents of the University of California. This material
was produced under U.S. Government contract W-7405-ENG-36 for Los Alamos
National Laboratory, which is operated by the University of California for
the U.S. Department of Energy. The U.S. Government has rights to use,
reproduce, and distribute this software. NEITHER THE GOVERNMENT NOR THE
UNIVERSITY MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
FOR THE USE OF THIS SOFTWARE. If software is modified to produce derivative
works, such modified software should be clearly marked, so as not to confuse
it with the version available from LANL.

Additionally, this program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version. Accordingly, this program is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
the GNU General Public License for more details.

LA-CC-04-094

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sexp.h"
#include "faststack.h"

/*
 * constants related to atom buffer sizes and growth.
 */
static int sexp_val_start_size = 256;
static int sexp_val_grow_size  = 64;

/*
 * Function for tuning growth parameters.
 */
void set_parser_buffer_params(int ss, int gs) {
  if (ss > 0)
    sexp_val_start_size = ss;
  else 
    fprintf(stderr,"%s: Cannot set buffer start size to value<1.\n",__FILE__);

  if (gs > 0)
    sexp_val_grow_size = gs;
  else
    fprintf(stderr,"%s: Cannot set buffer grow size to value<1.\n",__FILE__);
}

/**
 * this structure is pushed onto the stack so we can keep track of the
 * first and last elements in a list.
 * !!!!DON'T USE THESE OUTSIDE THIS FILE!!!!
 */
typedef struct parse_stack_data
{
  sexp_t *fst, *lst;
}
parse_data_t;

/**
 * parse_data_t stack - similar malloc prevention to sexp_t_cache.
 */
faststack_t *pd_cache;

/** 
 * The global <I>sexp_t_cache</I> is a faststack implementing a cache of
 * pre-alloced s-expression element entities.  Odds are a user should never 
 * touch this.  If you do, you're on your own.  This is used internally by 
 * the parser and related code to store unused but allocated sexp_t elements.
 * This should be left alone and manipulated only by the sexp_t_allocate and
 * sexp_t_deallocate functions.  Touching the stack is bad.  
 */ 
faststack_t *sexp_t_cache;

/**
 * sexp_t allocation
 */
#ifdef _NO_MEMORY_MANAGEMENT_
sexp_t *
sexp_t_allocate(void) {
  sexp_t *sx = (sexp_t *) sexp_calloc(1, sizeof(sexp_t));
  assert(sx != NULL);

  return(sx);
}
#else
sexp_t *
sexp_t_allocate(void) {
  sexp_t *sx;
  stack_lvl_t *l;

  if (sexp_t_cache == NULL) {
    sexp_t_cache = make_stack();

    sx = (sexp_t *)sexp_malloc(sizeof(sexp_t));
    assert(sx != NULL);

    sx->next = sx->list = NULL;
  } else {
    if (empty_stack(sexp_t_cache)) {
      sx = (sexp_t *)sexp_malloc(sizeof(sexp_t));
      assert(sx != NULL);

      sx->next = sx->list = NULL;
    } else {
      l = pop(sexp_t_cache);
      sx = (sexp_t *)l->data;
    }
  }

  return sx;
}
#endif

/**
 * sexp_t de-allocation
 */
#ifdef _NO_MEMORY_MANAGEMENT_
void
sexp_t_deallocate(sexp_t *s) {
  if (s->ty == SEXP_VALUE && s->val != NULL) {
    sexp_free(s->val,s->val_allocated);
  }

  sexp_free(s,sizeof(sexp_t));
}
#else
void
sexp_t_deallocate(sexp_t *s) {
  if (sexp_t_cache == NULL) sexp_t_cache = make_stack();

  if (s == NULL) return;

  s->list = s->next = NULL;

  if (s->ty == SEXP_VALUE && s->val != NULL) {
    sexp_free(s->val,s->val_allocated);
  }

  s->val = NULL;

  sexp_t_cache = push(sexp_t_cache, s);
}
#endif

/**
 * cleanup the sexp library.  Note this is implemented HERE since we need
 * to know about pd_cache, which is local to this file.
 */
#ifdef _NO_MEMORY_MANAGEMENT_
void sexp_cleanup(void) {
}
#else
void sexp_cleanup(void) {
  stack_lvl_t *l;

  if (pd_cache != NULL) {
    l = pd_cache->top;
    while (l != NULL) {
      sexp_free(l->data,sizeof(sexp_t));
      l = l->below;
    }
    destroy_stack(pd_cache);
    pd_cache = NULL;
  }

  if (sexp_t_cache != NULL) {
    l = sexp_t_cache->top;
    while (l != NULL) {
      sexp_free(l->data,sizeof(sexp_t));
      l = l->below;
    }
    destroy_stack(sexp_t_cache);
    sexp_t_cache = NULL;
  }
}
#endif

/**
 * allocation
 */
parse_data_t *
pd_allocate(void) {
  parse_data_t *p;
  stack_lvl_t *l;

  if (pd_cache == NULL) {
    pd_cache = make_stack();
    p = (parse_data_t *)sexp_malloc(sizeof(parse_data_t));
    assert(p!=NULL);
  } else {
    if (empty_stack(pd_cache)) {
      p = (parse_data_t *)sexp_malloc(sizeof(parse_data_t));
      assert(p!=NULL);
    } else {
      l = pop(pd_cache);
      p = (parse_data_t *)l->data;
    }
  }

  return p;
}

/**
 * de-allocation
 */
void
pd_deallocate(parse_data_t *p) {
  if (pd_cache == NULL) pd_cache = make_stack();

  pd_cache = push(pd_cache, p);
}

/**
 * Destroy a continuation by freeing all of its fields that it is responsible
 * for managing, and then free the continuation itself.  This includes internal
 * buffers, stacks, etc..
 */
void
destroy_continuation (pcont_t * pc)
{
  stack_lvl_t *lvl;
  parse_data_t *lvl_data;

  if (pc == NULL) return; /* return if null passed in */

  if (pc->stack != NULL) {
    lvl = pc->stack->top;
    
    /*
     * note that destroy_stack() does not free the data hanging off of the
     * stack.  we have to walk down the stack and do that here. 
     */
    
    while (lvl != NULL) {
      lvl_data = (parse_data_t *)lvl->data;

      /**
       * Seems to have fixed bug with destroying partially parsed
       * expression continuations with the short three lines below.
       */
      if (lvl_data != NULL) {
        lvl_data->lst = NULL;
        destroy_sexp(lvl_data->fst);
        lvl_data->fst = NULL;

        /* free(lvl_data); */
        pd_deallocate(lvl_data);
        lvl->data = lvl_data = NULL;
      }

      lvl = lvl->below;
    }
    
    /*
     * stack has no data on it anymore, so we can free it.
     */
    destroy_stack(pc->stack);
    pc->stack = NULL;
  }

  /*
   * free up data used for INLINE_BINARY mode
   */
  if (pc->bindata != NULL) {
    sexp_free(pc->bindata,pc->binexpected);
    pc->bindata = NULL;
  }

  /*
   * free up other allocated data
   */
  sexp_free (pc->val,pc->val_allocated);
  sexp_free (pc,sizeof(pcont_t));
}

/* 
 * wrapper around cparse_sexp.  assumes s contains a single, complete,
 * null terminated s-expression.  partial sexps or strings containing more
 * than one will act up.
 */
sexp_t *
parse_sexp (char *s, int len)
{
  pcont_t *pc = NULL;
  sexp_t *sx = NULL;

  if (len < 1 || s == NULL) return NULL; /* empty string - return */  

  pc = cparse_sexp (s, len, pc);
  sx = pc->last_sexp;

  destroy_continuation(pc);

  return sx;
}

pcont_t *
init_continuation(char *str) 
{
  pcont_t *cc;
  /* new continuation... */
  cc = (pcont_t *)sexp_malloc(sizeof(pcont_t));
  assert(cc != NULL);
  
  /* allocate atom buffer */
  cc->val = (char *)sexp_malloc(sizeof(char)*sexp_val_start_size);
  assert(cc->val != NULL);
  
  /* by default we assume a normal parser */
  cc->mode = PARSER_NORMAL;

  cc->val_allocated = sexp_val_start_size;
  cc->val_used = 0;

  cc->bindata = NULL;
  cc->binread = cc->binexpected = 0;

  /* allocate stack */
  cc->esc = 0;
  cc->stack = make_stack();
  cc->sbuffer = str;
  cc->lastPos = NULL;
  cc->state = 1;
  cc->vcur = cc->val;
  cc->depth = 0;
  cc->qdepth = 0;
  cc->squoted = 0;

  return cc;
}

/**
 * Iterative parser.  Wrapper around parse_sexp that is slightly more
 * intelligent and allows users to iteratively "pop" the expressions
 * out of a string that contains a bunch of expressions.
 * Useful if you have a string like "(foo bar)(goo har)(moo mar)" and
 * want to get "(foo bar)", "(goo har)", and "(moo mar)" individually on
 * repeated calls.
 */
sexp_t *
iparse_sexp (char *s, int len, pcont_t *cc) {
  pcont_t *pc;
  sexp_t *sx = NULL;
  
  /* 
   * sanity check.
   */
  if (cc == NULL) {
    fprintf(stderr,"iparse_sexp called with null continuation!\n");
    return NULL;
  }
  
  /* call the parser */
  pc = cparse_sexp(s,len,cc);
  
  if (cc->last_sexp != NULL) {
    sx = cc->last_sexp;
    cc->last_sexp = NULL;
  }
  
  return sx;
}

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

/**
 * Continuation based parser - the guts of the package.
 */
pcont_t *
cparse_sexp (char *str, int len, pcont_t *lc)
{
  char *t, *s;
  register unsigned int binexpected = 0;
  register unsigned int binread = 0; 
  register unsigned int mode = PARSER_NORMAL;
  register unsigned int val_allocated = 0;
  register unsigned int squoted = 0;
  register unsigned int val_used = 0;
  register unsigned int state = 1;
  register unsigned int depth = 0;
  register unsigned int qdepth = 0;
  register unsigned int elts = 0;
  register unsigned int esc = 0;
  pcont_t *cc;
  char *val, *vcur, *bindata = NULL;
  sexp_t *sx = NULL;
  faststack_t *stack;
  parse_data_t *data;
  stack_lvl_t *lvl;
#ifdef _DEBUG_
  unsigned long fsm_iterations = 0;
  unsigned long maxdepth = 0;
#endif /* _DEBUG_ */
  char *bufEnd;
  int keepgoing = 1;

  /* make sure non-null string */
  if (str == NULL) {
    fprintf(stderr,"cparse_sexp: called with null string.\n");
    return lc;
  }
  
  /* first, if we have a non null continuation passed in, restore state. */
  if (lc != NULL) {
    cc = lc;
    binexpected = cc->binexpected;
    binread = cc->binread;
    bindata = cc->bindata;
    val_used = cc->val_used;
    val_allocated = cc->val_allocated;
    squoted = cc->squoted;
    val = cc->val;
    vcur = cc->vcur;
    state = cc->state;
    depth = cc->depth;
    qdepth = cc->qdepth;
    stack = cc->stack;
    esc = cc->esc;
    mode = cc->mode;
    s = str;
    if (cc->lastPos != NULL)
      t = cc->lastPos;
    else {
      t = s;
      cc->sbuffer = str;
    }
  } else {
    /* new continuation... */
    cc = (pcont_t *)sexp_malloc(sizeof(pcont_t));
    assert(cc != NULL);
    
    cc->mode = mode;

    /* allocate atom buffer */
    cc->val = val = (char *)sexp_malloc(sizeof(char)*sexp_val_start_size);
    assert(val != NULL);
    
    cc->val_used = val_used = 0;
    cc->val_allocated = val_allocated = sexp_val_start_size;

    vcur = val;
    
    /* allocate stack */
    cc->stack = stack = make_stack();
    cc->bindata = NULL;
    cc->binread = cc->binexpected = 0;

    /* t is temp pointer into s for current position */
    s = str;
    t = s;
    cc->sbuffer = str;
  }
  
  bufEnd = cc->sbuffer+len;

  /* guard for loop - see end of loop for info.  Put it out here in the
     event that we're restoring state from a continuation and need to
     check before we start up. */
  if (state != 15 && t[0] == '\0') keepgoing = 0;

  /*==================*/
  /* main parser loop */
  /*==================*/
  while (keepgoing == 1 && t != bufEnd)
    {
#ifdef _DEBUG_
      printf("PARSER: STATE=%d CURCHAR=%c (0x%x)\n",state,t[0],t);
      printf("MODE: %d\n",mode);
      printf("        VAL_ALLOCATED=%d VAL_USED=%d\n",val_allocated,val_used);
      fsm_iterations++;
#endif

      /* based on the current state in the FSM, do something */
      switch (state)
	{
	case 1:
	  switch (t[0])
	    {
	      /* space,tab,CR,LF considered white space */
	    case '\n':
	    case ' ':
	    case '\t':
	    case '\r':             
	      t++;
	      break;
              /* semicolon starts a comment that extends until a \n is
                 encountered. */
            case ';':
              t++;
              state = 11;
              break;
	      /* enter state 2 for open paren */
	    case '(':
	      state = 2;
	      t++;
	      break;
	      /* enter state 3 for close paran */
	    case ')':
	      state = 3;
	      break;
	      /* begin quoted string - enter state 5 */
	    case '\"':
	      state = 5;
	      /* set cur pointer to beginning of val buffer */
	      vcur = val;
	      t++;
	      break;
	      /* single quote - enter state 7 */
	    case '\'':
	      state = 7;
	      t++;
	      break;
	      /* other characters are assumed to be atom parts */
	    default:
	      /* set cur pointer to beginning of val buffer */
	      vcur = val;

              /** NOTE: the following code originally required a transition
                  to state 4 before processing the first atom character --
                  this required two iterations for the first character
                  of each atom.  merging this into here allows us to process
                  what we already know to be a valid atom character before
                  entering state 4. **/
	      vcur[0] = t[0];
	      if (t[0] == '\\') esc = 1;
	      else esc = 0;
              val_used++;

              if (val_used == val_allocated) {
                val = (char *)sexp_realloc(val,
					   val_allocated+sexp_val_grow_size,
					   val_allocated);
                assert(val != NULL);
                vcur = val + val_used;
                val_allocated += sexp_val_grow_size;
              } else vcur++;

              /* if the atom starts with # and we're in inline
                 binary mode, we need to go to state 12 to start
                 checking for the #b# prefix.  otherwise,
                 if it's not a # or we're just in normal mode,
                 proceed to state 4 as usual. */
              if (t[0] == '#' && mode == PARSER_INLINE_BINARY) {
                state = 12;
              } else {
                state = 4;
              }

              t++;
	      break;
	    }
	  break;
	case 2:
	  /* open paren */
	  depth++;

          sx = sexp_t_allocate();
	  assert(sx!=NULL);
	  elts++;
	  sx->ty = SEXP_LIST;
	  sx->next = NULL;
	  sx->list = NULL;
	  
	  if (stack->height < 1)
	    {
              data = pd_allocate();
	      assert(data!=NULL);
	      data->fst = data->lst = sx;
	      push (stack, data);
	    }
	  else
	    {
	      data = (parse_data_t *) top_data (stack);
	      if (data->lst != NULL)
		data->lst->next = sx;
	      else
		data->fst = sx;
	      data->lst = sx;
	    }
	  
          data = pd_allocate();
	  assert(data!=NULL);
	  data->fst = data->lst = NULL;
	  push (stack, data);
	  
	  state = 1;
	  break;
	case 3:
	  /** close paren **/
#ifdef _DEBUG_    
          if (depth > maxdepth) maxdepth = depth;
#endif /* _DEBUG_ */

          /* check for close parens that were never opened. */
          if (depth == 0) {
            fprintf(stderr,"Badly formed s-expression.  Parser exiting.\n");
	    cc->val = val;
            cc->mode = mode;
            cc->val_used = val_used;
            cc->val_allocated = val_allocated;
	    cc->vcur = vcur;
	    cc->lastPos = t;
	    cc->depth = depth;
	    cc->qdepth = qdepth;
	    cc->state = 1;
	    cc->stack = stack;
	    cc->esc = 0;
	    cc->last_sexp = NULL;
            cc->error = 1;

#ifdef _DEBUG_
            fprintf(stderr,"State Machine Iterations: %d\nMaxdepth: %d\n",
                    fsm_iterations,maxdepth);
#endif /* debug */

	    return cc;
          }

	  t++;
	  depth--;

	  lvl = pop (stack);
	  data = (parse_data_t *) lvl->data;
	  sx = data->fst;
	  /* free (data); */
          pd_deallocate(data);
          lvl->data = NULL;

	  if (stack->top != NULL)
	    {
	      data = (parse_data_t *) top_data (stack);
	      data->lst->list = sx;
	    }
	  else
	    {
	      fprintf (stderr, "Hmmm. Stack->top is null.\n");
	    }

	  state = 1;

	  /** if depth = 0 then we finished a sexpr, and we return **/
	  if (depth == 0) {
            cc->error = 0;
            cc->mode = mode;
	    cc->val = val;
            cc->val_allocated = val_allocated;
            cc->val_used = val_used;
	    cc->vcur = vcur;
	    cc->lastPos = t;
	    cc->depth = depth;
	    cc->qdepth = qdepth;
	    cc->state = 1;
	    cc->stack = stack;
	    cc->esc = 0;
	    while (stack->top != NULL)
	      {
		lvl = pop (stack);
		data = (parse_data_t *) lvl->data;
		sx = data->fst;
		/* free (data); */
                pd_deallocate(data);
                lvl->data = NULL;
	      }
	    cc->last_sexp = sx;

#ifdef _DEBUG_
            fprintf(stderr,"State Machine Iterations: %d\nMaxdepth: %d\n",
                    fsm_iterations,maxdepth);
#endif /* debug */

	    return cc;
	  }
	  break;
	case 4: /** parsing atom **/
	  if (esc == 1 && (t[0] == '\"' || t[0] == '(' ||
			   t[0] == ')' || t[0] == '\'' ||
			   t[0] == '\\')) {
	    vcur--; /* back up to overwrite the \ */
	    vcur[0] = t[0];
	    vcur++;
	    t++;
	    esc = 0;
	    break;
	  }

	  /* look at an ascii table - these ranges are the non-whitespace, non
	     paren and quote characters that are legal in atoms */
	  if (!((t[0] >= '*' && t[0] <= '~') ||
		((unsigned char)(t[0]) > 127) || 
		(t[0] == '!') ||
		(t[0] >= '#' && t[0] <= '&')))
	    {
	      vcur[0] = '\0';
              val_used++;

              sx = sexp_t_allocate();
	      assert(sx!=NULL);
	      elts++;
	      sx->ty = SEXP_VALUE;
              sx->val = val;
              sx->val_allocated = val_allocated;
              sx->val_used = val_used;
	      sx->next = NULL;
              if (squoted != 0)
                sx->aty = SEXP_SQUOTE;
              else
                sx->aty = SEXP_BASIC;

              val = (char *)sexp_malloc(sizeof(char)*sexp_val_start_size);
              assert(val != NULL);
              val_allocated = sexp_val_start_size;
              val_used = 0;
              vcur = val;
	      
	      if (!empty_stack (stack))
		{
		  data = (parse_data_t *) top_data (stack);
		  if (data->fst == NULL)
		    {
		      data->fst = data->lst = sx;
		    }
		  else
		    {
		      data->lst->next = sx;
		      data->lst = sx;
		    }
		}
	      else
		{
                  /* looks like this expression was just a basic atom - so
                     return it. */
                  cc->mode = mode;
                  cc->error = 0;
                  cc->val = val;
                  cc->val_used = val_used;
                  cc->val_allocated = val_allocated;
                  cc->vcur = vcur;
                  cc->squoted = 0;
                  cc->lastPos = t;
                  cc->depth = depth;
                  cc->qdepth = qdepth;
                  cc->state = 1;
                  cc->stack = stack;
                  cc->esc = 0;
                  cc->last_sexp = sx;
                  assert(sx != NULL);

#ifdef _DEBUG_
                  fprintf(stderr,"STATE 4: CURCHAR=%c\n",vcur);
#endif /* _DEBUG_ */

                  return cc;
		}

	      switch (t[0]) {
              case ' ':
              case '\t':
              case '\n':
              case '\r':
                /** NOTE: we know whitespace following atom, so spin ahead
                    one and let state 1 do what it needs to for the next
                    character. **/
                state = 1;
                t++;
                squoted = 0;
                break;
              case ')':
                squoted = 0;
                state = 3;
                break;
              default:
                squoted = 0;
                state = 1;
              }
	    }
	  else
	    {
	      vcur[0] = t[0];
	      if (t[0] == '\\') esc = 1;
	      else esc = 0;
              val_used++;

              if (val_used == val_allocated) {
                val = (char *)sexp_realloc(val,
					   val_allocated+sexp_val_grow_size,
					   val_allocated);
                assert(val != NULL);
                vcur = val + val_used;
                val_allocated += sexp_val_grow_size;
              } else vcur++;

	      t++;
	    }
	  break;
	case 5:
	  if (esc == 1 && (t[0] == '\"' ||
			   t[0] == '\'' ||
			   t[0] == '(' ||
			   t[0] == ')' ||
			   t[0] == '\\')) {
	    vcur--;
	    vcur[0] = t[0];
	    vcur++;
            /** NO NEED TO UPDATE VAL COUNTS **/
	    t++;
	    esc = 0;
	  }

	  if (t[0] == '\"')
	    {
	      state = 6;

              if (squoted == 1) {
                vcur[0] = '\"';
                val_used++;
                
                if (val_used == val_allocated) {
                  val = (char *)sexp_realloc(val,
					     val_allocated+sexp_val_grow_size,
					     val_allocated);
                  assert(val != NULL);
                  vcur = val + val_used;
                  val_allocated += sexp_val_grow_size;
                } else vcur++;
              }

              vcur[0] = '\0';

              val_used++;
              sx = sexp_t_allocate();
	      assert(sx!=NULL);
	      elts++;
	      sx->ty = SEXP_VALUE;
              sx->val = val;
              sx->val_used = val_used;
              sx->val_allocated = val_allocated;
	      sx->next = NULL;

              if (squoted == 1) {
                sx->aty = SEXP_SQUOTE;
                squoted = 0;
              } else
                sx->aty = SEXP_DQUOTE;

              val = (char *)sexp_malloc(sizeof(char)*sexp_val_start_size);
              assert(val != NULL);
              val_allocated = sexp_val_start_size;
              val_used = 0;
              vcur = val;
	      
	      if (!empty_stack (stack))
		{
		  data = (parse_data_t *) top_data (stack);
		  if (data->fst == NULL)
		    {
		      data->fst = data->lst = sx;
		    }
		  else
		    {
		      data->lst->next = sx;
		      data->lst = sx;
		    }
		}
	      else
		{
                  /* looks like this expression was just a basic double
                     quoted atom - so return it. */
                  t++; /* spin past the quote */

                  cc->mode = mode;
                  cc->squoted = 0;
                  cc->error = 0;
                  cc->val = val;
                  cc->val_used = val_used;
                  cc->val_allocated = val_allocated;
                  cc->vcur = vcur;
                  cc->lastPos = t++;
                  cc->depth = depth;
                  cc->qdepth = qdepth;
                  cc->state = 1;
                  cc->stack = stack;
                  cc->esc = 0;
                  cc->last_sexp = sx;
                  assert(sx != NULL);

                  return cc;
		}
	    }
	  else
	    {
	      vcur[0] = t[0];
              val_used++;

              if (val_used == val_allocated) {
                val = (char *)sexp_realloc(val,
					   val_allocated+sexp_val_grow_size,
					   val_allocated);
                assert(val != NULL);
                vcur = val + val_used;
                val_allocated += sexp_val_grow_size;
              } else vcur++;

	      if (t[0] == '\\') { 
                esc = 1;  
	      } else 
                esc = 0;
	    }

	  t++;
	  break;
	case 6:
	  vcur = val;
	  state = 1;
	  break;
	case 7:
	  if (t[0] == '\"')
	    {
	      state = 5;
	      vcur = val;
              t++;

              vcur[0] = '\"';
              val_used++;
              
              if (val_used == val_allocated) {
                val = (char *)sexp_realloc(val,
					   val_allocated+sexp_val_grow_size,
					   val_allocated);
                assert(val != NULL);
                vcur = val + val_used;
                val_allocated += sexp_val_grow_size;
              } else vcur++;
              
              squoted = 1;
	    }
	  else if (t[0] == '(')
	    {
	      vcur = val;
	      state = 8;
	    }
	  else
	    {
	      vcur = val;
	      state = 4;
              squoted = 1;
	    }
	  break;
	case 8:
	  if (esc == 0) {
	    if (t[0] == '(')
	      {
		qdepth++;
	      }
	    else if (t[0] == ')')
	      {
		qdepth--;
		state = 9;
	      }
            else if (t[0] == '\"')
              {
                state = 10;
              }
	  } else {
	    esc = 0;
	  }
	  vcur[0] = t[0];
	  if (t[0] == '\\') esc = 1;
	  else esc = 0;
          val_used++;

          if (val_used == val_allocated) {
            val = (char *)sexp_realloc(val,
				       val_allocated+sexp_val_grow_size,
				       val_allocated);
            assert(val != NULL);
            vcur = val + val_used;
            val_allocated += sexp_val_grow_size;
          } else vcur++;

	  t++;
          /* let it fall through to state 9 if we know we're transitioning
             into that state */
          if (state != 9)
            break;
	case 9:
	  if (qdepth == 0)
	    {
	      state = 1;
	      vcur[0] = '\0';
              sx = sexp_t_allocate();
	      assert(sx!=NULL);
	      elts++;
	      sx->ty = SEXP_VALUE;
              sx->val = val;
              sx->val_allocated = val_allocated;
              sx->val_used = val_used;
	      sx->next = NULL;
	      sx->aty = SEXP_SQUOTE;

              val = (char *)sexp_malloc(sizeof(char)*sexp_val_start_size);
              assert(val != NULL);
              val_allocated = sexp_val_start_size;
              val_used = 0;
              vcur = val;
	      
	      if (!empty_stack (stack))
		{
		  data = (parse_data_t *) top_data (stack);
		  if (data->fst == NULL)
		    {
		      data->fst = data->lst = sx;
		    }
		  else
		    {
		      data->lst->next = sx;
		      data->lst = sx;
		    }
		}
	      else
		{
                  /* looks like the whole expression was a single
                     quoted value!  So return it. */
                  cc->mode = mode;
                  cc->error = 0;
                  cc->squoted = 0;
                  cc->val = val;
                  cc->val_used = val_used;
                  cc->val_allocated = val_allocated;
                  cc->vcur = vcur;
                  cc->lastPos = t;
                  cc->depth = depth;
                  cc->qdepth = qdepth;
                  cc->state = 1;
                  cc->stack = stack;
                  cc->esc = 0;
                  cc->last_sexp = sx;
                  assert(sx != NULL);

                  return cc;
		}
	    }
	  else
	    state = 8;
	  break;
        case 10:
          if (t[0] == '\"' && esc == 0)
            {
              state = 8;
            }
          vcur[0] = t[0];
          if (t[0] == '\\') esc = 1;
          else esc = 0;
          val_used++;

          if (val_used == val_allocated) {
            val = (char *)sexp_realloc(val,
				       val_allocated+sexp_val_grow_size,
				       val_allocated);
            assert(val != NULL);
            vcur = val + val_used;
            val_allocated += sexp_val_grow_size;
          } else vcur++;

          t++;
          break;
        case 11:
          if (t[0] == '\n') {
            state = 1;
          }
          t++;
          break;
        case 12: /* pre: we saw a # and we're in inline binary mode */
          vcur[0] = t[0];
          if (t[0] == '\\') esc = 1;
          else esc = 0;
          val_used++;
          
          if (val_used == val_allocated) {
            val = (char *)sexp_realloc(val,
				       val_allocated+sexp_val_grow_size,
				       val_allocated);
            assert(val != NULL);
            vcur = val + val_used;
            val_allocated += sexp_val_grow_size;
          } else vcur++;
          
          if (t[0] == 'b') {
            state = 13; /* so far, #b */
          } else {
            state = 4; /* not #b, so plain ol' atom */
          }
          t++;
          break;

        case 13: /* pre: we saw a #b and we're in inline binary mode */
          vcur[0] = t[0];
          if (t[0] == '\\') esc = 1;
          else esc = 0;
          val_used++;
          
          if (val_used == val_allocated) {
            val = (char *)sexp_realloc(val,
				       val_allocated+sexp_val_grow_size,
				       val_allocated);
            assert(val != NULL);
            vcur = val + val_used;
            val_allocated += sexp_val_grow_size;
          } else vcur++;
          
          if (t[0] == '#') {
            state = 14; /* so far, #b# - we're definitely in binary
                           land now. */
            /* reset vcur to val, overwrite #b# with the size string. */
            vcur = val;
            val_used = 0;
          } else {
            state = 4; /* not #b#, so plain ol' atom */
          }
          t++;
          break;
          
        case 14:
          /**
           * so far we've read #b#.  Now, the steps of the process become:
           * proceed to read bytes in until we see # again.  This will be
           * an ASCII representation of the size.  At this point, we want
           * to read as many bytes as specified by this size string after
           * the #.
           */
          if (t[0] == '#') { /* done with size string */
            t++;
            state = 15;
            vcur[0] = '\0';

            binexpected = atoi(val);
            assert(binexpected > 0);
            binread = 0;
            bindata = (char *)sexp_malloc(sizeof(char)*binexpected);
            assert(bindata != NULL);
          } else { /* still reading size string */
            vcur[0] = t[0];
            if (t[0] == '\\') esc = 1;
            else esc = 0;
            val_used++;
            
            if (val_used == val_allocated) {
              val = (char *)sexp_realloc(val,
					 val_allocated+sexp_val_grow_size,
					 val_allocated);
              assert(val != NULL);
              vcur = val + val_used;
              val_allocated += sexp_val_grow_size;
            } else vcur++;

            t++;
          }

          break;

        case 15: /* reading binary blob */
          bindata[binread] = t[0];
          binread++;
          t++;

          if (binread == binexpected) {
            /* state = 1 -- create a sexp_t and head back */
            sx = sexp_t_allocate();
            assert(sx!=NULL);
            elts++;
            sx->ty = SEXP_VALUE;
            sx->bindata = bindata;
            sx->binlength = binread;
            sx->next = NULL;
            sx->aty = SEXP_BINARY;

            bindata = NULL;
            binread = binexpected = 0;
            
            state = 1;

            val_used = 0;
            vcur = val;
	      
            if (!empty_stack (stack))
              {
                data = (parse_data_t *) top_data (stack);
                if (data->fst == NULL)
                  {
                    data->fst = data->lst = sx;
                  }
                else
                  {
                    data->lst->next = sx;
                    data->lst = sx;
                  }
              }

          }

          break;

	default:
	  fprintf (stderr, "Unknown parser state %d.\n", state);
	  break;
	}

      /* the null check used to be part of the guard on the while loop.
         unfortunately, if we're in state 15, null is considered a
         perfectly valid byte.  This means the length passed in better
         be accurate for the parser to not walk off the end of the 
         string! */
      if (state != 15 && t[0] == '\0') keepgoing = 0;
    }

#ifdef _DEBUG_
  fprintf(stderr,"State Machine Iterations: %d\nMax Paren Depth: %d\n",
          fsm_iterations, maxdepth);
#endif /* _DEBUG_ */

  if (depth == 0 && elts > 0) {
    cc->mode = mode;
    cc->error = 0;
    cc->val = val;
    cc->squoted = squoted;
    cc->val_used = val_used;
    cc->val_allocated = val_allocated;
    cc->vcur = vcur;
    cc->lastPos = t;
    cc->depth = depth;
    cc->qdepth = qdepth;
    cc->state = 1;
    cc->stack = stack;
    cc->esc = 0;
    while (stack->top != NULL)
    {
      lvl = pop (stack);
      data = (parse_data_t *) lvl->data;
      sx = data->fst;
      /* free (data); */
      pd_deallocate(data);
      lvl->data = NULL;
    }
    cc->last_sexp = sx;
  } else {
    cc->mode = mode;
    cc->val = val;
    cc->esc = esc;
    cc->squoted = squoted;
    cc->vcur = vcur;
    cc->val_allocated = val_allocated;
    cc->val_used = val_used;
    if (t[0] == '\0' || t == bufEnd)
      cc->lastPos = NULL;
    else
      cc->lastPos = t;
    cc->depth = depth;
    cc->qdepth = qdepth;
    cc->state = state;
    cc->stack = stack;
    cc->last_sexp = NULL;
    cc->error = 0;
  }
  
  return cc;
}
