#include "ppos.h"
#include "ppos-core-globals.h"

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "disk.h"
#include "ppos_disk.h"

#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

semaphore_t sem;
task_t disk_manager;

#define DISK_NAME "disk.dat" // arquivo com o conteúdo do disco
#define DISK_BLOCK_SIZE 64   // tamanho de cada bloco, em bytes
#define DISK_DELAY_MIN 30    // atraso minimo, em milisegundos
#define DISK_DELAY_MAX 300   // atraso maximo, em milisegundos

// should be static to avoid clash with "disk" variables in other files
static disk_t disk; // hard disk structure

// trata o sinal SIGIO do timer que simula o tempo de acesso ao disco
static void disk_sighandle(int sig)
{
#ifdef DEBUG_DISK
    printf("DISK: signal %d received\n", sig);
#endif

    // verificar qual a operacao pendente e realiza-la
    switch (disk.status)
    {
    case DISK_STATUS_READ:
        // faz a leitura previamente agendada
        lseek(disk.fd, disk.next_block * disk.blocksize, SEEK_SET);
        read(disk.fd, disk.buffer, disk.blocksize);
        break;

    case DISK_STATUS_WRITE:
        // faz a escrita previamente agendada
        lseek(disk.fd, disk.next_block * disk.blocksize, SEEK_SET);
        write(disk.fd, disk.buffer, disk.blocksize);
        break;

    default:
        // erro: estado desconhecido
        perror("DISK: unknown disk state");
        exit(1);
    }

    // guarda numero de bloco da ultima operacao
    disk.prev_block = disk.next_block;

    // disco se torna ocioso novamente
    disk.status = DISK_STATUS_IDLE;

    // gerar um sinal SIGUSR1 para o "kernel" do usuario
    raise(SIGUSR1);
}

static int disk_init()
{
    // o disco jah foi inicializado ?
    if (disk.status != DISK_STATUS_UNKNOWN)
        return -1;

    // estado atual do disco
    disk.status = DISK_STATUS_IDLE;
    disk.next_block = disk.prev_block = 0;

    // abre o arquivo no disco (leitura/escrita, sincrono)
    disk.filename = DISK_NAME;
    disk.fd = open(disk.filename, O_RDWR | O_SYNC);
    if (disk.fd < 0)
    {
        perror("DISK: " DISK_NAME);
        exit(1);
    }

    // define seu tamanho em blocos
    disk.blocksize = DISK_BLOCK_SIZE;
    disk.numblocks = lseek(disk.fd, 0, SEEK_END) / disk.blocksize;

    // ajusta atrasos mínimo e máximo de acesso no disco
    disk.delay_min = DISK_DELAY_MIN;
    disk.delay_max = DISK_DELAY_MAX;

    // associa SIGIO do timer ao handle apropriado
    disk.signal.sa_handler = disk_sighandle;
    sigemptyset(&disk.signal.sa_mask);
    disk.signal.sa_flags = 0;
    sigaction(SIGIO, &disk.signal, 0);

    // cria o timer que simula o tempo de acesso ao disco
    disk.sigev.sigev_notify = SIGEV_SIGNAL;
    disk.sigev.sigev_signo = SIGIO;
    if (timer_create(CLOCK_REALTIME, &disk.sigev, &disk.timer) == -1)
    {
        perror("DISK:");
        exit(1);
    }

#ifdef DEBUG_DISK
    printf("DISK: initialized\n");
#endif

    return 0;
}

void diskDriverBody(void *args)
{
    while (true)
    {
        // obtém o semáforo de acesso ao disco
        sem_down(&sem);

        // se foi acordado devido a um sinal do disco
        if (disco gerou um sinal)
        {
            // acorda a tarefa cujo pedido foi atendido
        }

        // se o disco estiver livre e houver pedidos de E/S na fila
        if (disco_livre && (fila_disco != NULL))
        {
            // escolhe na fila o pedido a ser atendido, usando FCFS
            // solicita ao disco a operação de E/S, usando disk_cmd()
        }

        // libera o semáforo de acesso ao disco
        sem_up(&sem);

        // suspende a tarefa corrente (retorna ao dispatcher)
        task_yield(); // sera q eh essa?
    }
}

int disk_mgr_init(int *numBlocks, int *blockSize)
{
    task_create(&disk_manager, diskDriverBody, NULL);
    task_join(&disk_manager);

    // put your customization here
#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
    int retorno = sem_create(&sem, 1);
    printf("sem_create retorno -> %d \n", retorno);

    // retorno = sem_down (&sem);
    // printf("sem_down retorno -> %d \n", retorno);

    // retorno = sem_up (&sem);
    // printf("sem_up retorno -> %d \n", retorno);

    // retorno = sem_down (&sem);
    // printf("sem_down retorno -> %d \n", retorno);

    *numBlocks = 2;
    *blockSize = DISK_BLOCK_SIZE;

    printf("numBlocks -> %d \n", *numBlocks);
    printf("blockSize -> %d \n", *blockSize);

    // disk_init();

    return retorno;
}

int disk_block_read(int block, void *buffer)
{
    // put your customization here
#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif

    printf("tentou ler bloco -> %d \n", block);
    sem_down(&sem);
}

int disk_block_write(int block, void *buffer)
{
    // put your customization here
#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif

    printf("tentou escrever bloco -> %d \n", block);
    sem_down(&sem);
}