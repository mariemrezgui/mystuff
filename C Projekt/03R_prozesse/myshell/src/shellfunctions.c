#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "plist.h"
#include "shellfunctions.h"
#include "shellutils.h"

// Callback function prints the processID and the command line of process
int callback(pid_t w, const char* command_line)
{
    printf("Prozess-ID:%d %s\n", (int)w, command_line);
    return 0;
}

// Function to read Current Directory
void read_input(char* const command_line, int size, FILE* stream)
{
    prompt();
    // get command and put it in command_line
    char* result = fgets(command_line, size, stream);
    if (result == NULL)
    {
        if (feof(stream))
        {
            // If we get EOF, print Goodbye and exit.
            printf("Goodbye!\n");
            exit(0);
        }
    }
}

void execute_command(char* command_line)
{
    command_t* s = parse_command_line(command_line);

    int wstatus;

    // check for emty command
    if (s->command_line == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    // check for other Errors in the parse_command_line method
    if (s->parse_error != NULL)
    {
        if (s->argv[0] == NULL)
        {
            perror("empty command");
        }
        else
        {
            perror(s->parse_error);
        }
        free(s);
        return;
    }

    // change directory Command
    if (strcmp(s->argv[0], "cd") == 0)
    {
        // check for Errors
        if (s->argv[1] != NULL)
        {
            if (chdir(s->argv[1]) == -1)
            {
                perror("cd");
            }
        }
        else
        {
            fprintf(stderr, "expected argument to \"cd\"\n");
        }
        free(s);
        return;
    }

    /* the Command jobs shows the processID and the command line of all
     *currently running backgrouprocesses
     */
    if (strcmp(s->argv[0], "jobs") == 0)
    {
        walk_list(callback);
        free(s);
        return;
    }

    pid_t child_pid, w;

    child_pid = fork();

    if (child_pid < 0)
    {
        // Error forking
        perror("Fork");
        exit(EXIT_FAILURE);
    }

    // child process
    if (child_pid == 0)
    {
        execvp((s->argv)[0], s->argv);
        if (execvp((s->argv)[0], s->argv) < 0)
        {
            perror(s->argv[0]);
            free(s);
            exit(EXIT_FAILURE);
        }
    }
    if (s->background == 1)
    {
        // Current Directory is as a Background process executed
        if (insert_element(child_pid, s->command_line) < 0)
        {
            // Error inserting ein element in List
            perror("insertion error");
            free(s);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Parent process
        // waiting for child to terminate
        w = waitpid(child_pid, &wstatus, 0);
        if (w == -1)
        {
            perror("waitpid");
            free(s);
            exit(EXIT_FAILURE);
        }

        print_status(s->command_line, wstatus);
    }
    free(s);
}

// Treatement of Zombieprocesses
void collect_defunct_process()
{
    char command_line[COMMAND_LINE_MAXIMUM];
    int status;
    pid_t w;

    while ((w = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // remove Zombies
        if (remove_element(w, command_line, COMMAND_LINE_MAXIMUM) < 0)
        {
            perror("remove error");
            exit(EXIT_FAILURE);
        }

        print_status(command_line, status);
    }
}
