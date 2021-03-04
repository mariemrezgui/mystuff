#ifndef SHELLUTILS_H
#define SHELLUTILS_H

/** @file shellutils.h
 *
 *  @brief Helper functions for shell implementations
 */

/**
 * @brief maximum number of characters in command line
 */
#ifndef COMMAND_LINE_MAXIMUM
#define COMMAND_LINE_MAXIMUM (sysconf(_SC_LINE_MAX))
#endif


/**
 * @brief The command_t structure contains a parsed command.
 */
typedef struct
{
    int background; // 1 if the command shall be executed in the background, 0 otherwise

    char* outFile;       // if redirection of stdout is specified, outfile points to the filename of the output file,
                         // NULL otherwise
    char* parse_error;   // points to an error message if the command line could not successfully be parsed,
                         // NULL if the parse was successful
    char*  command_line; // un-tokenized commandline (with & and > if present)
    char** argv;         // array of the command - not including &, > or outFile
} command_t;

/**
 * @brief Parses a command line.
 *
 * This function parses a command line string that may contain & (for
 * background execution) and > for stdout redirection.  parseCommand will
 * generate the argv array for the given commandline and return a command_t
 * structure containing the result. The command_t structure is dynamically
 * allocated and needs to be free'd by the caller (i.e., YOU).
 *
 * @param  command_line the command_line to be parsed. cmdLine is tokenized during parse_command_line.
 * @return pointer to the parsed command_t or NULL on error, with errno set appropriately. In case a custom error
 * message is supplied, the function returns a COMMAND structure that has the parseError field set to a custom error
 * message. If the parseError field is set to a value other than null, parsing the command line has failed and the
 * remaining fields of the COMMAND structure must not be interpreted.
 *
 */
command_t* parse_command_line(char* command_line);

/**
 * @brief Prints the status of a terminated or stopped child process.
 *
 * @param command_line command line of the child process
 * @param status  process status as returned by wait(2)
 */
void print_status(const char* command_line, int status);

/**
 * @brief Prints a prompt symbol including the working directory.
 */
void prompt();

#endif
