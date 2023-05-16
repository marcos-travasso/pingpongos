#include "ppos_disk.h"
#include "ppos.h"
#include "ppos-core-globals.h"
#include "disk.h"
#include "stdio.h"
#include <signal.h>

struct sigaction usrsig ;

task_t* taskQueues;

void addToQueue(task_t* t){
    if(taskQueues == NULL) {
        taskQueues = t;
        return;
    }

    task_t* last = taskQueues;
    while(last->next != NULL) {
        last = last->next;
    }

    last->next = t;
    return;
}

task_t* removeFromQueue() {
    if(taskQueues == NULL){
        printf("error removing from queue\n");
        exit(0);
    }

    task_t* tbr = taskQueues;
    taskQueues = taskQueues->next;
    tbr->next = NULL;

    return tbr;
}

mutex_t mtx;
void tratador (int signum){
//    printf("current queue: ");
//    task_t* it = taskQueues;
//    while(it != NULL){
//        printf("%d -> ", it->id);
//        it = it->next;
//    }
//    printf("NULL\n");

//    printf("%5d ms T%d: Retornado da fila\n", systime(), task_id());
    task_resume(removeFromQueue());
//    while(disk_cmd(DISK_CMD_STATUS, 0, 0) != DISK_STATUS_IDLE){
//
//    }
}
int disk_mgr_init (int *numBlocks, int *blockSize) {
    if(disk_cmd(DISK_CMD_INIT, 0, 0) < 0){
        printf("MAIN: Erro inicializando disco\n");
        return -1;
    }

    mutex_create(&mtx);

    usrsig.sa_handler = tratador ;
    sigemptyset (&usrsig.sa_mask) ;
    usrsig.sa_flags = 0 ;
    if (sigaction (SIGUSR1, &usrsig, 0) < 0) {
        perror ("MAIN: Erro inicializando sighandler\n");
        return -1;
    }

    return 0;
}
//TODO adicionar um mutex pra enviar os comandos do disco
int disk_block_read (int block, void *buffer) {
//    printf("%5d ms T%d: Esperando mutex\n", systime(), task_id());
    if(mutex_lock(&mtx)){
        printf("MNGR: erro lockando mtx\n");
        return -1;
    }
//    printf("%5d ms T%d: Com o mutex, esperando sair IDLE\n", systime(), task_id());

    while(disk_cmd(DISK_CMD_STATUS, block, buffer) != DISK_STATUS_IDLE){}

//    printf("%5d ms T%d: em IDLE, fazendo então LEITURA\n", systime(), task_id());

    if(disk_cmd(DISK_CMD_READ, block, buffer) < 0){
        printf("MAIN: Erro agendando leitura\n");
        return -1;
    }

//    printf("%5d ms T%d: Mutex liberado\n", systime(), task_id());
//    mutex_unlock(&mtx);

//    printf("%5d ms T%d: Suspendendo\n", systime(), task_id());
    taskExec->state = PPOS_TASK_STATE_SUSPENDED;
    addToQueue(taskExec);
//    printf("T%d esta suspensa\n", taskExec->id);
    task_suspend(taskExec, &sleepQueue);
    task_yield();
//    printf("%5d ms T%d: Voltando suspensao\n", systime(), task_id());

    while(disk_cmd(DISK_CMD_STATUS, block, buffer) != DISK_STATUS_IDLE){}


//    printf("%5d ms T%d: Mutex liberado\n", systime(), task_id());
    mutex_unlock(&mtx);

    return 0;
}

int disk_block_write (int block, void *buffer) {
//    printf("%5d ms T%d: Esperando mutex\n", systime(), task_id());
    if(mutex_lock(&mtx)){
        printf("MNGR: erro lockando mtx\n");
        return -1;
    }
//    printf("%5d ms T%d: Com o mutex, esperando sair IDLE\n", systime(), task_id());

    while(disk_cmd(DISK_CMD_STATUS, block, buffer) != DISK_STATUS_IDLE){}

//    printf("%5d ms T%d: em IDLE, fazendo então ESCRITA\n", systime(), task_id());


    if(disk_cmd(DISK_CMD_WRITE, block, buffer) < 0){
        printf("MAIN: Erro agendando escrita\n");
        return -1;
    }


//    printf("%5d ms T%d: Suspendendo\n", systime(), task_id());
    taskExec->state = PPOS_TASK_STATE_SUSPENDED;
    addToQueue(taskExec);
//    printf("T%d esta suspensa\n", taskExec->id);
    task_suspend(taskExec, &sleepQueue);
    task_yield();
//    printf("%5d ms T%d: Voltando suspensao\n", systime(), task_id());

    while(disk_cmd(DISK_CMD_STATUS, block, buffer) != DISK_STATUS_IDLE){}

//    printf("%5d ms T%d: Mutex liberado\n", systime(), task_id());
    mutex_unlock(&mtx);
    return 0;
}