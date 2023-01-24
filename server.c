/***************************** FILE HEADER *********************************/
/*!
* \file server.c
*
* \brief Written so that the author will learn to use POSIX message queue.
*        This server keeps track of the token requested by the clients using
*        the "db" file. each token has a time to live after wich it is free.
*
* \author Mihnea SERBAN \n
*
* \version 1.0 13.01.2023 Mihnea SERBAN created
* \version 1.1 23.01.2023 Mihnea SERBAN modified
*
*//**************************** FILE HEADER *********************************/


#include <stdio.h>
#include <stdlib.h>         /* For rand() */
#include <fcntl.h>          /* For O_* constants and fnctl*/
#include <sys/stat.h>       /* For mode constants and struct stat*/
#include <unistd.h>         /* For sleep */
#include <mqueue.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>         /* For int64_t */
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>
#include "utils.h"
#include "constants.h"
#include "common.h"

#define OPEN_BUF_LEN 4096
#define WORKERS_NO 12

typedef struct {
    pid_t owner;
    time_t aq_time;
} db_entry_t;

typedef struct {
    db_entry_t entry;
    uint8_t token;
    uint8_t pseudo_port;
    int db_fd;
    pthread_mutex_t *db_mutex;
} th_info_t;

static int open_database(int *fd);
static int write_tok_info(int fd, uint16_t token, db_entry_t entry);
static off_t get_offset(uint16_t token);
static void *th_f(void* args);


static const char k_db_magic_no[] = {0x4E, 0x41, 0x4E, 0x4F, 0x44, 0x42, 0x00, 0x01};

static off_t get_offset(uint16_t token)
{
    return sizeof(k_db_magic_no) + token*sizeof(db_entry_t);
}

static int open_database(int *fd) {
    struct stat db_st;
    int db_fd;
    int flags;
    int rc;
    const int open_flags = O_CREAT | O_NONBLOCK | O_NOFOLLOW | O_RDWR;
    const mode_t open_mode = MQ_MODE;
    bool discard_content = false;
    static_assert(sizeof(db_entry_t) <= OPEN_BUF_LEN);
    char buf[OPEN_BUF_LEN];
    ssize_t chr_no = 0;

    db_fd = open(DATABASE_NAME, open_flags, open_mode);
    if (-1 == db_fd)
    {
        handle_error();
    }
    rc = fstat(db_fd, &db_st);
    if (-1 == rc)
    {
        handle_error();
    }
    if (!S_ISREG(db_st.st_mode))
    {
        fprintf(stderr, "%s:%d ./" DATABASE_NAME " is not a regular file\n", __FILE__, __LINE__);
        exit(1);
    }

    /* discard O_NONBLOCK */
    flags = fcntl(db_fd, F_GETFL);
    if (-1 == flags)
    {
        handle_error();
    }
    rc = fcntl(db_fd, F_SETFL, flags & ~O_NONBLOCK);
    if (-1 == rc)
    {
        handle_error();
    }

    /* If file has the wrong size */
    static_assert(sizeof(db_entry_t) < SSIZE_MAX,
            "sizeof(db_entry_t) is way to large\n");
    if (db_st.st_size != get_offset(DB_MAX_TOK) + (ssize_t)sizeof(db_entry_t))
    {
        discard_content = true;
    }
    else
    {
        chr_no = read(db_fd, buf, sizeof(k_db_magic_no));
        if (-1 == chr_no)
        {
            handle_error();
        }

        static_assert(sizeof(k_db_magic_no) == sizeof(uint64_t),
                "k_db_magic_no has a different size from uint64_t\n");
        /* Treat magic number as uint64_t for comparison */
        const void *magic_no = k_db_magic_no;
        void *read_no = buf;
        if (*(const uint64_t*)magic_no != *(uint64_t*)read_no)
        {
            discard_content = true;
        }
    }

    if (discard_content)
    {
        rc = close(db_fd);
        if (-1 == rc)
        {
            handle_error();
        }
        db_fd = open(DATABASE_NAME, open_flags | O_TRUNC, open_mode);
        if (-1 == db_fd)
        {
            handle_error();
        }
        /* discard O_NONBLOCK */
        flags = fcntl(db_fd, F_GETFL);
        if (-1 == flags)
        {
            handle_error();
        }
        rc = fcntl(db_fd, F_SETFL, flags & ~O_NONBLOCK);
        if (-1 == rc)
        {
            handle_error();
        }
        chr_no = write(db_fd, k_db_magic_no, sizeof(k_db_magic_no));
        if (-1 == chr_no)
        {
            handle_error();
        }

        /* complete all entries with 0 */
        off_t first_entry = get_offset(0);
        static_assert(sizeof(int64_t) >= sizeof(off_t));
        uint64_t remaining_chrs = (uint64_t)(get_offset(DB_MAX_TOK) + sizeof(db_entry_t));
        memset(buf, 0, sizeof(buf));
        off_t seek_rc = lseek(db_fd, first_entry, SEEK_SET);
        if (-1 == seek_rc)
        {
            handle_error();
        }
        do
        {
            unsigned int chrs_to_write = 0;
            if (remaining_chrs > sizeof(buf))
            {
                chrs_to_write = sizeof(buf);
            }
            else
            {
                chrs_to_write = remaining_chrs;
            }
            chr_no = write(db_fd, buf, chrs_to_write);
            if (-1 == chr_no)
            {
                handle_error();
            }
            remaining_chrs -= chr_no;
        } while(remaining_chrs > 0);
        rc = fsync(db_fd);
        if (rc != 0)
        {
            handle_error();
        }
    }

    /* return db_fd through fd */
    *fd = db_fd;
    /* return success */
    return 0;
}

