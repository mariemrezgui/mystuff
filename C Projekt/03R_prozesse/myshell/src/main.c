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

int main()
{
    char command_line[COMMAND_LINE_MAXIMUM] ;
    while(1)
    {   
        collect_defunct_process();
        read_input(command_line, COMMAND_LINE_MAXIMUM, stdin);
        execute_command(command_line);
    }
}
