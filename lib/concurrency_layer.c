
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock; 
pthread_cond_t cond; 
int QueueBusy = 0; // this will determine wather the queue is availabe for the reader to read. 
// int number of readers? 

// initialization of the mutex and condition variables
void init_concurrency_mechanisms(){
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    printf("------>init concurrency mechanisns ok ");
}

// Deallocate memory for the mutex and condition variables 
void destroy_concurrency_mechanisms(){
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
     printf("------>destroy concurrency mechanisns ok ");
}

void* broker(void* args){

    // we declare the main data structures from the header file
   /* struct operations* _operation = (operation *) malloc(sizeof(operation));
    struct operations* _newOperation = (operation *) malloc(sizeof(operation));
    struct broker_info* _broker = (broker_info*) args;
    struct iterator* _iterator;
    struct stock_market* _stock_marquet;

    // variables 
    char* broker_batch = _broker->batch_file;
    _stock_marquet = _broker->market;

    //Create an iterator
    if((_iterator = new_iterator(broker_batch))!= NULL){
        printf("SUCCESS");
    }*/

}