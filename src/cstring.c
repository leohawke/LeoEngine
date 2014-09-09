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
 * Implementation of stuff in cstring.h to make ron happy 
 */
#include "cstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sexp_memory.h"

/**
 * growth size for cstrings -- default is 8k.  use sgrowsize() to adjust. 
 */
static size_t cstring_growsize = 8192;

void sgrowsize(size_t s) {
  assert(s > 0);
  cstring_growsize = s;
}

CSTRING *snew(size_t s) {
  CSTRING *cs;

  cs = (CSTRING *)sexp_malloc(sizeof(CSTRING));
  assert(cs != NULL);

  cs->len = s;
  cs->curlen = 0;

  cs->base = (char *)sexp_calloc(sizeof(char),s);
  assert(cs->base != NULL);

  return cs;
}

CSTRING *sadd(CSTRING *s, char *a) {
  int alen;
  char *newbase;

  /* no string, so bail */
  if (s == NULL) {
    return NULL;
  }

  /* nothing to add, return s */
  if (a  == NULL) {
    return s;
  }

  alen = strlen(a);

  if (s->curlen + alen >= s->len) {
    newbase = (char *)sexp_realloc(s->base,
				   s->len+cstring_growsize+alen,
				   s->len);
    if (! newbase) {
	perror("realloc string");
	s->len = s->curlen = 0;
	s->base = 0;
	return 0;
    }

    s->len += cstring_growsize + alen;
    s->base = newbase;
  }

  memcpy(&s->base[s->curlen],a,alen);
  s->curlen += alen;
  s->base[s->curlen] = 0;
  return s;
}

CSTRING *saddch(CSTRING *s, char a) {
  char *newbase;

  if (s == NULL) {
    return NULL;
  }

  if (s->curlen + 1 >= s->len) {
    newbase = (char *)sexp_realloc(s->base,
				   s->len+cstring_growsize+1,
				   s->len);
    if (! newbase) {
	perror("realloc string");
	s->len = s->curlen = 0;
	s->base = 0;
	return 0;
    }

    s->len += cstring_growsize+1;
    s->base = newbase;
  }

  s->base[s->curlen] = a;
  s->curlen++;
  s->base[s->curlen] = 0;
  return s;
}

CSTRING *strim(CSTRING *s) {
  char *newbase;

  if (s == NULL) {
    return NULL;
  }

  /* no trimming necessary? */
  if (s->len == s->curlen+1) {
    return s;
  }

  newbase = (char *)sexp_realloc(s->base,
				 s->curlen+1,
				 s->len);
  if (!newbase) {
    perror("realloc string in trim");
    s->len = s->curlen = 0;
    s->base = 0;
    return NULL;
  }

  s->len = s->curlen+1;
  s->base = newbase;

  return s;
}

char *toCharPtr(CSTRING *s) {
  return s->base;
}

void sempty(CSTRING *s) {
    s->curlen = 0;
}

void sdestroy(CSTRING *s) {
  sexp_free(s->base,s->len);
  sexp_free(s,sizeof(CSTRING));
}
