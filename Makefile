#target: dependencies
#	action

all: chef saladmaker create_saladmakers clean_semaphores

chef: chef.o chef_functions.o
	gcc chef.o chef_functions.o -o chef -lpthread -lm

chef.o: chef.c chef_functions.h
	gcc -c chef.c 

chef_functions.o: chef_functions.c chef_functions.h structs.h
	gcc -c chef_functions.c 

saladmaker: saladmaker.o saladmaker_functions.o
	gcc saladmaker.o saladmaker_functions.o -o saladmaker -lpthread -lm

saladmaker.o: saladmaker.c saladmaker_functions.h
	gcc -c saladmaker.c

saladmaker_functions.o: saladmaker_functions.c saladmaker_functions.h structs.h
	gcc -c saladmaker_functions.c 

create_saladmakers: create_saladmakers.o
	gcc create_saladmakers.o -o create_saladmakers

create_saladmakers.o: create_saladmakers.c
	gcc -c create_saladmakers.c

clean_semaphores: clean_semaphores.o
	gcc clean_semaphores.o -o clean_semaphores -lpthread

clean_semaphores.o: clean_semaphores.c
	gcc -c clean_semaphores.c 

clean_sem:
	./clean_semaphores

clean:
	rm *.o chef saladmaker create_saladmakers clean_semaphores