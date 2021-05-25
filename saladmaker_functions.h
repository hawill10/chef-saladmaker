#include "structs.h"

#define BUFFSIZE 255

extern int salmkrtime;
extern int shm_id;
extern int maker_num;
extern int required_weight[3];
extern char* ingredients[NUM_MAKER];

extern sem_t *sem;
extern sem_t* wb_mutex;
extern sem_t* shared_var_mutex;
extern sem_t* log_mutex;

extern SharedStruct *mem;

extern char* filename;

extern char buff[BUFFSIZE];

extern int required_weight[NUM_MAKER];
extern int current_weight[NUM_MAKER];
extern char* ingredients[NUM_MAKER];

void parse(int argc, char* argv[]);
void open_semaphores(char* sem_name);
int count_active(int* arr, int len);
float random_range(float min, float max);
void logging(char* msg);
void make_salad();
int sum_array(int* arr, int len);