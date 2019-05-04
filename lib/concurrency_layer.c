#include "../include/concurrency_layer.h"
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
    printf("------>init concurrency mechanisns ok \n");
}

// Deallocate memory for the mutex and condition variables 
void destroy_concurrency_mechanisms(){
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
     printf("------>destroy concurrency mechanisns ok \n");
}

void* broker(void* args){

    // we declare the main data structures from the header file
    struct operation* _operation = (operation *) malloc(sizeof(operation));
    struct operation* _newOperation = (operation *) malloc(sizeof(operation));
    struct broker_info* _broker = (broker_info*) args;
    struct iterator* _iterator;
    struct stock_market* _stock_marquet;

    // variables 
    char* broker_batch = _broker->batch_file;
    int operation_check;
    _stock_marquet = _broker->market;

    //Create an iterator
    if((_iterator = new_iterator(broker_batch))!= NULL){
        printf("------->SUCCESS\n"); // DEBUG ONLY
        operation_check = next_operation(_iterator, _operation->id, &_operation->type, &_operation->num_shares, &_operation->share_price);
        printf("------->OPEARTION CHECK--->%d\n", operation_check);
        new_operation(_newOperation, _operation->id, _operation->type, _operation->num_shares, _operation->share_price);
        pthread_mutex_lock(&lock);

        while (operations_queue_full(_stock_marquet->stock_operations) == 1)
            pthread_cond_wait(&cond,&lock);
        
        enqueue_operation(_stock_marquet->stock_operations, _newOperation);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);

    }
    destroy_iterator(_iterator);
}

