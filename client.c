/***************************** FILE HEADER *********************************/
/*!
* \file client.c
*
* \brief Written so that the author will learn to use POSIX message queue.
*        Starts several processes that will requests tokens from the server and
*        then start one final that will ask the server to shut down.
*
* \author Mihnea SERBAN \n
*
* \version 1.0 16.01.2023 Mihnea SERBAN created
* \version 1.1 23.01.2023 Mihnea SERBAN modified
*
*//**************************** FILE HEADER *********************************/


#include <stdio.h>
#include <stdlib.h>         /* For rand() */
#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */
#include <sys/wait.h>       /* for waitpid */
#include <unistd.h>         /* For sleep and getpid*/
#include <mqueue.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "constants.h"
#include "common.h"

static void do_work(uint8_t pseudo_port);
static void send_close_server_msg();

void do_work(uint8_t pseudo_port)
{
    int rc = 0;
    pid_t pid = getpid();
    char client_mq_name[MAX_MQUEUE_NAME] = {0};
    mqd_t client_mq;
    mqd_t server_mq;
    struct mq_attr qattr = {0};
    qattr.mq_maxmsg = MQ_MAXMSG;
    qattr.mq_msgsize = MQ_MSGSIZE;
    unsigned int seed = (unsigned int)pid; /**< Detailes of the conversion does not matter. */
    unsigned int msg_prio = MQ_DEFAULT_PRIO;

    printf("%5d_client: Starting.\n", pid);
    int wait_max = CLIENT_CONSUME_WAIT_MAX;
    int wait_min = CLIENT_CONSUME_WAIT_MIN;

    rc = get_client_mq_name(client_mq_name, sizeof(client_mq_name), pseudo_port);
    if (rc < 0 || (unsigned int)rc > sizeof(client_mq_name))
    {
        handle_error();
    }
    client_mq = mq_open(client_mq_name, O_RDONLY | O_CREAT, MQ_MODE, &qattr);
    if (-1 == client_mq)
    {
        handle_error();
    }
    for (int i = 0; i < CLIENT_CONSUME_RUN_NO; i++)
    {
        printf("%5d_client: Run %d.\n", pid, i);
        sleep(rand_r(&seed) % (wait_max - wait_min) + wait_min);

        request_msg_t request = {0};
        request.req_type = TOKEN;
        request.token_requested = rand_r(&seed) % (CLIENT_MAX_TOK+1);
        request.pid = pid;
        request.pseudo_port = pseudo_port;
        request.req_time = time(NULL);
        response_msg_t response;
        char buf[MQ_MSGSIZE + 1];
        unsigned int resp_prio = 0;
        bool discard_msg;
        if (-1 == request.req_time)
        {
            handle_error();
        }
        server_mq = mq_open(MQ_REQ_NAME, O_WRONLY | O_CREAT, MQ_MODE, &qattr);
        if (-1 == server_mq)
        {
            handle_error();
        }
        printf("%5d_client: Requesting %3d.\n", pid, request.token_requested);
        rc = mq_send(server_mq, (char*)&request, sizeof(request), msg_prio);
        if (-1 == rc)
        {
            handle_error();
        }
        rc = mq_close(server_mq);
        if (-1 == rc)
        {
            handle_error();
        }
        do
        {
            discard_msg = false;
            rc = mq_receive(client_mq, buf, sizeof(buf), &resp_prio);
            if (-1 == rc)
            {
                handle_error();
            }
            if (rc != sizeof(response)){
                discard_msg = true;
                continue;
            }
            memcpy(&response, buf, sizeof(response));
            if (response.pid != pid || response.token_requested != request.token_requested)
            {
                printf("%5d_client: Discarding a message.\n", pid);
                discard_msg = true;
            }
        } while(discard_msg == true);
        switch(response.resp_type)
        {
            case ACK:
                printf("%5d_client: Received token %3d.\n", pid, response.token_requested);
            break;
            case TOKEN_NOT_AVAILABLE:
                printf("%5d_client: Token %3d not available.\n", pid, response.token_requested);
            break;
            default:
                printf("%5d_client: Received unkown response.\n", pid);
        }
    }
    rc = mq_unlink(client_mq_name);
    if (-1 == rc)
    {
        handle_error();
    }
    printf("%5d_client: Closing.\n", pid);
}

static void send_close_server_msg()
{
    int rc = 0;
    pid_t pid = getpid();
    mqd_t server_mq;
    struct mq_attr qattr = {0};
    qattr.mq_maxmsg = MQ_MAXMSG;
    qattr.mq_msgsize = MQ_MSGSIZE;
    unsigned int msg_prio = MQ_DEFAULT_PRIO;

    printf("%5d_client: Starting. This client will close the server.\n", pid);

    server_mq = mq_open(MQ_REQ_NAME, O_WRONLY | O_CREAT, MQ_MODE, &qattr);
    if (-1 == server_mq)
    {
        handle_error();
    }
    request_msg_t request = {0};
    request.req_type = CLOSE;
    request.token_requested = 0;
    request.pid = pid;
    request.pseudo_port = 0;
    request.req_time = time(NULL);
    if (-1 == request.req_time)
    {
        handle_error();
    }
    printf("%5d_client: Requesting server closing.\n", pid);
    rc = mq_send(server_mq, (char*)&request, sizeof(request), msg_prio);
    if (-1 == rc)
    {
        handle_error();
    }
}

int main()
{
    int first_pseudo_port = 100;
    pid_t children[CLIENT_CONSUME_RUN_NO];
    for (int i = 0; i < CLIENT_CONSUME_WORKERS_NO; i++)
    {
        children[i] = fork();
        if (0 == children[i])
        {
            do_work(first_pseudo_port + i);
            exit(0);
        }
        else if (-1 == children[i])
        {
            handle_error();
        }
    }

    for (int i = 0; i < CLIENT_CONSUME_WORKERS_NO; i++)
    {
        int status;
        pid_t rc;
        rc = waitpid(children[i], &status, 0);
        if (-1 == rc)
        {
            handle_error();
        }
    }

    send_close_server_msg();

    return 0;
}
