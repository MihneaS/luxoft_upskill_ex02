/***************************** FILE HEADER *********************************/
/*!
* \file client.c
*
* \brief Written so that the author will learn to use POSIX message queue. This client expects to be executed after the server has been opened. It also exepects a seed as argument
*
* \author Mihnea SERBAN \n
* Copyright 2011 Hirschmann Automation and Control GmbH
*
* \version 1.0 16.01.2023 Mihnea SERBAN created
*
*//**************************** FILE HEADER *********************************/


#include <stdio.h>
#include <stdlib.h>         /* For rand() */
#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */
#include <unistd.h>         /* For sleep and getpid*/
#include <mqueue.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include "utils.h"
#include "constants.h"
#include "common.h"

static void consume();

static void consume(mqd_t mq, unsigned int seed)
{
    unsigned int wait_min = CLIENT_CONSUME_WAIT_MIN;
    unsigned int wait_max = CLIENT_CONSUME_WAIT_MAX;
    pid_t pid = getpid();

    for (int i = 0; i < CLIENT_CONSUME_RUN_NO; i++)
    {
        ticket_msg_t msg = {0}; /* assert that sending operation is blocking */
        char buf[MQ_MSGSIZE+1] = {0}; /* needs to be bigger then MAX_MSGSIZE */
        unsigned prio;
        size_t total_bytes = 0;
        ssize_t read_bytes = 0;

        static_assert(sizeof(msg) <= MQ_MAXMSG,
                "real size of message struct is smaller then defined size of"
                "the message in the mqueue(MQ_MAXMSG).\n");
        while (total_bytes < sizeof(msg))
        {
            read_bytes = mq_receive(
                    mq,
                    buf + total_bytes,
                    sizeof(buf),
                    &prio);
            if (-1 == read_bytes)
            {
                handle_error();
            }
            total_bytes += read_bytes;
        }

        memcpy(&msg, buf, sizeof(msg));

        printf("%4d consumed %3d\n",pid, msg.val);
        sleep(rand_r(&seed) % (wait_max - wait_min) + wait_min);
    }
}

int main (int argc, char** argv)
{
    int rc = 0;
    unsigned int seed = 0;
    pid_t pid = getpid();
    mqd_t mq;

    if (argc != 2)
    {
        printf("usage:\n%s <seed>\nThe seed have to be greater then 0.", argv[0]);
        exit(1);
    }

    rc = atoi(argv[1]);
    if (rc < 0)
    {
        printf("Could not convert seed to number or the seed was 0 or negative.\nusage:\n%s <seed>\n", argv[0]);
    }
    seed = rc;

    printf(" Starting the client %4d.\n", pid);

    mq = mq_open(MQ_NAME, O_RDONLY);
    if (-1 == mq)
    {
        handle_error();
    }
    printf(" Oppened message queue.\n");
    printf(" Consuming.\n");
    consume(mq, seed);

    rc = mq_close(mq);
    if (-1 == rc)
    {
        handle_error();
    }
    printf("Client %d closed.\n", pid);
    return 0;
}
