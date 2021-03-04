#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

#include <stdlib.h>

#define MAGIC_NUMBER ((void*)0xC0CAC01AADD511FE) // Magic Number that marks the memory being in use
#define FILE_NAME "memory_map.bin"
#define START_ADDRESS ((void*)0x0000000010000000)

/**
 * @brief get_page_size returns page size in bytes
 * @return page size in byte
 */
size_t get_page_size(void);

/**
 * @brief open_file: opens a file with the size of 1 page for that we want to manage the memory
 * @return file descriptor
 */
int open_file(void);

/**
 * @brief my_malloc allocates "size" of memory and returns a pointer to this memory
 * @param size: how much memory to allocate in bytes
 * @return a pointer that points at the start of the allocated memory region
 */
void* my_malloc(size_t size);

/**
 * @brief my_free releases memory pointed by memory_location
 * @param memory_location: pointer to the start of the memory region that should be released
 */
void my_free(void* memory_location);

typedef struct memory_block
{
    size_t               size;
    struct memory_block *next, *previous;
} memory_block_t;

typedef struct
{
    int             file;
    size_t          size;
    memory_block_t* free_space_list;
} chunk_t;

extern chunk_t* root;

#include "dotmalloc.h"
#endif
