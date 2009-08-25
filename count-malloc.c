/* count-malloc.c : count mallocs and frees */
/*

Copyright (C) 2005 by zunda <zunda at freeshell.org>

Permission is granted for use, copying, modification, distribution,
and distribution of modified versions of this work under the terms of
GPL version 2 or later.

*/

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#define __attribute__(x)	/*NOTHING*/
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <count-malloc.h>

#undef malloc
#undef free
#undef realloc
#include <sys/types.h>

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

struct _malloc_entry_s
{
	void *ptr;
	const char *func;
	const char *file;
	int line;
	size_t size;
	struct _malloc_entry_s *next;
};

int malloc_count = 0;
static int malloc_count_reg = 0;
static struct _malloc_entry_s *first = NULL;
static struct _malloc_entry_s *last = NULL;

static void _add(void *ptr, const char *func, const char *file, int line, size_t size);
static int _remove(void *ptr);
static void _counting_check(void);

void
_add(void *ptr, const char *func, const char *file, int line, size_t size)
{
	char ptrbuf[10];
	struct _malloc_entry_s *new;
	malloc_count++;
	new = (struct _malloc_entry_s *) malloc(sizeof(struct _malloc_entry_s));
	if (new)
		{
			new->ptr = ptr;
			new->func = func;
			new->file = file;
			new->line = line;
			new->size = size;
			new->next = NULL;
			if (!first) first = new;
			if (last) last->next = new;
			last = new;
		}
	else
		{
			fputs("!!    ", stderr);
			snprintf(ptrbuf, sizeof(ptrbuf), "%9p", ptr);
			fputs(ptrbuf, stderr);
			fputs(" - failed to register a memory allocation.\n", stderr);
		}
}

int
_remove(void *ptr)
{
	struct _malloc_entry_s *prev, *cur;
	malloc_count--;
	prev = NULL;
	cur = first;
	while(cur && cur->ptr != ptr)
		{
			prev = cur;
			cur = cur->next;
		}
	if (cur && cur->ptr == ptr)
		{
			if (prev)
				{
					prev->next = cur->next;
				}
			else
				{
					first = cur->next;
				}
			if (last == cur)
				{
					last = prev;
				}
			free(cur);
		}
	if (cur)
		return 1;
	else
		return 0;
}

void
_counting_check(void)
{
	size_t i;
	unsigned char *ptr;
	if (malloc_count != 0)
		{
			struct _malloc_entry_s *cur;
			fprintf(stderr, "!! %d block(s) left\n", malloc_count);
			cur = first;
			while(cur)
				{
					fprintf(stderr, "!! %9p allocated in %s %s:%d\n", cur->ptr, cur->func, cur->file, cur->line);
					fputs("   ", stderr);
					for(i = 0; i < (cur->size > 70 ? 70 : cur->size); i++)
						{
							ptr = (unsigned char*) cur->ptr + i;
							fputc(isprint((int) *ptr) ? *ptr : ' ', stderr);
						}
					fputs("\n", stderr);
					cur = cur->next;
				}
			abort();
		}
}

void
counting_malloc_register_only(void *ptr, const char *func, const char *file, const int line)
{
	if (!malloc_count_reg)
		{
			malloc_count_reg = 1;
			atexit(_counting_check);
		}
	_add(ptr, func, file, line, 0);
	if (getenv("MALLOC_COUNT_SHOW"))
		fprintf(stderr, "! %3d %9p - in %s around %s:%d\n", malloc_count, ptr, func, file, line);
}

void *
counting_malloc(size_t size, const char *func, const char *file, const int line)
{
	void *r;
	
	if (!malloc_count_reg)
		{
			malloc_count_reg = 1;
			atexit(_counting_check);
		}
	r = malloc(size);
	if (r) _add(r, func, file, line, size);
	if (getenv("MALLOC_COUNT_SHOW"))
		fprintf(stderr, "! %3d %9p - %ld bytes in %s %s:%d\n", malloc_count, r, size, func, file, line);
	return r;
}

void
counting_free(void *ptr)
{
	if (getenv("MALLOC_COUNT_SHOW"))
		fprintf(stderr, "! %3d %9p - freeing\n", malloc_count, ptr);
	free(ptr);
	if (!_remove(ptr))
		fprintf(stderr, "!!    %9p - DOUBLE FREE?\n", ptr );
}

void *
counting_realloc(void *ptr, size_t size, const char *func, const char *file, const int line)
{
	void *r;
	
	if (!malloc_count_reg)
		{
			malloc_count_reg = 1;
			atexit(_counting_check);
		}
	r = realloc(ptr, size);
	if (ptr)
		{
			_remove(ptr);
			if (getenv("MALLOC_COUNT_SHOW"))
				fprintf(stderr, "! %3d %9p - reallocated in %s %s:%d\n", malloc_count, ptr, func, file, line);
		}
	if (getenv("MALLOC_COUNT_SHOW"))
		fprintf(stderr, "! %3d %9p - %ld bytes in %s %s:%d\n", malloc_count, r, size, func, file, line);
	if (r) _add(r, func, file, line, size);
	return r;
}
