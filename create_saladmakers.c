#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    for(int i=0; i < 3; i++) {
        int pid = fork();

        if (pid < 0){    
            fprintf(stderr, "Failed to fork saladmaker %d\n", i);  // fork failed
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            char** args = malloc(sizeof(*args)*(argc+3));
            argv[0] = "./saladmaker";
            memcpy(&args[0], &argv[0], sizeof(*argv)*argc);
            args[argc] = "-n";
            char num[2];
            sprintf(num, "%d", i);
            args[argc + 1] = num;
            printf("Execute saladmaker %d\n", i);
            execv(args[0], &args[0]);
            exit(-1);
        }
    }
}