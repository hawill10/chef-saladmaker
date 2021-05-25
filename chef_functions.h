#include "structs.h"

#define BUFFSIZE 255
#define SEGMENTSIZE sizeof(SharedStruct)
#define SEGMENTPERM 0666

typedef struct TimeList {
    double time;
    struct TimeList* next;
} TimeList;

extern int id, err;

extern SharedStruct* mem;
extern int num_salad;
extern int chef_time;

extern sem_t* sems[3];
extern sem_t* wb_mutex;
extern sem_t* shared_var_mutex;
extern sem_t* log_mutex;

extern char* filename;

extern char buff[BUFFSIZE];
extern int time_buffer_out;

extern TimeList *head;
extern TimeList *cur;

extern int put_weight[NUM_MAKER];
extern int maker_index[NUM_MAKER];
extern char* ingredients[NUM_MAKER];

extern double start_t;
extern struct tms tb;
extern double tics_per_sec;

void parse(int argc, char* argv[]); // parse command line arguments
void create_shared_mem();
void init_shared_struct(int* maker_index);
void open_semaphores();
void create_log_file();
int rand_maker(int n); // randomly select a saladmaker to provide ingredients for
void shuffle(int len, int* list); // shuffle a list (decide which salad maker gets which ingredient)
float random_range(float min, float max); // create random floating number between min and max
void take_break(); // sleep for a random amount of time
void print_ingredients_used(char* ingredients[NUM_MAKER]); // print all ingredients used by each saladmaker
void print_waiting_working_time(); // print how much time each salad maker waited and worked
void logging(char* msg); // log action
int sum_array(int* arr, int len); // add all elements of an array
void place_ingredients(int ind);
void read_time_buff();
void wait_final_salad();
void print_results();
void remove_shared_mem();
void close_semaphores(); // close and unlink all semaphores used