#include "lili.h"
#include <stdio.h>
#include <stdlib.h>

element_t* head = NULL;

unsigned int* insert_element(unsigned int value)
{

    element_t* element = malloc(sizeof(element_t));
    element->data      = value;
    element->next      = NULL;

    if (element == NULL)
    {

        perror("Fehler Speicherallokation");
        return NULL;
    }

    if (head == NULL)
    {
        head = element;
        return &element->data;
    }
    else
    {

        element_t* ElementAtEndOfList = head;
        while (ElementAtEndOfList->next != NULL)
        {
            ElementAtEndOfList = ElementAtEndOfList->next;
        }
        ElementAtEndOfList->next = element;
    }
    return &element->data;
}

/*-----------------------------------*/

unsigned int remove_element(void)
{
    element_t* actuel;
    unsigned int returnedValue;
    if (head == NULL)
    {

        printf("WARNING: nothing to remove, lili is empty\n");

        return 0;
    }
    else
    {
        actuel        = head;
        returnedValue = actuel->data;
        head          = head->next;
        free(actuel);
    }

    return returnedValue;
}

/*-----------------------------------*/

void print_lili(void)
{
    element_t* actuel = head;
    printf("print_lili: ");
    while (actuel != NULL)
    {

        printf("%d, ", actuel->data);
        actuel = actuel->next;
    }
    printf("\n");
}
