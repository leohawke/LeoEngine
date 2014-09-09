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
#include <stdlib.h>
#include <stdio.h>
#include "sexp_memory.h"

#ifdef _SEXP_LIMIT_MEMORY_

static int sexp_max_memory  = 4*1024*1024; /* default: 4MB */
static int sexp_used_memory = 0;

void *sexp_malloc(size_t size) {
  void *ptr;

  if (sexp_used_memory+size > sexp_max_memory) {
    fprintf(stderr,"ERROR: exceeded allocation limit of %d bytes\n",
	    sexp_max_memory);
    return NULL;
  }

  ptr = malloc(size);
  if (ptr != NULL) sexp_used_memory += size;

  return ptr;
}

void *sexp_calloc(size_t count, size_t size) {
  void *ptr;

  if (sexp_used_memory+(size*count) > sexp_max_memory) {
    fprintf(stderr,"ERROR: exceeded allocation limit of %d bytes\n",
	    sexp_max_memory);
    return NULL;
  }

  ptr = calloc(count, size);
  if (ptr != NULL) sexp_used_memory += size*count;

  return ptr;
}


void sexp_free(void *ptr, size_t size) {
  sexp_used_memory -= size;
  if (sexp_used_memory < 0) {
    fprintf(stderr,"WARNING: sexp_free caused used_memory to go negative.\n");
  }

  free(ptr);
}

void *sexp_realloc(void *ptr, size_t size, size_t oldsize) {
  void *p;

  if (sexp_used_memory+(size-oldsize) > sexp_max_memory) {
    fprintf(stderr,
	    "ERROR: realloc attempt would exceed memory limit of %d bytes\n",
	    sexp_max_memory);
    return NULL;
  }

  p = realloc(ptr,size);
  if (p != NULL) sexp_used_memory += size-oldsize;

  return p;
}

#endif /* _SEXP_LIMIT_MEMORY_ */
