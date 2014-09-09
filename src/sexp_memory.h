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

#ifndef __SEXP_MEMORY_H__
#define __SEXP_MEMORY_H__

/**
 * \file sexp_memory.h
 *
 * \brief Wrappers around basic memory allocation/deallocation routines to
 *        allow memory usage limiting.
 *
 * Written by Matt Sottile (matt@lanl.gov), August 2004.  See LICENSE
 * file for licensing details.
 */


/**
 * \defgroup memory Memory management routines
 */

#ifdef _SEXP_LIMIT_MEMORY_

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * \ingroup memory
   * Wrapper around malloc, will check and increment memory usage
   * counters if space is available.  Returns NULL if no memory left
   * to use, otherwise returns whatever malloc returns.
   */
  void *sexp_malloc(size_t size);

  /**
   * \ingroup memory 
   * Wrapper around calloc, will check and increment memory usage
   * counters if space is available.  Returns NULL if no memory left
   * to use, otherwise returns whatever calloc returns.
   */
  void *sexp_calloc(size_t count, size_t size);

  /**
   * \ingroup memory
   * Wrapper around free.  Instead of trusting sizeof(ptr) to return the
   * proper value, we explicitly pass the size of memory associated with
   * ptr.  Decrements memory usage counter and frees ptr.
   */
  void sexp_free(void *ptr, size_t size);

  /**
   * \ingroup memory
   * Wrapper around realloc.  Instead of trusting sizeof(ptr) to return the
   * proper value, we explicitly pass the size of memory associated with
   * ptr as the oldsize.  Increments the memory usage counter by 
   * (size-oldsize) if enough space available for realloc.  
   * Returns NULL if no memory left to use, otherwise returns whatever 
   * realloc returns.
   */
  void *sexp_realloc(void *ptr, size_t size, size_t oldsize);

#ifdef __cplusplus
}
#endif

#else

#define sexp_calloc(count,size) calloc(count,size)
#define sexp_malloc(size) malloc(size)
#define sexp_free(ptr,size) free(ptr)
#define sexp_realloc(ptr,size,oldsize) realloc((ptr),(size))

#endif

#endif /* __SEXP_MEMORY_H__ */
