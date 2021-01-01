#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

//global variables
static int next_id = 0;
//semaphore for the id
static sem_t id_sem;

//semaphore for the phone lines
static sem_t connected_lock;
//semaphore for the operators
static sem_t operators;
static int NUM_OPERATORS = 2;
static int NUM_LINES = 5;
//number of phone lines currently connected 
static int connected = 0;

void* phonecall(void* argv);
//this sets a timer for the debate time
void *waitFunction(void *vargp) {
    int time = (int) vargp;
    sleep(time);
}

int main(int argc, char **argv) {
    //if there is no debate time in the standard input
    if(argc == 1) {
        exit(0);
    }
    
    //initialize the semaphore for the operators, lines and id
    sem_init(&operators, 0, NUM_OPERATORS);
    sem_init(&connected_lock, 0, NUM_LINES);
    sem_init(&id_sem, 0, 1);

    
    //this converts the argument in standard input, which is a char, to a int
    int c = atoi(argv[1]);
    
    //this sets and starts the timer for the debate
    pthread_t tidWait;
    pthread_create(&tidWait, NULL, waitFunction, (void *) c);
    
    //this is a pointer to all 200 threads
    pthread_t *threadIds = (pthread_t *) malloc(200);
    for(int i = 0; i < 200; i++) {
        //this creates each thread and calls the phonecall function with it
        int returnThread = pthread_create(&threadIds[i], NULL, phonecall, NULL);
        if(returnThread != 0 ) {
            printf("Error in creating thread\n");
        }
    }
    
    //this waits for the wait thread to complete
    pthread_join(tidWait, NULL);
    
    //destory sempahores at end
    sem_destroy(&operators);
    sem_destroy(&connected_lock);
    sem_destroy(&id_sem);
    

    //this terminates ALL threads
    exit(0);
    
}

void* phonecall(void* argv) {

    //increases the global id variable, and sets the id variable to it using a sempahore
    int id = 0;
    sem_wait(&id_sem);
    next_id++;
    id = next_id;
    sem_post(&id_sem);
   

    printf("Thread %d is attempting to connect …\n", id);
    
    
    //attempting to make a connection to a phone line, access to the connected varibale is in a critical section so a semaphore is used
    sem_wait(&connected_lock);
    while (connected == NUM_LINES) {
        sem_post(&connected_lock);
        //if all lines are busy, it sleeps for 1 seconds and tries again
        sleep(1);
        sem_wait(&connected_lock);
    }
    
    //once a connection is made to a phone line, the connected variable increases
    connected++;
    sem_post(&connected_lock);
    printf("Thread %d connects to an available line, call ringing ...\n", id);
    
    //the line now waits for an operator availble using a semaphore
    sem_wait(&operators);
    printf("Thread %d is speaking to an operator.\n", id);
    //once connected to an operator, it sleeps for a second which represents a question being asked
    sleep(1);
    sem_post(&operators);

    //after the question is asked, the number of lines decreases and the call is disconnected using a semaphore
    printf("Thread %d has proposed a question for candidates! The operator has left …\n", id);
    sem_wait(&connected_lock);
    connected--;
    sem_post(&connected_lock);

    printf("Thread %d has hung up!\n", id);
    
    return NULL;
}
