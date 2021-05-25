#include "saladmaker_functions.h"

int main(int argc, char* argv[]) {
    srand(time(NULL));

    parse(argc, argv);

    int err;

    mem = (struct SharedStruct*) shmat(shm_id, (void*) 0, 0);
    if ((int) mem == -1) perror("Attachment Error");
    else printf("[Saladmaker %d] Attached.\n", maker_num);

    // open semaphores
    char sem_name[20];
    open_semaphores(sem_name);
    
    // for timing----------------------------------
    double t, start, end;
    struct tms tb, tb_s, tb_e;
    double tics_per_sec;
    
    tics_per_sec = (double) sysconf(_SC_CLK_TCK);
    // ----------------------------------------------

    int ind = mem->maker_index[maker_num];
    current_weight[ind] = required_weight[ind];
    
    printf("[Saladmaker %d] Ready to go\n", maker_num);
    while(1){
        // waiting time
        start = (double) times(&tb_s);
        sem_wait(sem);
        end = (double) times(&tb_e);

        // check if more salad has to be made
        sem_wait(shared_var_mutex);
        if(mem->salad_made + sum_array(mem->status, NUM_MAKER) == mem->target) {
            sem_post(shared_var_mutex);
            sem_post(wb_mutex);
            break;
        }
        sem_post(shared_var_mutex);

        //record waiting time
        mem->waiting_time[maker_num] += (end - start)/tics_per_sec;

        // take ingredients
        int count = 0;
        printf("[Saladmaker %d] Take Ingredients (Salad Made: %d):\n", maker_num, mem->salad_made);
        for(int ing=0; ing<3; ing++){
            if (ing != ind) {
                sprintf(buff, "[Saladmaker %d] Take %dg of %s on workbench\n", maker_num, mem->workbench[count], ingredients[ing]);
                logging(buff);
                current_weight[ing] += mem->workbench[count];
                count++;
            }
        } 

        // check if it has enough ingredients
        int enough_flag = 1;
        for (int i=0; i<NUM_MAKER; i++) {
            if (current_weight[i] < required_weight[i]) {
                enough_flag = 0;
                sprintf(buff, "[Saladmaker %d] Not enough ingredients\n", maker_num);
                logging(buff);
                break;
            }
        }
        if (!enough_flag) { 
            sem_post(wb_mutex);
            continue;
        }

        // start making salad
        sem_wait(shared_var_mutex);
        mem->status[maker_num] = 1;
        sem_post(wb_mutex);

        // record start working time
        start = (double) times(&tb_s);
        printf("[Saladmaker %d] Working...\n", maker_num);
        sprintf(buff, "[Saladmaker %d] Start Making Salad\n", maker_num);
        logging(buff);

        // check if two or more saladmakers are working together
        int active = count_active(mem->status, NUM_MAKER);
        if (active == 2) {
            // record start working time together
            t = (double) times(&tb);
            mem->time_start = t;
            logging("[Saladmakers] Working Simultaneously!\n");
            mem->time_buffer[mem->time_buffer_in] = t;
            mem->time_buffer_in = (mem->time_buffer_in + 1)%TIMEBUFF;
            mem->time_count += 1;
        }
        sem_post(shared_var_mutex);

        // time for making salad
        make_salad();

        sem_wait(shared_var_mutex);
        // add to used ingredients
        for(int i=0; i<3; i++) {
            mem->used_ingred[maker_num][i] += current_weight[i];
            if (i != ind) {
                current_weight[i] = 0;
            }
        }
        mem->salad_made++;
        sprintf(buff, "[Saladmaker %d] Made Salad! (%dth salad)\n", maker_num, mem->salad_made);
        logging(buff);
        mem->status[maker_num] = 0;
        
        // record end of working time
        end = (double) times(&tb_e);
        mem->working_time[maker_num] += (end - start)/tics_per_sec;

        // check if two or more saladmakers are working together
        active = count_active(mem->status, NUM_MAKER);
        if (active == 1) {
            // record end working time together
            t = (double) times(&tb);
            mem->time_total += (t - mem->time_start)/tics_per_sec;
            logging("[Saladmakers] Back to Working Alone\n");

            // write to shared memory (end working together)
            mem->time_buffer[mem->time_buffer_in] = t;
            mem->time_buffer_in = (mem->time_buffer_in + 1)%TIMEBUFF;
            mem->time_count += 1;
        }
        sem_post(shared_var_mutex);
    }

    // close all semaphores
    sem_close(sem);
    sem_close(wb_mutex);
    sem_close(shared_var_mutex);
    sem_close(log_mutex);
    printf("Semaphore closed\n");

    // remove segment
    err = shmdt((void*) mem);
    if (err == -1) perror("Detachment Error");
    else printf("Detached Shared Memory.\n");

    printf("[Saladmaker %d] Done!\n", maker_num);
}