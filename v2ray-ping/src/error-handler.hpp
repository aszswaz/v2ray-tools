#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <cstring>
#include <errno.h>
#include <string>
#include <libgen.h>

/**
 * Encapsulate error messages as exceptions.
 */
#define MESSAGE_ERROR(msg) \
    message_error(__FILE_NAME__, __LINE__, msg);

/**
 * Standard exception handling, usually exceptions generated by system calls.
 */
#define STAND_ERROR(experssion) \
    if (experssion) { \
        message_error(__FILE_NAME__, __LINE__, strerror(errno)); \
    }

void message_error(const char *filename, int line, std::string msg);

#endif