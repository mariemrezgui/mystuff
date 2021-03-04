#include "shellutils.h"
#include <stdio.h>
#include <stdlib.h>

int main () 
{

	char commandLineArr[] = "ls -l -a >ls.log &";

	COMMAND *p_cmd = parseCommandLine(commandLineArr);


	printf("CMD:\n");
	printf(" background: %d\n", p_cmd->background);
	printf(" outFile: %s\n",    p_cmd->outFile);
	printf(" parseError: %s\n", p_cmd->parseError);
	printf(" cmdLine: %s\n",    p_cmd->cmdLine);
	for (int i=0; p_cmd->argv[i]; i++)
	{
		printf("  argv[%d]: %s\n",i,p_cmd->argv[i]);
	}
	free(p_cmd);

}
