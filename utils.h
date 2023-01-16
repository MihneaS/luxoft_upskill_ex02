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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "constants.h"

/* Asserts the XSI-compliant version  of strerror_r is provided 
 * see https://linux.die.net/man/3/strerror_r */
#define handle_error_en(en) \
    do \
    {\
        char __define_buf__[SYSERR_MSGSIZE] = {0};\
        int __define_rc__ = strerror_r(en, __define_buf__, sizeof(__define_buf__));\
        if (0 == __define_rc__)\
        {\
            fprintf(stderr, "%s:%d error:%s\n", __FILE__, __LINE__, __define_buf__);\
        }\
        else\
        {\
            fprintf(stderr, "%s:%d\n\n", __FILE__, __LINE__);\
            fprintf(stderr, "Calling strerror_r(%d, __define_buf__, %ld) returned with error %d."\
                    " See utils.h\n",\
                    en, sizeof(__define_buf__), __define_rc__);\
        }\
        exit(1);\
    } while(0)

#define handle_error() do {handle_error_en(errno);} while(0)

#endif // UTILS_H

