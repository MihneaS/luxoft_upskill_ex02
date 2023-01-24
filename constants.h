/***************************** FILE HEADER *********************************/
/*!
* \file constants.h
*
* \brief Contains the constants and literals.
*
* \author Mihnea SERBAN \n
*
* \version 1.0 13.01.2023 Mihnea SERBAN created
* \version 1.1 23.01.2023 Mihnea SERBAN modified
*
*//**************************** FILE HEADER *********************************/


#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MQ_REQ_NAME "/server_requests"
#define DATABASE_NAME "db"
#define MAX_MQUEUE_NAME 64 /**< It includes the NULL charater */
#define MQ_MAXMSG 10
#define MQ_MSGSIZE 2048
#define MQ_MODE 0660

#define SERVER_PROD_VAL_MIN 12
#define SERVER_PROD_VAL_MAX 42
#define SERVER_PROD_WAIT_MIN 0
#define SERVER_PROD_WAIT_MAX 3
#define SERVER_PROD_RUN_NO 10
#define SERVER_PROD_WORKERS_NO 12
#define MQ_DEFAULT_PRIO 10

#define CLIENT_MAX_TOK 20

#define CLIENT_CONSUME_RUN_NO 20
#define CLIENT_CONSUME_WORKERS_NO 6
#define CLIENT_CONSUME_WAIT_MIN 0
#define CLIENT_CONSUME_WAIT_MAX 3

#define DB_MAX_TOK 65535
#define DB_ENTRY_TTL 10 /**< This is in seconds */

#endif /* CONSTANTS_H */
