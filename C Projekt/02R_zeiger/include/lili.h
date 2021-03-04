#ifndef _LILI_H_
#define _LILI_H_

typedef struct element
{
    unsigned int    data;
    struct element* next;
} element_t;

unsigned int* insert_element(unsigned int value);

unsigned int remove_element(void);

void print_lili(void);

#endif
