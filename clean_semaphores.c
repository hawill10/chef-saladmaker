#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>  
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define TIMEBUFF 6

int main() {

    sem_t* sems[3];

    sems[0] = sem_open("/semaphore0", O_CREAT,  0666, 0);
    sems[1] = sem_open("/semaphore1", O_CREAT,  0666, 0);
    sems[2] = sem_open("/semaphore2", O_CREAT,  0666, 0);
    sem_t* wb_mutex;
    wb_mutex = sem_open("/workbench", O_CREAT, 0666, 1);
    sem_t* shared_var_mutex = sem_open("/shared_var_mutex", O_CREAT, 0666, 1);
    sem_t* log_mutex = sem_open("/log_mutex", O_CREAT, 0666, 1);

    sem_close(sems[0]);
    sem_unlink("/semaphore0");
    sem_close(sems[1]);
    sem_unlink("/semaphore1");
    sem_close(sems[2]);
    sem_unlink("/semaphore2");
    sem_close(wb_mutex);
    sem_unlink("/workbench");
    sem_close(shared_var_mutex);
    sem_unlink("/shared_var_mutex");
    sem_close(log_mutex);
    sem_unlink("/log_mutex");
    printf("Semaphore unlinked\n");
}