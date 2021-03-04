#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "plist.h"
#include "shellutils.h"

/* Sie können diese Vorgabe unverändert verwenden oder Sie können Ihre
 * shellutils.c aus 01R nutzen. Falls Sie einen Bug in
 * dieser Implementierung finden, melden Sie diesen bitte an
 * ruesch@ibr.cs.tu-bs.de
 */

command_t* parse_command_line(char* command_line)
{
    command_t* command;
    size_t     command_size;
    size_t     real_size;
    size_t     length = strlen(command_line);
    size_t     ceiling;
    int        argument_counter = 0;

    // strip newline
    if(command_line[length - 1] == '\n')
    {
        command_line[--length] = '\0';
    }

    // alloc command structure
    ceiling      = ((length + 1) / sizeof(int) + 1) * sizeof(int); // ceiling to sizeof(int)
    command_size = sizeof(*command);
    command_size += ceiling;       // for command_line copy
    command_size += sizeof(char*); // for argv array
    command_size += sizeof(char*); // for NULL terminator in argv
    real_size = command_size + 128;

    command = calloc(1, real_size);
    if(command == NULL)
    {
        return NULL;
    }

    // create copy of tokenized command line
    char* command_line_copy = ((char*)command) + sizeof(*command);
    memcpy(command_line_copy, command_line, length + 1);
    command->command_line = command_line_copy;

    char** argv_pos = (char**)(((char*)command) + sizeof(*command) + ceiling);
    for(char* token = strtok(command_line, " \t"); token; token = strtok(NULL, " \t"))
    {
        command_size += sizeof(char*);
        if(command_size > real_size)
        {
            real_size              = command_size + 128;
            command_t* new_command = realloc(command, real_size);
            if(new_command == NULL)
            {
                free(command);
                return NULL;
            }
            argv_pos = (char**)new_command + (argv_pos - (char**)command); // adjust to new position
            command  = new_command;
        }
        *(argv_pos++) = token;
        argument_counter++;
    }
    *argv_pos = NULL;

    // Regenerate command structure
    command_line_copy     = ((char*)command) + sizeof(*command);
    command->command_line = command_line_copy;

    command->argv = (char**)(command_line_copy + ceiling);

    // check for empty command
    if(argument_counter < 1)
    {
        command->parse_error = "empty command line";
        return command;
    }

    if(strcmp(*(argv_pos - 1), "&") == 0)
    {
        command->background = 1;
        argument_counter--;
        if(argument_counter < 1)
        {
            command->parse_error = "background execution of empty command requested";
            return command;
        }
        argv_pos--;
        *argv_pos = NULL;
    }

    if((*(argv_pos - 1))[0] == '>')
    {
        command->outFile = &(*(argv_pos - 1))[1];

        if(strlen(command->outFile) < 1)
        {
            command->parse_error = "stdout redirection to zero length filename requested";
            return command;
        }

        argument_counter--;
        if(argument_counter < 1)
        {
            command->parse_error = "stdout redirection of empty command requested";
            return command;
        }
        argv_pos--;
        *argv_pos = NULL;
    }

    return command;
}

/* prints status of terminated child processes */
void print_status(const char* command_line, int status)
{
    if(WIFSIGNALED(status))
    {
        fprintf(stderr, "Signal [%s] = %d\n", command_line, WTERMSIG(status));
    }
    else if(WIFEXITED(status))
    {
        fprintf(stderr, "Exitstatus [%s] = %d\n", command_line, WEXITSTATUS(status));
    }
    else if(WIFSTOPPED(status))
    {
        fprintf(stderr, "Stopped [%s] = %d\n", command_line, WSTOPSIG(status));
    }
}

/* just to make sure there is enough room for [unknown] */
#if PATH_MAX > 10
#define MAX_PATH_LEN PATH_MAX
#else
#define MAX_PATH_LEN 10
#endif
void prompt()
{
    char current_working_directory[MAX_PATH_LEN];
    if(NULL == getcwd(current_working_directory, MAX_PATH_LEN))
    {
        perror("getcwd");
        strcpy(current_working_directory, "[unknown]");
    }
    fprintf(stderr, "%s: ", current_working_directory);
}
