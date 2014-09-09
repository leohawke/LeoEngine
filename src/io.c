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
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sexp.h"
#include <assert.h>

/**
 * initialize an io-wrapper
 */
sexp_iowrap_t *init_iowrap(int fd) {
  sexp_iowrap_t *iow;

  iow = (sexp_iowrap_t *)sexp_malloc(sizeof(sexp_iowrap_t));
  assert(iow != NULL);

  iow->cc = NULL;
  iow->fd = fd;
  iow->cnt = 0;

  return iow;
}

/**
 *
 */
void destroy_iowrap(sexp_iowrap_t *iow) {
  if (iow == NULL) return; /* idiot */

  destroy_continuation(iow->cc);
  sexp_free(iow, sizeof(sexp_iowrap_t));
}

/**
 *
 */
sexp_t *read_one_sexp(sexp_iowrap_t *iow) {
  sexp_t  *sx = NULL;

  if (iow->cnt == 0) {
    iow->cnt = read(iow->fd,iow->buf,BUFSIZ);
    if (iow->cnt == 0) return NULL;
  }

  iow->cc = cparse_sexp(iow->buf,iow->cnt,iow->cc);

  while (iow->cc->last_sexp == NULL) {
    if (iow->cc->error != 0) {
      fprintf(stderr,"ERROR\n");
      return NULL;
    }

    iow->cnt = read(iow->fd,iow->buf,BUFSIZ);

    if (iow->cnt == 0)
      return NULL;

    iow->cc = cparse_sexp(iow->buf,iow->cnt,iow->cc);
  }

  sx = iow->cc->last_sexp;
  iow->cc->last_sexp = NULL;

  return sx;
}
