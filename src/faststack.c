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
/**
 * faststack.c : implementation of fast stack.
 *
 * matt sottile / matt@lanl.gov
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "faststack.h"
#include "sexp.h"

/**
 * create an empty stack.
 */
faststack_t *
make_stack ()
{
  faststack_t *s;

  s = (faststack_t *) sexp_malloc (sizeof (faststack_t));
  assert (s != NULL);

  s->top = s->bottom = NULL;
  s->height = 0;

  return s;
}

/**
 * free all levels of a stack
 */
void
destroy_stack (faststack_t * s)
{
  stack_lvl_t *sl;

  /* start at the bottom */
  sl = s->bottom;

  /* if bottom is null, no levels to free. */
  if (sl == NULL)
    return;

  /* go up to the top of the allocated stack */
  while (sl->above != NULL)
    sl = sl->above;

  /* until we get to the bottom (where below is null), free the data
     at each level and the level itself. */
  while (sl->below != NULL)
    {
      sl = sl->below;
      sexp_free (sl->above, sizeof(stack_lvl_t));
    }

  /* free the bottom level */
  sexp_free (sl, sizeof(stack_lvl_t));

  /* free the stack wrapper itself. */
  sexp_free (s, sizeof(faststack_t));
}

/**
 * push a level onto the cur_stack.  reuse levels that have
 * been previously allocated, allocate a new one if none
 * are available.
 */
faststack_t *
push (faststack_t * cur_stack, void *data)
{
  stack_lvl_t *top = cur_stack->top;
  stack_lvl_t *tmp;

  /* if top isn't null, try to push above it. */
  if (top != NULL)
    {
      /* if above isn't null, set the stack top to it and set the
         data */
      if (top->above != NULL)
	{
	  top = cur_stack->top = cur_stack->top->above;
	  top->data = data;
	}
      else
	{
	  /* otherwise, allocate a new level. */

	  tmp = top->above = (stack_lvl_t *) sexp_malloc (sizeof (stack_lvl_t));
          assert(tmp != NULL);

	  tmp->below = cur_stack->top;
	  tmp->above = NULL;
	  cur_stack->top = tmp;
	  tmp->data = data;
	}
    }
  else
    {
      if (cur_stack->bottom != NULL)
	{
	  cur_stack->top = cur_stack->bottom;
	  cur_stack->top->data = data;
	}
      else
	{
	  tmp = cur_stack->top =
	    (stack_lvl_t *) sexp_malloc (sizeof (stack_lvl_t));
          assert(tmp != NULL);

	  cur_stack->bottom = tmp;
	  tmp->above = NULL;
	  tmp->below = NULL;
	  tmp->data = data;
	}
    }

  cur_stack->height++;

  return cur_stack;
}

/**
 * pop the top of the stack, return the stack level that was
 * popped of.
 */
stack_lvl_t *
pop (faststack_t * s)
{
  stack_lvl_t *top;
  top = s->top;

  /* if stack top isn't null, set the top pointer to the next
     level down and return the old top. */
  if (top != NULL && s->height > 0)
    {
      s->top = s->top->below;
      s->height--;
    }
  else
    {
      fprintf(stderr,"STACK: non-null top, but height < 0!\n");
    }

  return top;
}
