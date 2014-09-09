/**
 * malloc_util.h : malloc debugging routines
 * 
 * written by erik a. hendriks (hendriks@lanl.gov).
 *
 * Copyright 2002 Erik Arjan Hendriks.
 * This software may be used and distributed according to the terms of the 
 * GNU Public License, incorporated herein by reference. 
 */

#ifndef _MALLOC_UTIL_H_
#define _MALLOC_UTIL_H_

#include <sys/types.h>

/*-- Memory debugging stuff --*/
void  x_init_alloc(void);

void  x_check_point_count(void);
void  x_inc_check_point(void);

void *x_malloc(char *, int, size_t);
void  x_free  (char *, int, void *);
char *x_strdup(char *, int, char *);

#ifdef _DEBUG_MALLOCS_
#define malloc(x) x_malloc(__FILE__, __LINE__, (x))
#define safe_malloc(x) x_malloc(__FILE__, __LINE__, (x))
#define free(x)   x_free  (__FILE__, __LINE__, (x))
#define strdup(x) x_strdup(__FILE__, __LINE__, (x))
#endif

#endif
