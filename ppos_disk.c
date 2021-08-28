#include "ppos.h"
#include "ppos-core-globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

int disk_mgr_init(int *numBlocks, int *blockSize)
{
    // put your customization here
#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
    return -1;
}

int disk_block_read(int block, void *buffer)
{
    // put your customization here
#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
}

int disk_block_write(int block, void *buffer)
{
    // put your customization here
#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
}