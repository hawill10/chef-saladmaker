#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>  
#include <sys/times.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <math.h>

#define NUM_MAKER 3
#define TIMEBUFF 6

typedef struct SharedStruct {
    int workbench[2]; // weight of ingredients placed on workbench
    int status[NUM_MAKER]; // working status for each salad maker (0: not making salad, 1: making salad)
    double time_start; // time salad makers start working in parallel
    double time_total; // total amount of time salad makers worked in parallel
    double time_buffer[TIMEBUFF]; // buffer for recording time saladmakers worked in parallel
    int time_buffer_in; // index of buffer to write to (synchronize among saladmakers)
    int time_count; // count of records to be consumed
    double waiting_time[NUM_MAKER]; // time each salad maker waited for ingredients
    double working_time[NUM_MAKER]; // time each salad maker worked (making salads)
    int salad_made; // number of salads made so far
    int target; // target number of salads to be made
    int used_ingred[3][3]; // weight of ingredients used by each salad maker
    int maker_index[3]; // index that notifies which ingredient a salad maker has all the time
} SharedStruct;