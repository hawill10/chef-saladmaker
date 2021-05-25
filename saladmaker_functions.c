#include "saladmaker_functions.h"

int salmkrtime;
int shm_id;
int maker_num;
int required_weight[3];
char* ingredients[NUM_MAKER];

sem_t *sem;
sem_t* wb_mutex;
sem_t* shared_var_mutex;
sem_t* log_mutex;

SharedStruct *mem;

char* filename = "log";

char buff[BUFFSIZE];

int required_weight[3] = {80, 50, 30};
int current_weight[3] = {0, 0, 0};
char* ingredients[NUM_MAKER] = {"tomato", "green pepper", "onion"};

// parsing arguments: https://www.geeksforgeeks.org/getopt-function-in-c-to-parse-command-line-arguments/
void parse(int argc, char* argv[]){
    char opt;

    while((opt = getopt(argc, argv, ":m:s:n:")) != -1) {

        switch(opt)
        {
            case 'm':
                salmkrtime = atoi(optarg);
                // printf("salmkrtime: %s\n", optarg);
                break;
            case 's':
                shm_id = atoi(optarg);
                // printf("shm_id: %d\n", atoi(optarg));
                break;
            case 'n':
                maker_num = atoi(optarg);
                // printf("maker_num: %d\n", atoi(optarg));
                break;
            case ':':
                fprintf(stderr, "Error: ./create_saladmakers -m salmkrtime -s shmid\n");  // fork failed
                exit(EXIT_FAILURE);
            case '?':
                fprintf(stderr, "Error: ./create_saladmakers -m salmkrtime -s shmid\n");  // fork failed
                exit(EXIT_FAILURE);
        }
    }

    if ((optind < argc) || !(salmkrtime && shm_id)) {
        fprintf(stderr, "Error: ./create_saladmakers -m salmkrtime -s shmid\n");  // fork failed
        exit(EXIT_FAILURE);
    }
}

void open_semaphores(char* sem_name) {
    sprintf(sem_name, "/semaphore%d", maker_num);
    sem = sem_open(sem_name, 0);
    
    wb_mutex = sem_open("/workbench", 0);

    shared_var_mutex = sem_open("/shared_var_mutex", 0);

    log_mutex = sem_open("/log_mutex", 0);
}

int count_active(int* arr, int len) {
    int count = 0;
    for(int i=0; i<len; i++) {
        if (arr[i] == 1) {
            count++;
        }
    }
    return count;
}

float random_range(float min, float max) {

    return min + (((float) rand()) / RAND_MAX * (max - min));
}

void logging(char* msg) {
    sem_wait(log_mutex);
    FILE* fptr = fopen(filename, "a");
    fprintf(fptr, "%s", msg);
    fclose(fptr);
    sem_post(log_mutex);
}

void make_salad() {
    int saltime = round(salmkrtime * random_range(0.8, 1.0));
    sprintf(buff, "[Saladmaker %d] Make salad for %d seconds\n", maker_num, saltime);
    logging(buff);
    sleep(saltime);
}

int sum_array(int* arr, int len) {
    int sum = 0;
    for (int i=0; i<len; i++) {
        sum += arr[i];
    }
    return sum;
}