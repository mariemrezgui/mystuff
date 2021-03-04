#include <stdio.h>
#include "lili.h"

int main()
{
    printf("remove() => %d\n", remove_element());
    print_lili();
    printf("insert(%d)\n", *insert_element(47));
    print_lili();
    printf("insert(%d)\n", *insert_element(11));
    print_lili();
    printf("insert(%d)\n", *insert_element(23));
    print_lili();
    printf("insert(%d)\n", *insert_element(11));
    print_lili();
    printf("remove() => %d\n", remove_element());
    print_lili();
    printf("remove() => %d\n", remove_element());
    print_lili();
    printf("insert(%d)\n", *insert_element(18));
    print_lili();
    printf("insert(%d)\n", *insert_element(43));
    print_lili();
}
