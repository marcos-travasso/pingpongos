#include "ppos_disk.h"
#include "ppos-core-globals.h"
#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
//Se der algum problema de que a leitura dos blocos parece estar indo para o lado, troque o separador de linhas do disk.dat para LF
struct sigaction usrsig;
disk_t* disk;

void add_waiting_task(waiting_task_t* task) {
    mutex_lock(&disk->queueMtx);

    if(disk->waitingTasks == NULL) {
        disk->waitingTasks = task;
        mutex_unlock(&disk->queueMtx);
        return;
    }

    waiting_task_t* lt = disk->waitingTasks;
    while(lt->next != NULL){
        lt = lt->next;
    }

    lt->next = task;
    mutex_unlock(&disk->queueMtx);
}

waiting_task_t* pop_waiting_task(){
    if(disk->waitingTasks == NULL){
        return NULL;
    }
    waiting_task_t* wt = disk->waitingTasks;
    disk->waitingTasks = wt->next;
    wt->next = NULL;
    return wt;
}

waiting_task_t* create_waiting_task(task_t* task, int command, int block, void *buffer){
    waiting_task_t* wt = malloc(sizeof(waiting_task_t));
    wt->task = task;
    wt->command = command;
    wt->block = block;
    wt->buffer = buffer;
    wt->next = NULL;

    return wt;
}

int disk_execute_command(waiting_task_t* wt){
    mutex_lock(&disk->diskAccessMtx);

    disk->processingTask = wt;
    if(disk_cmd(wt->command, wt->block, wt->buffer) < 0) {
        printf ("T%d - Erro escrevendo disco\n", task_id());
        exit (1);
    }

    wt->task->state = PPOS_TASK_STATE_SUSPENDED;
    task_suspend(wt->task, &sleepQueue);
    task_yield();

    while(disk_cmd(DISK_CMD_STATUS, 0, 0) != DISK_STATUS_IDLE){}

    mutex_unlock(&disk->diskAccessMtx);

    return 0;
}

void tratador (int signum){
    task_resume(disk->processingTask->task);
}

void disk_scheduler(void* arg){
    while(sleepQueue != NULL || readyQueue != NULL){
        waiting_task_t* wt = pop_waiting_task();
        if(wt == NULL) {
            taskExec->state = PPOS_TASK_STATE_SUSPENDED;
            task_suspend(taskExec, &sleepQueue);
            task_yield();
            continue;
        }
        task_resume(wt->task);
    }
}

int disk_mgr_init (int *numBlocks, int *blockSize) {
    disk = malloc(sizeof(disk_t));
    disk->waitingTasks = NULL;

    usrsig.sa_handler = tratador ;
    sigemptyset (&usrsig.sa_mask) ;
    usrsig.sa_flags = 0 ;
    if (sigaction (SIGUSR1, &usrsig, 0) < 0) {
        perror ("Erro em sigaction");
        exit (1);
    }

    if (mutex_create(&disk->diskAccessMtx)) {
        perror ("Erro criando mutex de acesso ao disco");
        exit (1);
    }

    if (mutex_create(&disk->queueMtx)) {
        perror ("Erro criando mutex de fila");
        exit (1);
    }

    if(disk_cmd(DISK_CMD_INIT, 0, 0) < 0) {
        perror ("Erro iniciando disco");
        exit (1);
    }

    task_create(&disk->scheduler, disk_scheduler, "");
    return 0;
}

int disk_block_read (int block, void *buffer) {
    waiting_task_t* wt = create_waiting_task(taskExec, DISK_CMD_READ, block, buffer);

    add_waiting_task(wt);

    wt->task->state = PPOS_TASK_STATE_SUSPENDED;
    task_suspend(wt->task, &sleepQueue);
    task_yield();

    disk_execute_command(wt);

    return 0;
}

int disk_block_write (int block, void *buffer) {
    waiting_task_t* wt = create_waiting_task(taskExec, DISK_CMD_WRITE, block, buffer);

    add_waiting_task(wt);

    wt->task->state = PPOS_TASK_STATE_SUSPENDED;
    task_suspend(wt->task, &sleepQueue);
    task_yield();

    disk_execute_command(wt);

    return 0;
}

