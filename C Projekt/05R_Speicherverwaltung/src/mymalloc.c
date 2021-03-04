#include "mymalloc.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "colorCodes.h"

chunk_t* root = NULL;

//===================================================================================================================//

//! --------------------------------
//! Write your helper functions here
//! --------------------------------






//===================================================================================================================//

/**
 * @brief get_page_size returns page size in bytes
 * @return page size in byte
 */
size_t get_page_size(void)
{
    return sysconf(_SC_PAGESIZE);
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief open_file: opens a file with the size of 1 page for that we want to manage the memory
 * @return file descriptor
 */
int open_file(void)
{
    // Open or create file and check for errors
    int fd;
    if((fd = open(FILE_NAME, O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR)) == -1){
        perror("Open");

        // Exit on open error
        exit(-1);
    }

    // Reserve one page and check for errors
    int pf;
    if((pf = posix_fallocate(fd, 0, get_page_size())) != 0){
        fprintf(stderr, "posix_fallocate: %d Exiting...\n", pf);

        //exit on posix_fallocate error
        exit(-1);
    }

    return fd;
}

//----------------------------------block---------------------------------------------------------------------------------//

/**
 * @brief my_malloc allocates memory that is backed by a file
 * @param size: size of memory to allocate in bytes
 * @return pointer to allocated memory
 */
void* my_malloc(size_t size){
    memory_block_t* mb;
    void* start = START_ADDRESS;

    // root = NULL: first run of my_alloc
    if (root == NULL){
        int fd = open_file();

        // mmap file
        int file;
        if ((file = mmap(START_ADDRESS, get_page_size(), PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0) == MAP_FAILED)){
            perror("mmap");
            exit(-1);
        }

        // Initialize root
        root = (chunk_t*) START_ADDRESS;
        root->file = fd;
        root->size = get_page_size();

        //create first memory_block
        memory_block_t* fmb = (memory_block_t*) (((char*) root) + sizeof(chunk_t));
        fmb->size = get_page_size() - sizeof(chunk_t) - sizeof(memory_block_t);

        // set first element of free space list to fmb
        root->free_space_list = fmb;

        // set mb to start of free space list
        mb = root->free_space_list;
        mb->size = fmb->size;
    }
        // root already exists
    else {
        mb = root->free_space_list;
    }

    if (size != 0) {
        // calculate needen memory as multiples of sizeof(memory_block_t)
        size = ((size - 1) / sizeof(memory_block_t) + 1) * sizeof(memory_block_t);

        // search free space list for memory block with enough space
        while(mb->next != NULL && mb->size < size) {
            mb = mb->next;
        }

        // return if there is no memory block with enough space left
        if (mb->size < size){
            errno = ENOMEM;
            return NULL;
        }
    }
        // return if size is 0
    else {
        errno = ENOMEM;
        return NULL;
    }

    // calculate start of data field
    start = ((char*) mb + sizeof(memory_block_t));

    // check if the current memory block has enough space left for another one after allocation
    if (mb->size - size > sizeof(memory_block_t)){

        // create and initialize following memory block
        memory_block_t* nmb = (memory_block_t*) ((char*) start + size);
        nmb->size = mb->size - size - sizeof(memory_block_t);
        nmb->previous = mb->previous;
        nmb->next = mb->next;

        // update pointers of the free space list
        if (root->free_space_list == mb) {
            root->free_space_list = nmb;
        }
        if (mb->previous != NULL) {
            mb->previous->next = nmb;
        }
        if (nmb->next != NULL) {
            nmb->next->previous = nmb;
        }
        mb->size = size;
    }
    else {
        //update pointers of the free space list
        if (mb->previous == NULL) {
            mb->next->previous = NULL;
        }
        else if (mb->next == NULL) {
            mb->previous->next =  NULL;
        }
        else {
            mb->previous->next = mb->next;
            mb->next->previous = mb->previous;
        }
        if (root->free_space_list == mb) {
            root->free_space_list = mb->next;
        }
    }

    // mark allocated memory block as full with MAGIC_NUMBER
    mb->previous = MAGIC_NUMBER;
    mb->next = MAGIC_NUMBER;

    return start;
}

void my_free(void *memory_location) {
    // check for initial errors
    if (root == NULL) {
        fprintf(stderr, "root doesnt exist\n");
        return;
    }
    if (memory_location == NULL) {
        return;
    }

    // get memory block of memory_location and check if it is allocated
    memory_block_t *mb = (memory_block_t *) ((char *) memory_location - sizeof(memory_block_t));
    if (mb->previous != MAGIC_NUMBER || mb->next != MAGIC_NUMBER) {
        fprintf(stderr, "memory location not allocated\n");
        return;
    }

    // get the positon of the memory block in relation to other blocks from the free space list and update the pointers of the elements
    char *location = (char *) mb;
    if (location < (char *) root->free_space_list) {
        // memory block is in front of the first element
        mb->next = root->free_space_list;
        mb->previous = NULL;
        root->free_space_list = mb;
        mb->next->previous = mb;
    } else {
        memory_block_t *li = root->free_space_list;
        while (li->next != NULL) {
            li = li->next;
        }

        if (location > (char *) li) {
            // memory block is behind the entire free space list
            mb->previous = li->previous;
            mb->next = NULL;
            mb->previous->next = mb;
        } else if ((char *) root->free_space_list < location && location < (char *) li) {
            // memory block is between two elements of the free space list
            li = root->free_space_list;
            while ((char *) li->next < location) {
                li = li->next;
            }

            mb->next = li->next;
            mb->previous = li;
            mb->next->previous = mb;
            mb->previous->next = mb;
        } else {
            fprintf(stderr, "memory location invalid\n");
        }
    }

    // check for other empty blocks around the recently freed one and merge empty memory blocks
    // get the memory block right in front of mb
    memory_block_t *pmb = root->free_space_list;
    while ((char *) pmb + sizeof(memory_block_t) + pmb->size < location) {
        pmb = (memory_block_t *) ((char *) pmb + sizeof(memory_block_t));
    }

    // get the memory block right after mb
    memory_block_t *nmb = (memory_block_t *) (location + sizeof(memory_block_t) + mb->size);


    if (pmb->next == mb && nmb->previous == mb) {
        // case 1: mb is between two empty memory blocks
        // merge mb and mb->next into pmb
        pmb->size += sizeof(memory_block_t) + mb->size + sizeof(memory_block_t) + nmb->size;
        if (nmb->next != NULL) {
            pmb->next = nmb->next;
            nmb->next->previous = pmb;
        } else {
            pmb->next = NULL;
        }
    }
        // case 2: either the previous or the next memory block to mb is empty
    else if (pmb->next == mb) {
        // merge mb into pmb
        pmb->size += sizeof(memory_block_t) + mb->size;
        if (nmb != NULL) {
            pmb->next = mb->next;
            mb->next->previous = pmb;
        } else {
            pmb->next = NULL;
        }
    } else if (nmb->previous == mb) {
        // merge nmb into mb
        mb->size += sizeof(memory_block_t) + nmb->size;
        if (nmb->next != NULL) {
            mb->next = nmb->next;
            mb->next->previous = mb;
        } else {
            mb->next = NULL;
        }
    }
}

