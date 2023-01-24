/***************************** FILE HEADER *********************************/
/*!
* \file common.h
*
* \brief Simple header with common definitions for both the client and the server.
*
* \author Mihnea SERBAN \n
*
* \version 1.0 13.01.2023 Mihnea SERBAN created
* \version 1.1 25.01.2023 Mihnea SERBAN modified
*
*//**************************** FILE HEADER *********************************/

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>  /* For pid_t */

/*
*******************************************************************************
*   RESP_TYPE
*******************************************************************************
*
*  \brief           <b> RESP_TYPE </b>\n
*                   This enum is used for the types of responses form the
*                   server
*
*  \author          <Mihnea SERBAN>
*
*  \date            <25.01.2023>
*******************************************************************************/
typedef enum {
    ACK,
    TOKEN_NOT_AVAILABLE
} RESP_TYPE;

/*
*******************************************************************************
*   REQ_TYPE
*******************************************************************************
*
*  \brief           <b> REQ_TYPE </b>\n
*                   This enum is used for the types of requests to the server
*
*  \author          <Mihnea SERBAN>
*
*  \date            <25.01.2023>
*******************************************************************************/
typedef enum {
    TOKEN,
    CLOSE
} REQ_TYPE;

/*
*******************************************************************************
*   request_msg_t
*******************************************************************************
*
*  \brief           <b> request_msg_t </b>\n
*                   This struct is used to send requests to the server
*
*  \var             req_type                          The type of request. Must
*                                                     be from enum REQ_TYPE.
*
*  \var             token_requested                   Token requested by the
*                                                     client.
*
*  \var             pdi_t pid                         Pid of the client.
*
*  \var             uint8_t pseudo_port               Number used to create the
*                                                     name of the message queue
*                                                     for the response.
*
*  \author          <Mihnea SERBAN>
*
*  \date            <25.01.2023>
*******************************************************************************/
typedef struct
{
    int req_type;
    uint16_t token_requested;
    pid_t pid;
    uint8_t pseudo_port;
    time_t req_time;
} request_msg_t;

/*
*******************************************************************************
*   response_msg_t
*******************************************************************************
*
*  \brief           <b> response_msg_t </b>\n
*                   This struct is used to send response from the server
*
*  \var             resp_type                         The type of response.
*                                                     Must be from enum
*                                                     RESP_TYPE.
*
*  \var             token_requested                   Token requested by the
*                                                     client.
*
*  \var             pdi_t pid                         Pid of the client that
*                                                     requested the token.
*
*
*  \author          <Mihnea SERBAN>
*
*  \date            <25.01.2023>
*******************************************************************************/
typedef struct
{
    int resp_type;
    uint16_t token_requested;
    pid_t pid;

} response_msg_t;


/*
*******************************************************************************
*   get_clinet_mq_name
*******************************************************************************
*
*  \brief           <b> get_clinet_mq_name </b>\n
*                   This function fills buf with the apropriate name for the
*                   message queue. If buf_len is too small, output is
*                   truncated. If function succedes, it guarantees a NULL
*                   terminated string.
*
*  \param[out]      char *buf             Buffer which will store the name.
*
*  \param[in]       size_t buf_len        Length of the buffer. Must include
*                                         space for the null character.
*
*  \param[in]       int pseudo_port       Number chosen by the client to
*                                         differentiate message queues. must be
*                                         0 or greater and less then 100.
*
*  \return          negative values       In case of error. The underlying
*                                         functions failing might set errno.
*
*  \return          number of             Success
*                   characters that
*                   would have been
*                   printed as if
*                   buf_len were
*                   high enough
*
*  \author          Mihnea SERBAN
*
*  \date            25.01.2023
*******************************************************************************/
int get_client_mq_name(char *buf, size_t buf_len, uint8_t pseudo_port);

#endif /* COMMON_H */
