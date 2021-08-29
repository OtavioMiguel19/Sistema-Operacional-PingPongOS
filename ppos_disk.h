// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "disk.h"

#ifndef __DISK_MGR__
#define __DISK_MGR__

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// estrutura que representa um disco no sistema operacional
typedef struct
{
  int status ;			// estado do disco
  char *filename ;		// nome do arquivo que simula o disco
  int fd ;			// descritor do arquivo que simula o disco
  int numblocks ;		// numero de blocos do disco
  int blocksize ;		// tamanho dos blocos em bytes
  char *buffer ;		// buffer da proxima operacao (read/write)
  int prev_block ;		// bloco da ultima operacao
  int next_block ;		// bloco da proxima operacao
  int delay_min, delay_max ;	// tempos de acesso mínimo e máximo
  timer_t           timer ;	// timer que simula o tempo de acesso
  struct itimerspec delay ;	// struct do timer de tempo de acesso
  struct sigevent   sigev ;	// evento associado ao timer
  struct sigaction  signal ;	// tratador de sinal do timer
} disk_t ;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif
