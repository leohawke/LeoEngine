/**
 * malloc_util.c : malloc debugging routines
 * 
 * written by erik a. hendriks (hendriks@lanl.gov).
 *
 * Copyright 2002 Erik Arjan Hendriks.
 * This software may be used and distributed according to the terms of the 
 * GNU Public License, incorporated herein by reference. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malloc_util.h"

/* XXX Write some malloc wrappers to allow clean recovery from parse errors */
#undef malloc
#undef free

typedef struct {
    void *addr;
    int   valid;
    int   size;
    char *file;
    int   line;
    char *dfile;
    int   dline;
    int   checkpt;		/* value of last_check when created */
} alloc_t;
#define NALLOC 1000
static alloc_t alloc_list[NALLOC];
static int last_check = 0;

static inline
void set_alloc(int i, void *addr, size_t bytes, char *file, int lineno) {
    alloc_list[i].addr = addr;
    alloc_list[i].valid= 1;
    alloc_list[i].size = bytes;
    alloc_list[i].file = file;
    alloc_list[i].line = lineno;
    alloc_list[i].dfile = "NOT FREED";
    alloc_list[i].dline = 0;
    alloc_list[i].checkpt = last_check;
    printf("    ALLOC %10s %4d %4d %p\n", file, lineno, bytes, addr);
}

static void
add_alloc(void *addr, size_t bytes, char *file, int lineno) {
    int i;
    /* First look for a completely unused slot */
    for (i=0; i < NALLOC; i++) {
	if (alloc_list[i].addr == 0 ||
	    alloc_list[i].addr == addr) {
	    set_alloc(i, addr,bytes, file, lineno);
	    return;
	}
    }
    fprintf(stderr, "XXX wrapping.\n");
    for (i=0; i < NALLOC; i++) {
	if (alloc_list[i].valid == 0) {
	    set_alloc(i, addr,bytes, file, lineno);
	    return;
	}
    }

    fprintf(stderr, "XXX Ran out of alloc spaces (%d allocated)\n", NALLOC);
    abort();
}

static void
check_dealloc(void *addr, char *file, int lineno) {
    int i;
    for (i=0; i < NALLOC; i++) {
	if (alloc_list[i].addr == addr) {
	    if (alloc_list[i].valid) {
		alloc_list[i].valid = 0;
		alloc_list[i].dfile = file;
		alloc_list[i].dline = lineno;
		printf("    FREE  %10s %4d %4d %p\n",
		       file, lineno, alloc_list[i].size, addr);
		return;
	    } else {
		printf("XXX BLOCK FREED TWICE:  addr=%p bytes=%d\n"
		       "    ALLOC AT:    %10s %d\n"
		       "    1st FREE AT: %10s %d\n"
		       "    2nd FREE AT: %10s %d\n",
		       alloc_list[i].addr, alloc_list[i].size,
		       alloc_list[i].file, alloc_list[i].line,
		       alloc_list[i].dfile, alloc_list[i].dline,
		       file, lineno);
		abort();
	    }
	}
    }
    printf("XXX BLOCK NEVER ALLOCATED: %10s %4d %p\n", file, lineno, addr);
    abort();
}

void x_init_alloc(void) {
    int i;
    for (i=0; i < NALLOC; i++) alloc_list[i].addr = 0;
    last_check = 0;
}

void x_check_point_count(void) {
    int i;
    int nblocks=0, nbytes=0;
    for (i=0; i < NALLOC; i++) {
	if (alloc_list[i].addr && alloc_list[i].valid &&
	    alloc_list[i].checkpt == last_check) {
	    nblocks++;
	    nbytes += alloc_list[i].size;
	    printf("XXX Block: %p %5d %10s %d\n",
		   alloc_list[i].addr, alloc_list[i].size,
		   alloc_list[i].file, alloc_list[i].line);
	}
    }
    printf("XXX CHECK POINT %d    %d blocks %d bytes allocated and\n"
	   "                      not freed since last checkpoint\n",
	   last_check, nblocks, nbytes);
}

void x_inc_check_point(void) {
    last_check++;
}

void *x_malloc(char * file, int lineno, size_t bytes) {
    void *t;
    t = malloc(bytes);
    add_alloc(t, bytes, file, lineno);
    return t;
}

void x_free(char *file, int lineno, void *block) {
    check_dealloc(block, file, lineno);
    free(block);
}

char *x_strdup(char *file, int lineno, char *str) {
    int len;
    char *tmp;
    len = strlen(str);
    tmp = x_malloc(file, lineno, len+1);
    strcpy(tmp, str);
    return tmp;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * End:
 */
