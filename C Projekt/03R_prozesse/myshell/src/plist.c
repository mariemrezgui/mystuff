#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "plist.h"

/* Die Funktionen insertElement() und removeElement() bitte unveraendert lassen!
 * Falls Sie einen Bug in dieser Implementierung finden, melden Sie diesen bitte
 * an ruesch@ibr.cs.tu-bs.de.
 */

static struct list_element
{
    pid_t process_id;          // saved processID
    char* command_line;        // saved command line of process
    struct list_element* next; // next list element
} * head;

void walk_list(int (*callback)(pid_t, const char*))
{
    struct list_element* actual = head;
    /*Iterating through the linked List (of all not removed
     *background processes)
     */
    while (actual != NULL)
    {
        callback(actual->process_id, actual->command_line);
        actual = actual->next;
    }
}
int insert_element(pid_t process_id, const char* command_line)
{
    struct list_element* run   = head;
    struct list_element* carry = NULL;

    while (run)
    {
        if (run->process_id == process_id)
        {
            return -1;
        }

        carry = run;
        run   = run->next;
    }

    run = malloc(sizeof(struct list_element));
    if (NULL == run)
    {
        return -2;
    }

    run->command_line = strdup(command_line);
    if (NULL == run->command_line)
    {
        free(run);
        return -2;
    }

    run->process_id = process_id;
    run->next       = NULL;

    /* Add new element */
    if (NULL == carry)
    {
        head = run;
    }
    else
    {
        carry->next = run;
    }

    return process_id;
}

int remove_element(pid_t process_id, char* buffer, size_t buffer_length)
{
    if (head == NULL)
    {
        return -1;
    }

    struct list_element* run   = head;
    struct list_element* carry = NULL;

    while (run)
    {
        if (run->process_id == process_id)
        {
            if (NULL == carry)
            {
                head = head->next;
            }
            else
            {
                carry->next = run->next;
            }

            strncpy(buffer, run->command_line, buffer_length);
            if (buffer_length > 0)
            {
                buffer[buffer_length - 1] = '\0';
            }
            int return_value = strlen(run->command_line);

            /* Free memory */
            free(run->command_line);
            free(run);
            return return_value;
        }

        carry = run;
        run   = run->next;
    }

    /* PID not found */
    return -1;
}
