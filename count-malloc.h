/* count-malloc.h : count mallocs and frees */
/*

  export MALLOC_COUNT_SHOW
or
  setenv MALLOC_COUNT_SHOW
to see details of malloc(), realloc(), and free()s.

Add the following to the source code and compile with -DCOUNT_MALLOC to
count mallocs and frees in the source code:

#ifdef COUNT_MALLOC
#include <count-malloc.h>
#endif

Copyright (C) 2005 by zunda <zunda at freeshell.org>

Permission is granted for use, copying, modification, distribution,
and distribution of modified versions of this work under the terms of
GPL version 2 or later.

*/

#ifndef COUNT_MALLOC_H
#define COUNT_MALLOC_H

#include <stdlib.h>

void counting_malloc_register_only(void *ptr, const char *func, const char *file, const int line);
void *counting_malloc(size_t size, const char *func, const char *file, const int line);
void counting_free(void *ptr);
void *counting_realloc(void *ptr, size_t size, const char *func, const char *file, const int line);

#define malloc(size) counting_malloc((size), __func__, __FILE__, __LINE__)
#define nomalloc(ptr, func) counting_malloc_register_only((ptr), (func), __FILE__, __LINE__)
#define free counting_free
#define realloc(ptr, size) counting_realloc((ptr), (size), __func__, __FILE__, __LINE__)

#endif	/* COUNT_MALLOC_H */
