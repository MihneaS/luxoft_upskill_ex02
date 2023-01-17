/***************************** FILE HEADER *********************************/
/*!
* \file utils.h
*
* \brief utility header with macros for error handling
*
* \author Mihnea SERBAN \n
* Copyright 2011 Hirschmann Automation and Control GmbH
*
* \version 1.0 6.01.2023 Mihnea SERBAN created
*
*//**************************** FILE HEADER *********************************/

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <errno.h>

/* Asserts the XSI-compliant version  of strerror_r is provided 
 * see https://linux.die.net/man/3/strerror_r */
#define handle_error_en(en) \
    do \
    {\
        fprintf(stderr, "%s:%d ", __FILE__, __LINE__);\
        int last_errno = errno;\
        errno = en;\
        perror(NULL);\
        errno = last_errno;\
        exit(1);\
    } while(0)

#define handle_error() do {handle_error_en(errno);} while(0)

void handle_error_explicit(int en, const char *file, int line);

#endif // UTILS_H

