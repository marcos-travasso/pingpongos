// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.2 -- Julho de 2017

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

#include "ppos.h"

typedef struct waiting_task_t {
    task_t* task;
    int command;
    int block;
    void *buffer;
    struct waiting_task_t* next;
} waiting_task_t;

// estrutura que representa um disco no sistema operacional
typedef struct {
    mutex_t mtx;
    waiting_task_t* waitingTasks;
    waiting_task_t* processingTask;
    task_t scheduler;
} disk_t;


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
