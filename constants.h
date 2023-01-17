/***************************** FILE HEADER *********************************/
/*!
* \file constants.h
*
* \brief Written so that the author will learn to use POSIX message queue. Contains the constants and literals
*
* \author Mihnea SERBAN \n
* Copyright 2011 Hirschmann Automation and Control GmbH
*
* \version 1.0 13.01.2023 Mihnea SERBAN created
*
*//**************************** FILE HEADER *********************************/


#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MQ_NAME "/server_mq"
#define MQ_MAXMSG 10
#define MQ_MSGSIZE 2048

#define SERVER_PROD_VAL_MIN 12
#define SERVER_PROD_VAL_MAX 42
#define SERVER_PROD_WAIT_MIN 0
#define SERVER_PROD_WAIT_MAX 3
#define SERVER_PROD_RUN_NO 10
#define SERVER_PROD_WORKERS_NO 12
#define MQ_DEFAULT_PRIO 10;

#define CLIENT_CONSUME_RUN_NO 20
#define CLIENT_CONSUME_WAIT_MIN 0
#define CLIENT_CONSUME_WAIT_MAX 3

#endif /* CONSTANTS_H */
