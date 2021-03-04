#ifndef _H_DOT_MALLOC_
#define _H_DOT_MALLOC_

#include "mymalloc.h"

void checkpoint(const char *name);
void analyse_memory(chunk_t *addr);
void analyse_file(const char *infile);

#endif
