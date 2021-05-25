#include "chef_functions.h"

int id, err;

int num_salad;
int chef_time;
SharedStruct* mem;

sem_t* sems[3];
sem_t* wb_mutex;
sem_t* shared_var_mutex;
sem_t* log_mutex;

char* filename = "log";

char buff[BUFFSIZE];
int time_buffer_out;

TimeList *head;
TimeList *cur;

int put_weight[NUM_MAKER] = {80, 50, 30};
int maker_index[NUM_MAKER] = {0, 1, 2};
char* ingredients[NUM_MAKER] = {"tomato", "green pepper", "onion"};

double start_t;
struct tms tb;
double tics_per_sec;


// parsing arguments: https://www.geeksforgeeks.org/getopt-function-in-c-to-parse-command-line-arguments/
void parse(int argc, char* argv[]){
    char opt;

    while((opt = getopt(argc, argv, ":n:m:")) != -1) {

        switch(opt)
        {
            case 'm':
                chef_time = atoi(optarg);
                // printf("chef_time: %s\n", optarg);
                break;
            case 'n':
                num_salad = atoi(optarg);
                // printf("num_salad: %d\n", atoi(optarg));
                break;
            case ':':
                fprintf(stderr, "Error: ./chef -n numOfSalads -m cheftime\n");
                exit(EXIT_FAILURE);
            case '?':
                fprintf(stderr, "Error: ./chef -n numOfSalads -m cheftime\n");
                exit(EXIT_FAILURE);
        }
    }

    if ((optind < argc) || !(num_salad && chef_time)) {
        fprintf(stderr, "Error: ./chef -n numOfSalads -m cheftime\n");  
        exit(EXIT_FAILURE);
    }
}

void create_shared_mem() {
    // create shared memory
    id = shmget(IPC_PRIVATE, SEGMENTSIZE, SEGMENTPERM);
    if ( id == -1) perror ("Creation Error");
    else printf("Allocated Shared Memory.\n");

    // attach shared memory
    mem = (SharedStruct*) shmat(id, NULL, 0);
    if ((int) mem == -1) perror("Attachment Error");
    else printf("Attached Shared Memory.\n");
}

void init_shared_struct(int* maker_index){
    mem->salad_made = 0;
    mem->time_start = 0;
    mem->time_total = 0;
    mem->time_count = 0;
    mem->target = num_salad;
    mem->time_buffer_in = 0;
    for(int i=0; i<NUM_MAKER; i++){
        (mem->status)[i] = 0;
        for(int j=0; j<NUM_MAKER; j++) {
            (mem->used_ingred)[i][j] = 0;
        }
        (mem->maker_index)[i] = maker_index[i];
    }
}

void open_semaphores() {
    // named semaphores between chef and saladmakers
    sems[0] = sem_open("/semaphore0", O_CREAT,  0666, 0);
    sems[1] = sem_open("/semaphore1", O_CREAT,  0666, 0);
    sems[2] = sem_open("/semaphore2", O_CREAT,  0666, 0);

    // workbench mutex
    wb_mutex = sem_open("/workbench", O_CREAT, 0666, 1);

    // shared variables mutex
    shared_var_mutex = sem_open("/shared_var_mutex", O_CREAT, 0666, 1);

    // log file mutex
    log_mutex = sem_open("/log_mutex", O_CREAT, 0666, 1);
}

void create_log_file() {
    FILE *log_file = fopen(filename, "w");

    // reference: https://stackoverflow.com/questions/1442116/how-to-get-the-date-and-time-values-in-a-c-program
    time_t t = time(NULL);
    struct tm tm1 = *localtime(&t);
    fprintf(log_file, "Start time: %d-%02d-%02d %02d:%02d:%02d\n", tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);

    fclose(log_file);
}

// generate random number https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c
int rand_maker(int n) {
    int r = rand()%n;
    return r;
}

// shuffling: https://www.w3resource.com/c-programming-exercises/array/c-array-exercise-77.php
void shuffle(int len, int* list) {

    for(int i=len-1; i>0; i--) {
        int j = rand() % (i+1);
        int temp = list[i];
        list[i] = list[j];
        list[j] = temp;
    }
}

