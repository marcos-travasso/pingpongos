#include "ppos_disk.h"
#include "ppos.h"
#include "ppos-core-globals.h"
#include "disk.h"
#include "stdio.h"
#include <signal.h>
//Se der algum problema de que a leitura dos blocos parece estar indo para o lado, troque o separador de linhas do disk.dat
struct sigaction usrsig ;
mutex_t mtx;
task_t* waitingTask;

void tratador (int signum){
    task_resume(waitingTask);
}

int disk_mgr_init (int *numBlocks, int *blockSize) {
    usrsig.sa_handler = tratador ;
    sigemptyset (&usrsig.sa_mask) ;
    usrsig.sa_flags = 0 ;
    if (sigaction (SIGUSR1, &usrsig, 0) < 0) {
        perror ("Erro em sigaction");
        exit (1);
    }

    if (mutex_create(&mtx)) {
        perror ("Erro criando mutex");
        exit (1);
    }

    if(disk_cmd(DISK_CMD_INIT, 0, 0) < 0) {
        perror ("Erro iniciando disco");
        exit (1);
    }

    return 0;
}

int disk_block_read (int block, void *buffer) {
    mutex_lock(&mtx);

    if(disk_cmd(DISK_CMD_READ, block, buffer) < 0) {
        perror ("Erro lendo disco");
        exit (1);
    }

    taskExec->state = PPOS_TASK_STATE_SUSPENDED;
    waitingTask = taskExec;
    task_suspend(waitingTask, &sleepQueue);
    task_yield();

    while(disk_cmd(DISK_CMD_STATUS, 0, 0) != DISK_STATUS_IDLE){}

    mutex_unlock(&mtx);

    return 0;
}

int disk_block_write (int block, void *buffer) {
    mutex_lock(&mtx);

    if(disk_cmd(DISK_CMD_WRITE, block, buffer) < 0) {
        perror ("Erro escrevendo disco");
        exit (1);
    }

    taskExec->state = PPOS_TASK_STATE_SUSPENDED;
    waitingTask = taskExec;
    task_suspend(waitingTask, &sleepQueue) ;
    task_yield();

    while(disk_cmd(DISK_CMD_STATUS, 0, 0) != DISK_STATUS_IDLE){}

    mutex_unlock(&mtx);

    return 0;
}