static int write_tok_info(int fd, uint16_t token, db_entry_t entry)
{
    off_t off = get_offset(token);
    int rc;
    off_t seek_rc;
    db_entry_t old_entry;
    seek_rc = lseek(fd, off, SEEK_SET);
    if (-1 == seek_rc)
    {
        handle_error();
    }
    rc = read(fd, &old_entry, sizeof(entry));
    if (-1 == rc)
    {
        handle_error();
    }

    errno = 0;
    time_t current_time = time(NULL);
    if (-1 == current_time)
    {
        handle_error();
    }

    if (old_entry.owner != 0 &&
        old_entry.owner != entry.owner &&
        old_entry.aq_time + DB_ENTRY_TTL > current_time)
    {
        return TOKEN_NOT_AVAILABLE;
    }

    seek_rc = lseek(fd, off, SEEK_SET);
    if (-1 == seek_rc)
    {
        handle_error();
    }
    rc = write(fd, &entry, sizeof(entry));
    if (-1 == rc)
    {
        handle_error();
    }
    rc = fsync(fd);
    if (rc != 0)
    {
        handle_error();
    }
    return 0;
}

static void *th_f(void* args)
{
    th_info_t *info = args;
    db_entry_t entry = info->entry;
    uint16_t token_requested = info->token;
    uint8_t pseudo_port = info->pseudo_port;
    int db_fd = info->db_fd;
    pthread_mutex_t *db_mutex = info->db_mutex;
    char client_mq_name[NAME_MAX] = {0};
    mqd_t client_mq;
    response_msg_t response_msg = {0};
    unsigned int msg_prio = MQ_DEFAULT_PRIO;
    int rc;

    /* Cache time. */
    time_t current_time = time(NULL);
    if (-1 == current_time)
    {
        handle_error_en(0);
    }

    /* Open mqueue specified by the client. */
    rc = get_client_mq_name(client_mq_name, sizeof(client_mq_name), pseudo_port);
    if (rc < 0 || (unsigned int)rc > sizeof(client_mq_name))
    {
        handle_error();
    }
    client_mq = mq_open(client_mq_name, O_WRONLY);
    if (-1 == client_mq)
    {
        handle_error();
    }

    /* Attempt to reserve the tokken. */
    rc = pthread_mutex_lock(db_mutex);
    if (rc != 0)
    {
        handle_error_en(rc);
    }
    int write_result = write_tok_info(db_fd, token_requested, entry);
    rc = pthread_mutex_unlock(db_mutex);
    if (rc != 0)
    {
        handle_error_en(rc);
    }

    /* Send results to the client. */
    response_msg.resp_type = write_result;
    response_msg.token_requested = token_requested;
    response_msg.pid = entry.owner;

    /* TODO Should verify with preprocessor directives or static assert if time_t is on 64 bits */
    struct timespec wait_time = {.tv_sec = current_time + DB_ENTRY_TTL, .tv_nsec = 0};
    switch (write_result)
    {
        case ACK:
        printf("Server responding to TOKEN request token:%3d; pid:%5d; with ACK\n",
                token_requested, entry.owner);
        break;
        case TOKEN_NOT_AVAILABLE:
        printf("Server responding to TOKEN request token:%3d; pid:%5d; with TOKEN_NOT_AVAILABLE.\n",
                token_requested, entry.owner);
        break;
        default:
        printf("Server Responding to TOKEN request token:%3d; pid:%5d; with unkown response.\n",
                token_requested, entry.owner);
    }
    rc = mq_timedsend(client_mq, (char*)&response_msg, sizeof(response_msg), msg_prio, &wait_time);
    if (-1 == rc)
    {
        if (ETIMEDOUT == errno)
        {
            printf("Server response to TOKEN request token:%3d; pid:%5d; timed out.\n",
                    token_requested, entry.owner);
        }
        else
        {
            handle_error();
        }
    }
    printf("Server responded to TOKEN request token:%3d; pid:%5d; succesfully.\n",
            token_requested, entry.owner);

    /* Unlink the queue. */
    rc = mq_close(client_mq);
    if (-1 == rc)
    {
        handle_error();
    }
    return NULL;
}

