#include "chef_functions.h"

int main(int argc, char* argv[]) {
    srand(time(NULL)); // seed random with current time

    parse(argc, argv);
    printf("Chef running\n");

    // create and attach shared memory
    create_shared_mem();

    // randomly assign ingredient for each saladmaker
    shuffle(sizeof(maker_index)/sizeof(*maker_index), maker_index);

    // initialize sharedstruct
    init_shared_struct(maker_index);
    
    //open semaphores
    open_semaphores();

    // create log file
    create_log_file();

    head = NULL;
    time_buffer_out = 0;

    // prompt saladmakers to run
    printf("Start saladmakers. Press 'Enter/Return' after running saladmakers (Shared Memory id: %d) >", id); getchar();

    // record start time of the program
    tics_per_sec = (double) sysconf(_SC_CLK_TCK);
    start_t = (double) times(&tb);

    // place ingredients until all salads are made
    printf("[Chef] Distributing ingredients...\n");
    while(1){
        // read if there's anything written to time_buff
        sem_wait(shared_var_mutex);
        read_time_buff();
        sem_post(shared_var_mutex);

        // randomly choose which saladmaker to give ingredients to
        int i = rand_maker(NUM_MAKER);
        int ind = maker_index[i];

        sem_wait(wb_mutex);

        // check salads made
        sem_wait(shared_var_mutex);
        // if all salads are made, break
        // sum of status array is added in order to prevent chef from placing more ingredients than needed
        if (mem->salad_made + sum_array(mem->status, NUM_MAKER) == num_salad) {
            sem_post(shared_var_mutex);
            break;
        }
        sem_post(shared_var_mutex);

        //place ingredients on workbench
        place_ingredients(ind);
        
        printf("[Chef] Distributing ingredients...\n");

        // wake particular saladmaker up to take ingredients
        sem_post(sems[i]);

        take_break(); //chef takes break for a random amount of time
    }
    
    // wait until all saladmakers finish making salad, then post their semaphores
    printf("[Chef] Waiting for all saladmakers to be finished\n");
    wait_final_salad();

    // print results
    print_results();

    printf("[Chef] Done!\n");

    // remove shared segment
    remove_shared_mem();

    // close and unlink all semaphores
    close_semaphores();
}