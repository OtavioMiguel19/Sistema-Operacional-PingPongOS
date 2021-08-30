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

task_t disk_manager;             // Task do Gerenciador de Tarefas
struct sigaction sigaction_disk; // Struct que trata o sinal definido pelo usuário - n 1
static disk_t disk;              // Struct que simula o disco

void diskDriverBody(void *args)
{
    while (1)
    {
        // bloqueando o disco pelo semaforo para tratar pela task de gerenciamento
        sem_down(&disk.semaphore);
        if (disk.signal == 1)
        {
            // ao receber o sinal, removemos a primeira task da lista de tasks suspensas do disco e a devolvemos para a fila de tarefas prontas
            task_t *requester = (task_t *)queue_remove((queue_t **)&(disk.suspendedQueue), (queue_t *)disk.current_op->requester);
            queue_append((queue_t **)&readyQueue, (queue_t *)requester);

            disk.signal = 0;
            free(disk.current_op); //para evitar segmentationfault
        }

        // verificaçao se o disco esta em espera e se a fila nao esta vazia
        if (((disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE) == 1) && (disk.queue != NULL))
        {
            // removemos da lista queue do disco a primeira tarefa que foi colocada lá
            op_disk_t *request = (op_disk_t *)queue_remove((queue_t **)&disk.queue, (queue_t *)disk.queue);
            disk.current_op = request;
            // printf("Request block -> %d \n\n\n", request->block);
            disk_cmd((request->isRead == 1 ? DISK_CMD_READ : DISK_CMD_WRITE), request->block, request->buffer);
        }

        // liberando novamente o semaforo
        sem_up(&disk.semaphore);

        task_yield();
    }
}

void handleSignal(int signum)
{
    if (signum == SIGUSR1)
    {
        disk.signal = 1;
        // devolvendo a tarefa do gerenciador de disco para ser executada no processador
        queue_append((queue_t **)&readyQueue, (queue_t *)&disk_manager);
    }
}

int setupHandler()
{
    sigaction_disk.sa_handler = handleSignal;
    sigemptyset(&sigaction_disk.sa_mask);
    sigaction_disk.sa_flags = 0;

    sigaction(SIGUSR1, &sigaction_disk, 0);
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
    disk_manager.is_user_task = 0; // definindo como task de sistema

#ifdef DEBUG
#endif

    // o valor final do retorno verifica se todas as operaçoes importantes deram certo
    int finalReturn = (status_request_disk_init == 0 &&
                       status_request_signal_handler == 0 &&
                       status_request_sem_create == 0) == 1
                          ? 0
                          : -1;

    return finalReturn;
}

int disk_block_read(int block, void *buffer)
{
    sem_down(&disk.semaphore);

    //alocamos dinamicamente o objeto de operaçao de disco para utilizar na fila
    op_disk_t *op = malloc(sizeof(op_disk_t));
    op->isRead = 1;
    op->requester = taskExec;
    op->prev = op->next = NULL;
    op->buffer = buffer;
    op->block = block;

    queue_append((queue_t **)&disk.queue, (queue_t *)op);

    queue_append((queue_t **)&readyQueue, (queue_t *)&disk_manager);

    sem_up(&disk.semaphore);

    task_suspend(taskExec, &(disk.suspendedQueue));
    task_switch(taskDisp);

#ifdef DEBUG
#endif
    return 0;
}

int disk_block_write(int block, void *buffer)
{
    sem_down(&disk.semaphore);

    op_disk_t *op = malloc(sizeof(op_disk_t));
    op->isRead = 0;
    op->requester = taskExec;
    op->prev = op->next = NULL;
    op->buffer = buffer;
    op->block = block;

    queue_append((queue_t **)&disk.queue, (queue_t *)op);

    queue_append((queue_t **)&readyQueue, (queue_t *)&disk_manager);
    sem_up(&disk.semaphore);

    task_suspend(taskExec, &(disk.suspendedQueue));
    task_switch(taskDisp);

#ifdef DEBUG
#endif
    return 0;
}