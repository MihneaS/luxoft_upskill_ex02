/***************************** FILE HEADER *********************************/
/*!
* \file common.c
*
* \brief Implements functions common for both the client and the server.
*
* \author Mihnea SERBAN \n
*
* \version 1.0 20.01.2023 Mihnea SERBAN created
* \version 1.1 23.01.2023 Mihnea SERBAN modified
*
*//**************************** FILE HEADER *********************************/


#include "common.h"
#include <stdio.h>

int get_client_mq_name(char *buf, size_t buf_len, uint8_t pseudo_port)
{
    int rc =  snprintf(buf, buf_len, "/client_%d", pseudo_port);
    if (rc >= 0 && (unsigned int)rc >= buf_len)
    {
        buf[buf_len-1] = '\0';
    }
    return rc;
}
