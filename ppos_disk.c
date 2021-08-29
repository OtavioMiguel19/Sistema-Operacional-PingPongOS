#include "ppos.h"
#include "ppos-core-globals.h"
#include "disk.h"
#include "queue.h"
#include "ppos_disk.h"
#include <signal.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

task_t disk_manager;
struct sigaction sig;
static disk_t disk;

void printDiskQueue();

void diskDriverBody(void *args)
{
    while (1)
    {
        sem_down(&disk.semaphore);
        if (disk.signal == 1)
        {
            task_t *requester = (task_t *)queue_remove((queue_t **)&sleepQueue, (queue_t *)disk.current_request->requester);
            queue_append((queue_t **)&readyQueue, (queue_t *)requester);
            
            free(disk.current_request);
            disk.signal = 0;
        }

        int disk_idle = disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE;

        if ((disk_idle == 1) && (disk.queue != NULL))
        {
            disk_request_t *request = (disk_request_t *)queue_remove((queue_t **)&disk.queue, (queue_t *)disk.queue);
            disk.current_request = request;
            printf("Request block -> %d \n\n\n", request->block);
            int command = disk.current_request->isRead == 1 ? DISK_CMD_READ : DISK_CMD_WRITE;
            disk_cmd(command, disk.current_request->block, disk.current_request->buffer);
        }

        sem_up(&disk.semaphore);

        task_yield();
    }
}

void handleSignal()
{
    // printf("Entrou handleSignal\n");
    sem_down(&disk.semaphore);
    disk.signal = 1;
    queue_append((queue_t **)&readyQueue, (queue_t *)&disk_manager);
    // task_switch(&disk_manager);
    sem_up(&disk.semaphore);
    // printf("Saiu handleSignal\n");
}

int setupHandler()
{
    sig.sa_handler = handleSignal;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;

    if (sigaction(SIGUSR1, &sig, 0) < 0)
    {
        return -1;
    }
    return 0;
}

int disk_mgr_init(int *numBlocks, int *blockSize)
{
    int status_request_disk_init = disk_cmd(DISK_CMD_INIT, 0, 0) == 0 ? 0 : -1;

    *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
    *numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);

    int status_request_signal_handler = setupHandler() == 0 ? 0 : -1;
    int status_request_sem_create = sem_create(&disk.semaphore, 1) == 0 ? 0 : -1;

    task_create(&disk_manager, diskDriverBody, NULL);
    disk_manager.is_user_task = 0;

#ifdef DEBUG
    // printf("\ninit - BEFORE");
#endif

    int finalReturn = (status_request_disk_init == 0 &&
                       status_request_signal_handler == 0 &&
                       status_request_sem_create == 0) == 1
                          ? 0
                          : -1;

    return finalReturn;
}

int disk_block_read(int block, void *buffer)
{
    // printf("Entrou disk_block_read\n");

    sem_down(&disk.semaphore);

    disk_request_t *request = malloc(sizeof(disk_request_t));
    request->prev = request->next = NULL;
    request->requester = taskExec;
    request->block = block;
    request->buffer = buffer;
    request->isRead = 1;

    // printf("disk.current_request->block => %d\n", block);
    // printf("disk.current_request->buffer => %p\n", buffer);

    queue_append((queue_t **)&disk.queue, (queue_t *)request);

    queue_append((queue_t **)&readyQueue, (queue_t *)&disk_manager);
    sem_up(&disk.semaphore);
    // printf("SEM UP \n");

    // printDiskQueue();

    task_suspend(taskExec, &sleepQueue);
    task_switch(taskDisp);
    // printf("Saiu disk_block_read\n");

#ifdef DEBUG
    // printf("\ninit - BEFORE");
#endif
    return 0;
}

int disk_block_write(int block, void *buffer)
{
    // printf("Entrou disk_block_read\n");

    sem_down(&disk.semaphore);

    disk_request_t *request = malloc(sizeof(disk_request_t));
    request->prev = request->next = NULL;
    request->requester = taskExec;
    request->block = block;
    request->buffer = buffer;
    request->isRead = 0;

    queue_append((queue_t **)&disk.queue, (queue_t *)request);

    queue_append((queue_t **)&readyQueue, (queue_t *)&disk_manager);
    sem_up(&disk.semaphore);
    // printf("SEM UP \n");

    // printDiskQueue();

    task_suspend(taskExec, &sleepQueue);
    task_switch(taskDisp);
    // printf("Saiu disk_block_read\n");

#ifdef DEBUG
    // printf("\ninit - BEFORE");
#endif
    return 0;
}

void printDiskQueue()
{
    disk_request_t * first = disk.queue;
    disk_request_t * val = disk.queue;
    while (val != NULL && val->next != NULL && first != (disk_request_t *) val->next) {
        printf("Queue block -> %d\n", val->block);
        val = (disk_request_t *) val->next;
    }
}


// disk_request * removeFromSuspended()