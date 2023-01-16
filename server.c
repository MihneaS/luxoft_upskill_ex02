/***************************** FILE HEADER *********************************/
/*!
* \file server.c
*
* \brief Written so that the author will learn to use POSIX message queue. This server is ment to be opened before the clients and closed after the clients finished consuming.
*
* \author Mihnea SERBAN \n
* Copyright 2011 Hirschmann Automation and Control GmbH
*
* \version 1.0 13.01.2023 Mihnea SERBAN created
*
*//**************************** FILE HEADER *********************************/


#include <stdio.h>
#include <stdlib.h>         /* For rand() */
#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */
#include <unistd.h>         /* For sleep */
#include <mqueue.h>
#include <pthread.h>
#include <assert.h>
#include "utils.h"
#include "constants.h"
#include "common.h"

typedef struct
{
    unsigned int id;
    mqd_t mq;
    int val_min;
    int val_max;
    int wait_min;
    int wait_max;
    int run_no;
    unsigned int seed;
} th_info_t;

static void *th_f(void* args);
static void wait_for_input(const char * msg)

static void *th_f(void* args)
{
    th_info_t *info = args;

    for (int i = 0; i < info->run_no; i++)
    {
        int rc;
        unsigned int msg_prio = MQ_DEFAULT_PRIO;
        ticket_msg_t msg = {0}; /* assert that sending operation is blocking */
        char buf[MQ_MSGSIZE] = {0};

        msg.val = rand_r(&info->seed) % (info->val_max - info->val_min) + info->val_min;

        printf("th%2d: produced %3d\n",info->id, msg.val);
        static_assert(sizeof(msg) <= MQ_MAXMSG,
                "real size of message struct is smaller then defined size of"
                "the message in the mqueue(MQ_MAXMSG);\n");

        memcpy(buf, &msg, sizeof(msg)); /* nu e inutil? */

        rc = mq_send(info->mq, buf, sizeof(msg), msg_prio);
        if (-1 == rc)
        {
            handle_error();
        }
        sleep(rand_r(&info->seed) % (info->wait_max - info->wait_min) + info->wait_min);
    }

    return NULL;
}

/* Function copeid from https://github.com/systems-cs-pub-ro/so/blob/master/labs/lab06/skel/lin/1-intro/intro.c
*/
static void wait_for_input(const char * msg)
{
    char buf[32];

    printf(" %s\n", msg);
    printf(" -- Press ENTER to continue ...\n");
    fflush(stdout);
    fgets(buf, 32, stdin);
}

int main ()
{
    printf(" Starting the server.\n");

    unsigned int rand_seed = 42;
    int rc = 0;
    pthread_t th_ids[SERVER_PROD_WORKERS_NO];
    struct mq_attr qattr = {0};
    qattr.mq_maxmsg = MQ_MAXMSG;
    qattr.mq_msgsize = MQ_MSGSIZE;

    mqd_t mq;
    // open and delete to clear queue
    mq = mq_open(MQ_NAME, O_WRONLY | O_CREAT, 0660, &qattr);
    if (-1 == mq)
    {
        handle_error();
    }
    rc = mq_unlink(MQ_NAME);
    if (-1 == rc)
    {
        handle_error();
    }

    mq = mq_open(MQ_NAME, O_WRONLY | O_CREAT, 0660, &qattr);
    if (-1 == mq)
    {
        handle_error();
    }
    printf(" Oppened message queue.\n");


    th_info_t th_info[SERVER_PROD_WORKERS_NO] = {0};
    for (int i = 0; i < SERVER_PROD_WORKERS_NO; i++)
    {
        th_info[i].id = i;
        th_info[i].mq = mq;
        th_info[i].val_min = 100*i + SERVER_PROD_VAL_MIN;
        th_info[i].val_max = 100*i + SERVER_PROD_VAL_MAX;
        th_info[i].wait_min = SERVER_PROD_WAIT_MIN;
        th_info[i].wait_max = SERVER_PROD_WAIT_MAX;
        th_info[i].run_no = SERVER_PROD_RUN_NO;
        th_info[i].seed = rand_r(&rand_seed);
    }

    wait_for_input("May start workers.");
    printf(" Starting workers.\n");
    for (int i = 0; i < SERVER_PROD_WORKERS_NO; i++)
    {
        rc = pthread_create(&th_ids[i], NULL, th_f, &th_info[i]);
        if (0 != rc)
        {
            handle_error_en(rc);
        }
    }

    printf("Workers started.\n");

    for (int i = 0; i < SERVER_PROD_WORKERS_NO; i++)
    {
        rc = pthread_join(th_ids[i], NULL);
        if (0 != rc)
        {
            handle_error_en(rc);
        }
    }

    wait_for_input("May close the server after the clients are done.");
    rc = mq_unlink(MQ_NAME);
    if (-1 == rc)
    {
        handle_error();
    }
    printf("Message queue deleted.\n");

    printf("Server closed.\n");
    return 0;
}
