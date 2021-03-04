#ifndef SHELLUTILS_H
#define SHELLUTILS_H

/** @file shellutils.h
 *
 *  @brief Helper functions for shell implementations
 */

/**
 * @brief The COMMAND structure contains a parsed command.
 * For execution of commands inside the shell it is necessary
 * to know things like commandname, list of parameters 
 * or if the command should be executed in background.
 * To make it easier to access this necessary information 
 * the command line should be parsed into this command structure.
 */
typedef struct {
	int    background; /*!< 1 if the command shall be executed in the background, 0 otherwise */
	char     *outFile; /*!< if redirection of stdout is specified, outfile points to the filename of the output file, NULL otherwise */
	char  *parseError; /*!< points to an error message if the command line could not successfully be parsed, NULL if the parse was successful */ 
	char     *cmdLine; /*!< un-tokenized commandline (with & and > if present) */
	char       **argv; /*!< array of the tokenized command - not including &, > or outFile */
} COMMAND;

/**
 * @brief Parses a command line.
 *
 * This function parses a command line string that may contain & (for
 * background execution) and > for stdout redirection.  parseCommand will
 * generate the argv array for the given commandline and return a COMMAND
 * structure containing the result. The COMMAND structure is dynamically
 * allocated and needs to be free'd by the caller (i.e., YOU).
 *
 *  @param   cmdLine the cmdLine to be parsed. cmdLine is tokenized during parseCommandLine.
 *  @return  pointer to the parsed COMMAND or NULL on error, with errno set appropriately. In case a custom error message is supplied, the function returns a COMMAND structure that has the parseError field set to a custom error message. If the parseError field is set to a value other than null, parsing the command line has failed and the remaining fields of the COMMAND structure must not be interpreted.
 *
 */
COMMAND* parseCommandLine(char *cmdLine);

/**
 * @brief Prints the status of a terminated or stopped child process.
 *
 * @param cmdLine command line of the child process
 * @param status  process status as returned by wait(2)
 */
void printStat(const char *cmdLine, int status);

/**
 * @brief Prints a prompt symbol including the working directory.
 */
void prompt();

#endif

