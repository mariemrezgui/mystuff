#ifndef PLIST_H
#define PLIST_H

#include <sys/types.h>

/** \file plist.h
 *
 *  \brief Linked List for maintaining process id - command line pairs.
 *
 *  This implementation is not thread safe.
 */

/**
 *  \brief Inserts a new pid-command line pair into the linked list.
 *
 * During the insert operation, the passed command_line is copied to
 * an internally allocated buffer. The caller may free or otherwise
 * reuse the memory occupied by command_line after return from
 * insertElement.
 *
 *  \param process_id The process id of the pair that is to be inserted.
 *  \param command_line The command_line corresponding to the process with id process_id.
 *
 *  \return process_id on success, negative value on error
 *    \retval process_id  success
 *    \retval  -1  a pair with the given process_id already exists
 *    \retval  -2  insufficient memory to complete the operation
 */
int insert_element(pid_t process_id, const char* command_line);

/**
 *  \brief Remove a specific pid-command line pair from the linked list.
 *
 * The linked list is searched for a pair with the given process_id. If such a pair is
 * found, the '\\0'-terminated command line is copied to the buffer provided by
 * the caller. If the length of the command line exceeds the size of the
 * buffer, only the first (buffersize-1) characters of the command line are
 * copied to the buffer and terminated with the '\\0' character. Upon
 * completion, removeElement deallocates all resources that were occupied by
 * the removed pair.
 *
 *  \param process_id The process id of the pair that is to be removed.
 *  \param command_line_buffer A buffer provided by the caller that the '\\0'-terminated commandline is written to.
 *  \param buffer_size The size of command_line_buffer.
 *
 *  \return actual length of the command line on success, negative value on error.
 *    \retval >0  success, actual length of the command line
 *    \retval -1  a pair with the given process_id does not exist
 */
int remove_element(pid_t process_id, char* command_line_buffer, size_t buffer_size);

/**
 *  \brief Invokes a callback function on each element in the list.
 *
 * 	The callback function is passed the pid-command line pair for the current
 * 	list element. The callback function shall return 0 to request further
 * 	processing of the list. Any other return value will cause the early
 * 	termination of the list walk. The callback function must not modify the
 * 	process list.
 *
 *  \param callback Pointer to the function to be invoked for each list element.
 */
void walk_list(int (*callback)(pid_t, const char*));

#endif