int main ()
{
    printf("Starting the server.\n");

    int rc = 0;
    struct mq_attr qattr = {0};
    qattr.mq_maxmsg = MQ_MAXMSG;
    qattr.mq_msgsize = MQ_MSGSIZE;
    int db_fd;
    ssize_t read_bytes = 0;
    bool shall_close = false;
    unsigned int prio;
    db_entry_t db_entry = {0};
    th_info_t th_infos[WORKERS_NO];
    pthread_t th_ids[WORKERS_NO];
    int last_worker = -1;
    int max_worker_no = -1;
    pthread_mutex_t db_mutex;
    request_msg_t request;
    char buf[MQ_MSGSIZE + 1];

    mqd_t server_mq;
    server_mq = mq_open(MQ_REQ_NAME, O_RDONLY | O_CREAT, MQ_MODE, &qattr);
    if (-1 == server_mq)
    {
        handle_error();
    }
    rc = open_database(&db_fd);
    if (rc != 0)
    {
        handle_error_en(0);
    }
    rc = pthread_mutex_init(&db_mutex, NULL);
    if (rc != 0)
    {
        handle_error_en(rc);
    }
    printf("The server is ready to recieve requests.\n");

    do
    {
        read_bytes = mq_receive(server_mq, buf, sizeof(buf), &prio);
        if (read_bytes == -1)
        {
            handle_error();
        }
        if (read_bytes != sizeof(request))
        {
            printf("Server reciceved an aunkown request\n");
            continue;
        }
        memcpy(&request, buf, sizeof(request));

        switch(request.req_type)
        {
            case TOKEN:
                printf("Server reciceved a TOKEN request "
                        "token:%3d; pid:%5d;\n",
                        request.token_requested, request.pid);
                db_entry.owner = request.pid;
                db_entry.aq_time = request.req_time;
                /* use worker to work on database and send result to client*/
                if (max_worker_no < WORKERS_NO)
                {
                    max_worker_no++;
                    last_worker++;
                } else {
                    last_worker = (last_worker+1) % WORKERS_NO;
                    pthread_join(th_ids[last_worker], NULL);
                }
                th_infos[last_worker].entry = db_entry;
                th_infos[last_worker].token = request.token_requested;
                th_infos[last_worker].pseudo_port = request.pseudo_port;
                th_infos[last_worker].db_fd = db_fd;
                th_infos[last_worker].db_mutex = &db_mutex;
                rc = pthread_create(&th_ids[last_worker], NULL, th_f, &th_infos[last_worker]);
                if (rc != 0)
                {
                    handle_error_en(rc);
                }
            break;
            case CLOSE:
                /* TODO Probably not the safest way to close. */
                printf("Server reciceved a CLOSE request\n");
                shall_close = true;
            break;
            default:
                printf("Server reciceved an aunkown request\n");
        }
    } while(shall_close != true);

    printf("Server is closing.\n");
    for (int i = 0; i < max_worker_no; i++)
    {
        rc = pthread_join(th_ids[i], NULL);
        if (0 != rc)
        {
            handle_error_en(rc);
        }
    }
    printf("Server's workers have been closed\n");

    rc = mq_unlink(MQ_REQ_NAME);
    if (-1 == rc)
    {
        handle_error();
    }
    printf("Message queue deleted.\n");

    rc = close(db_fd);
    if (-1 == rc)
    {
        handle_error();
    }

    printf("Server closed.\n");
    return 0;
}