float random_range(float min, float max) {

    return min + (((float) rand()) / RAND_MAX * (max - min));
}

void take_break() {
    int breaktime = round(chef_time * random_range(0.5, 1.0));
    sprintf(buff, "[Chef] Take break for %d seconds\n", breaktime);
    logging(buff);
    sleep(breaktime);
}

void print_ingredients_used(char* ingredients[NUM_MAKER]){
    sprintf(buff, "[Chef] Total Ingredients Used:\n");
    logging(buff);
    for(int i=0; i<3; i++){
        sprintf(buff, "[Chef] Saladmaker %d: ", i);
        logging(buff);
        for(int j=0; j<3; j++) {
            sprintf(buff, "%s - %dg | ", ingredients[j], mem->used_ingred[i][j]);
            logging(buff);
        }
        sprintf(buff, "\n");
        logging(buff);
    }
}

void print_waiting_working_time(){
    sprintf(buff, "[Chef] Total Working and Waiting Time:\n");
    logging(buff);
    for(int i=0; i<NUM_MAKER; i++){
        sprintf(buff, "[Chef] Saladmaker %d worked for %.1f seconds and waited for %.1f seconds\n", i, mem->working_time[i], mem->waiting_time[i]);
        logging(buff);
    }
}

void logging(char* msg) {
    sem_wait(log_mutex);
    FILE* fptr = fopen(filename, "a");
    fprintf(fptr, "%s", msg);
    fclose(fptr);
    sem_post(log_mutex);
}

int sum_array(int* arr, int len) {
    int sum = 0;
    for (int i=0; i<len; i++) {
        sum += arr[i];
    }
    return sum;
}

void place_ingredients(int ind) {
    int count = 0;
        for(int ing=0; ing<3; ing++){
            if (ing != ind) {
                mem->workbench[count] = round(put_weight[ing] * random_range(0.8, 1.2)); //randomize weight
                sprintf(buff, "[Chef] Put %dg of %s on workbench\n", mem->workbench[count], ingredients[ing]);
                logging(buff);
                count++;
            }
        }
}

void read_time_buff(){
    while(mem->time_count > 0) {
        TimeList *newTime = malloc(sizeof(TimeList));
        newTime->time = mem->time_buffer[time_buffer_out];
        newTime->next= NULL;
        if(!head){
            head = newTime;
            cur = head;
        } else {
            cur->next = newTime;
        }
        cur = newTime;
        time_buffer_out = (time_buffer_out + 1)%TIMEBUFF;
        mem->time_count--;
    }
}

void wait_final_salad() {
    while(1) {
        sem_wait(shared_var_mutex);
        if(mem->salad_made == num_salad){
            read_time_buff();
            // wake all salad makers up so that they can finish
            for(int i=0; i<NUM_MAKER; i++){
                sem_post(sems[i]);
            }
            sem_post(shared_var_mutex);
            break;
        }
        sem_post(shared_var_mutex);
    }
}

void print_results() {
    sprintf(buff, "\n===============================================\n\
    Results\n===============================================\n");
    logging(buff);

    sprintf(buff, "[Chef] A Total of %d Salads Made\n", mem->salad_made);
    logging(buff);

    print_ingredients_used(ingredients);
    print_waiting_working_time();

    sprintf(buff, "[Chef] List of Time Periods Workig in Parallel (seconds from program execution):\n");
    logging(buff);
    int cnt = 0;
    while(head) {
        if (cnt%2 == 0) {
            sprintf(buff, "Start Time: %.1f - ", (head->time- start_t)/tics_per_sec);
            logging(buff);
        }
        else {
            sprintf(buff, "End Time: %.1f\n", (head->time - start_t) / tics_per_sec);
            logging(buff);
        }
        cnt++;
        head = head->next;
    }

    sprintf(buff, "[Chef] Total time together: %.1f seconds\n", mem->time_total);
    logging(buff);
}

void remove_shared_mem() {
    err = shmctl(id, IPC_RMID, NULL);
    if (err == -1) perror("Removal Error");
    else printf("Removed Shared Memory.\n");
}

void close_semaphores() {
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