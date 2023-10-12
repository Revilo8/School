//  Author: Oliver Mokras (xmokra01)
//  Date: 4/20/2023
//  Description: IOS Project 2

#define _DEFAULT_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include "sys/wait.h"
#include "sys/shm.h"
#include "sys/mman.h"
#include<fcntl.h>
#include<semaphore.h>

FILE *file;
int *line;
int *office_closed;

sem_t *sem;
sem_t *queue_sem1;
sem_t *queue_sem2;
sem_t *queue_sem3;
sem_t *ser_finished1;
sem_t *ser_finished2;
sem_t *ser_finished3;

int *id_queue1;
int *tail1;
int *head1;
int *count1;

int *id_queue2;
int *tail2;
int *head2;
int *count2;

int *id_queue3;
int *tail3;
int *head3;
int *count3;

int MAX_QUEUE_SIZE;


void check_args(int argc, char *argv[]){
    //Check if number of arguments is correct
    if (argc != 6){
        fprintf(stderr, "Invalid number of arguments\n");
        exit(1);
    }
    //Check if all arguments are integers
    char *endptr;
    for (int i = 1; i < argc; i++){
        strtol(argv[i], &endptr, 10); // If endptr is anything else than '\0' then it is not an integer
        if (*endptr != '\0') {
            fprintf(stderr, "Error: Input must be an integer\n");
            exit(1);
        }
    }
    //First arg check
    long argument = strtol(argv[1], &endptr, 10);
    if (argument < 0){
        fprintf(stderr, "Error: Negative number of customers\n");
        exit(1);
    }
    //Second arg check
    argument = strtol(argv[2], &endptr, 10);
    if (argument < 0){
        fprintf(stderr, "Error: Negative number of office workers\n");
        exit(1);
    }
    //Third arg check
    argument = strtol(argv[3], &endptr, 10);
    if (!(argument >= 0 && argument <= 10000)){
        fprintf(stderr, "Error: Third argument must be in range 0-10000\n");
        exit(1);
    }
    //Fourth arg check
    argument = strtol(argv[4], &endptr, 10);
    if (!(argument >= 0 && argument <= 100)){
        fprintf(stderr, "Error: Fourth argument must be in range 0-100\n");
        exit(1);
    }
    //Fifth arg check
    argument = strtol(argv[5], &endptr, 10);
    if (!(argument >= 0 && argument <= 10000)){
        fprintf(stderr, "Error: Fifth argument must be in range 0-10000\n");
        exit(1);
    }
}
// Parse the arguments into an array of longs
long* parse_args(int argc, char *argv[]){
    long *arg_array;
    char *endptr;
    arg_array = (long*) malloc((argc-1) * sizeof(long));
    for (int i = 1; i < argc; i++){
        arg_array[i-1] = strtol(argv[i], &endptr, 10);
    }
    return arg_array;
}
// Print map error and exit
void map_error(){
    fprintf(stderr, "Error while mapping memory\n");
    exit(1);
}
// Print semaphore error and exit
void sem_init_error(){
    fprintf(stderr, "Error while initializing semaphore\n");
    exit(1);
}
// Create shared memory
void shared_mem(){
    if ( (line = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (office_closed = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (queue_sem1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (queue_sem2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (queue_sem3 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (ser_finished1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (ser_finished2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (ser_finished3 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    
    if ( (id_queue1 = mmap(NULL, sizeof(int) * MAX_QUEUE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (count1 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (tail1 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (head1 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    *(tail1) = 0;
    *(head1) = 0;
    *(count1) = 0;

    if ( (id_queue2 = mmap(NULL, sizeof(int) * MAX_QUEUE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (count2 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (tail2 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (head2 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    *(tail2) = 0;
    *(head2) = 0;
    *(count2) = 0;

    if ( (id_queue3 = mmap(NULL, sizeof(int) * MAX_QUEUE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (count3 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (tail3 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    if ( (head3 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0)) == MAP_FAILED) map_error();
    *(tail3) = 0;
    *(head3) = 0;
    *(count3) = 0;


    *line = 0;
    *office_closed = 0;

    if(sem_init(sem, 1, 1) == -1) sem_init_error();
    if(sem_init(queue_sem1, 1, 0) == -1) sem_init_error();
    if(sem_init(queue_sem2, 1, 0) == -1) sem_init_error();
    if(sem_init(queue_sem3, 1, 0) == -1) sem_init_error();
    if(sem_init(ser_finished1, 1, 0) == -1) sem_init_error();
    if(sem_init(ser_finished2, 1, 0) == -1) sem_init_error();
    if(sem_init(ser_finished3, 1, 0) == -1) sem_init_error();
}
// Clean up shared memory
void clean_up(){
    munmap(line, sizeof(int));
    munmap(office_closed, sizeof(int));
    munmap(sem, sizeof(sem_t));
    munmap(queue_sem1, sizeof(sem_t));
    munmap(queue_sem2, sizeof(sem_t));
    munmap(queue_sem3, sizeof(sem_t));
    munmap(ser_finished1, sizeof(sem_t));
    munmap(ser_finished2, sizeof(sem_t));
    munmap(ser_finished3, sizeof(sem_t));

    munmap(id_queue1, sizeof(int) * MAX_QUEUE_SIZE);
    munmap(count1, sizeof(int));
    munmap(tail1, sizeof(int));
    munmap(head1, sizeof(int));

    munmap(id_queue2, sizeof(int) * MAX_QUEUE_SIZE);
    munmap(count2, sizeof(int));
    munmap(tail2, sizeof(int));
    munmap(head2, sizeof(int));

    munmap(id_queue3, sizeof(int) * MAX_QUEUE_SIZE);
    munmap(count3, sizeof(int));
    munmap(tail3, sizeof(int));
    munmap(head3, sizeof(int));

    sem_destroy(sem);
    sem_destroy(queue_sem1);
    sem_destroy(queue_sem2);
    sem_destroy(queue_sem3);
    sem_destroy(ser_finished1);
    sem_destroy(ser_finished2);
    sem_destroy(ser_finished3);
}

void queue1(int id){
    fprintf(file, "%d: Z %d: entering office for a service 1\n", ++(*line), id);
    id_queue1[*tail1] = id;     // Adding process id to queue
    *tail1 = (*tail1 + 1) % MAX_QUEUE_SIZE;
    (*count1)++;
    
    sem_post(sem);
    sem_wait(queue_sem1);       // Waiting for office worker to call

    int leaving_id;
    if (*count1 > 0 ){
        leaving_id = id_queue1[*head1];     // Getting process id from queue
        *head1 = (*head1 + 1) % MAX_QUEUE_SIZE;
        (*count1)--;
    } else{
        fprintf(stderr, "Error: Queue 1 is empty\n");
        exit(1);
    }

    fprintf(file, "%d: Z %d: called by office worker\n", ++(*line), leaving_id);

    usleep((rand() % 11) * 1000);

    sem_wait(ser_finished1);        // Waiting for service to finish
    sem_wait(sem);
    fprintf(file, "%d: Z %d: going home\n", ++(*line), leaving_id);
    sem_post(sem);
}

void queue2(int id){
    fprintf(file, "%d: Z %d: entering office for a service 2\n", ++(*line), id);
    id_queue2[*tail2] = id;
    *tail2 = (*tail2 + 1) % MAX_QUEUE_SIZE;
    (*count2)++;

    sem_post(sem);
    sem_wait(queue_sem2);

    int leaving_id;
    if (*count2 > 0 ){
        leaving_id = id_queue2[*head2];
        *head2 = (*head2 + 1) % MAX_QUEUE_SIZE;
        (*count2)--;
    } else{
        fprintf(stderr, "Error: Queue 2 is empty\n");
        exit(1);
    }

    fprintf(file, "%d: Z %d: called by office worker\n", ++(*line), leaving_id);

    usleep((rand() % 11) * 1000);

    sem_wait(ser_finished2);
    sem_wait(sem);
    fprintf(file, "%d: Z %d: going home\n", ++(*line), leaving_id);
    sem_post(sem);
}

void queue3(int id){
    fprintf(file, "%d: Z %d: entering office for a service 3\n", ++(*line), id);
    id_queue3[*tail3] = id;
    *tail3 = (*tail3 + 1) % MAX_QUEUE_SIZE;
    (*count3)++;

    sem_post(sem);
    sem_wait(queue_sem3);

    int leaving_id;
    if (*count3 > 0 ){
        leaving_id = id_queue3[*head3];
        *head3 = (*head3 + 1) % MAX_QUEUE_SIZE;
        (*count3)--;
    } else{
        fprintf(stderr, "Error: Queue 3 is empty\n");
        exit(1);
    }

    fprintf(file, "%d: Z %d: called by office worker\n", ++(*line), leaving_id);

    usleep((rand() % 11) * 1000);

    sem_wait(ser_finished3);
    sem_wait(sem);
    fprintf(file, "%d: Z %d: going home\n", ++(*line), leaving_id);
    sem_post(sem);
}
// Customer process
void customer(int id, int TZ){
    sem_wait(sem);
    fprintf(file, "%d: Z %d: started\n", ++(*line), id);
    sem_post(sem);

    //Generating random sleep time based on TZ
    srand(time(NULL) + id);
    int sleep_time_TZ = (rand() % (TZ + 1));
    usleep(sleep_time_TZ * 1000);

    //check if office is closed
    sem_wait(sem);
    if(*office_closed == 1){
        fprintf(file, "%d: Z %d: going home\n", ++(*line), id);
        sem_post(sem);
        return;
    }

    //Generating random service from 1,2,3
    srand(time(NULL) + id);
    int service = (rand() % 3) + 1;

    //Entering the office for service
    switch(service) {
    case 1:
        queue1(id);
        break;
    case 2:
        queue2(id);
        break;
    case 3:
        queue3(id);
        break;
    }
}
// Office worker process
void office_worker(int id, int TU){
    sem_wait(sem);
    fprintf(file, "%d: U %d: started\n", ++(*line), id);
    sem_post(sem);

    //Cycling through queues until office is closed and all queues are empty
    while (*office_closed == 0 || *count1 > 0 || *count2 > 0 || *count3 > 0){
        sem_wait(sem);
        if (*count1 > 0){
            sem_post(queue_sem1);       // Calling customer from queue
            fprintf(file, "%d: U %d: serving a service of type 1\n", ++(*line), id);

            srand(time(NULL) + id);
            usleep((rand() % 11) * 1000);

            fprintf(file, "%d: U %d: service finished\n", ++(*line), id);
            sem_post(ser_finished1);        // Customer can go home
        } else if (*count2 > 0){
            sem_post(queue_sem2);
            fprintf(file, "%d: U %d: serving a service of type 2\n", ++(*line), id);

            srand(time(NULL) + id);
            usleep((rand() % 11) * 1000);

            fprintf(file, "%d: U %d: service finished\n", ++(*line), id);
            sem_post(ser_finished2);
        } else if (*count3 > 0){
            sem_post(queue_sem3);
            fprintf(file, "%d: U %d: serving a service of type 3\n", ++(*line), id);

            srand(time(NULL) + id);
            usleep((rand() % 11) * 1000);
            
            fprintf(file, "%d: U %d: service finished\n", ++(*line), id);
            sem_post(ser_finished3);
        } else if (*office_closed == 0){        // If all queues are empty, office worker takes a break
            fprintf(file, "%d: U %d: taking break\n", ++(*line), id);
            sem_post(sem);
            srand(time(NULL) + id);
            usleep((rand() % (TU + 1)) * 1000);
            sem_wait(sem);
            fprintf(file, "%d: U %d: break finished\n", ++(*line), id);
        }
        sem_post(sem);
    }

    sem_wait(sem);
    fprintf(file, "%d: U %d: going home\n", ++(*line), id);
    sem_post(sem);
}


int main(int argc, char *argv[])
{
    check_args(argc, argv);

    long* arg_array = parse_args(argc, argv);

    if((file = fopen("proj2.out","w")) == NULL){
        fprintf(stderr, "Error while opening file\n");
        return 1;
    }
    setbuf(file, NULL);     // Disabling buffering

    int NZ = arg_array[0];
    int NU = arg_array[1];
    int TZ = arg_array[2];
    int TU = arg_array[3];
    int F = arg_array[4];

    if (NU == 0){
        fprintf(stderr, "Error: NU must be greater than 0\n");
        exit(1);
    }
    //Setting max queue size so it can map shared memory correctly
    if (NZ == 0){
        MAX_QUEUE_SIZE = 1;
    }else {
        MAX_QUEUE_SIZE = NZ;
    }
    
    shared_mem();       // Creating shared memory

    pid_t customer_child_pids[NZ];
    pid_t officer_child_pids[NU];

    //Creating office worker processes
    for(int j = 1; j <= NU; j++){
        int id = fork();
        if (id == 0){
            office_worker(j, TU);
            exit(0);
        }else if(id == -1){
            fprintf(stderr, "Error while creating office worker process (fork error)\n");
            clean_up();
            free(arg_array);
            for (int jj = 0; jj < j; jj++) {    //if fork fails, kill all processes
                kill(officer_child_pids[jj], SIGTERM);
            }
            exit(1);
        } else {
            officer_child_pids[j-1] = id;
        }
    }

    //Creating customer processes
    for(int i = 1; i <= NZ; i++){
        int id = fork();
        if (id == 0){
            customer(i, TZ);
            exit(0);
        }else if(id == -1){
            fprintf(stderr, "Error while creating customer process (fork error)\n");
            clean_up();
            free(arg_array);
            for (int ii = 0; ii < i; ii++) {
                kill(customer_child_pids[ii], SIGTERM);
            }
            exit(1);
        } else {
            customer_child_pids[i-1] = id;
        }
    }

    
    //Generating random sleep time
    srand(time(NULL));
    int sleep_time = (rand() % (F - F/2 + 1)) + F/2;
    usleep(sleep_time * 1000);

    sem_wait(sem);
    fprintf(file, "%d: closing\n", ++(*line));
    *office_closed = 1;
    sem_post(sem);

    //Waiting for all processes to finish
    while (wait(NULL) > 0);
    

    clean_up();     // Cleaning up shared memory
    free(arg_array);

    return 0;
}
