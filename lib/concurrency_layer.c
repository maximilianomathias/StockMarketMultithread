#include "../include/concurrency_layer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock; 
pthread_cond_t cond; 
pthread_cond_t cond2;
int QueueBusy = 0; // this will determine wather the queue is availabe for the reader to read. 
// int number of readers? 

// initialization of the mutex and condition variables
void init_concurrency_mechanisms(){
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond2, NULL);
    printf("------>init concurrency mechanisns ok \n");
}

// Deallocate memory for the mutex and condition variables 
void destroy_concurrency_mechanisms(){
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
    printf("------>destroy concurrency mechanisns ok \n");
}

void* broker(void* args){

    // we declare the main data structures that the broker need to interact with
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
        printf("------->SUCCESS ITERATOR\n"); // DEBUG ONLY
        // this retruns the muiber of successfull arguments manipulated through fscanf
        operation_check = next_operation(_iterator, _operation->id, &_operation->type, &_operation->num_shares, &_operation->share_price);

        // we keep reading the batch file until we reach the end of the file
        while(operation_check != EOF){ 
            printf("------->OPEARTION CHECK--->%d\n", operation_check);
            printf("------->OPEARTION CHECK--->%s - %d- %d-  %d\n", _operation->id, _operation->type, _operation->num_shares, _operation->share_price);

            new_operation(_newOperation, _operation->id, _operation->type, _operation->num_shares, _operation->share_price);
            printf("------->NEW OPEARTION CHECK--->%s - %d- %d-  %d\n", _newOperation->id, _newOperation->type, _newOperation->num_shares, _newOperation->share_price);
            
            //*****************************+
            //*****************************+ CRITICAL AREA START
            //*****************************+
            pthread_mutex_lock(&lock);

            // we wait while the queue is full. 
            while (operations_queue_full(_stock_marquet->stock_operations) == 1){
                printf("-------->operations_queue_full\n"); // this means that the queue is full 
                pthread_cond_wait(&cond,&lock);
            }
                
            if((enqueue_operation(_stock_marquet->stock_operations, _newOperation))<0)
                return -1; // if it fails, we interrupt the program
            sleep(2);
            //*****************************+
            //*****************************+ CRITICAL AREA FINISH
            //*****************************+
            pthread_cond_signal(&cond);
            pthread_cond_signal(&cond2);
            pthread_mutex_unlock(&lock);
            operation_check =  next_operation(_iterator, _operation->id, &_operation->type, &_operation->num_shares, &_operation->share_price);
        }
    }
    destroy_iterator(_iterator);
}

void* operation_executer(void* args){

    // we declare the main data structures that the broker need to interact with
    struct operation* _operation = (operation *) malloc(sizeof(operation));
    struct exec_info* _operationExec = (exec_info*) args;
    struct stock_market* _stock_marquetEx = _operationExec->market;

    //variables
    int* exit_OpExe = _operationExec->exit;
    int operation_check;
    int proc_operation;

    //*****************************+
    //*****************************+ CRITICAL AREA START
    //*****************************+
    pthread_mutex_lock(_operationExec->exit_mutex);

    while(*exit_OpExe != 1){
        pthread_mutex_unlock(_operationExec->exit_mutex);
        pthread_mutex_lock(&lock);

        while(operations_queue_empty(_stock_marquetEx->stock_operations) == 1){
            pthread_cond_wait(&cond2, &lock);
        }

        operation_check = dequeue_operation(_stock_marquetEx->stock_operations, _operation);
        //Check
        if(operation_check == -1){
            perror("Error in dequeue_operation()");
            exit(-1);
        }
        proc_operation =  process_operation(_stock_marquetEx, _operation);
        //Check
        if(proc_operation == -1){
            perror("Error in process_operation()");
            exit(-1);
        }
        sleep(2);
        //*****************************+
        //*****************************+ CRITICAL AREA FINISH
        //*****************************+
        pthread_cond_signal(&cond);
        pthread_cond_signal(&cond2);
        pthread_mutex_unlock(&lock);
        pthread_mutex_lock(_operationExec->exit_mutex);
    }
    pthread_mutex_unlock(_operationExec->exit_mutex);
    
        //When the flag exit is active, we have to close the Op_Ex,
      //Before closing, we need to process all operations remaining in the queue
    while(operations_queue_empty(_stock_marquetEx->stock_operations) == 0){
        pthread_mutex_lock(&lock);
        dequeue_operation(_stock_marquetEx->stock_operations, _operation);
        if(operation_check == -1){
            perror("Error in dequeue_operation()");
            exit(-1);
        }
        proc_operation =  process_operation(_stock_marquetEx, _operation);
        if(proc_operation == -1){
            perror("Error in process_operation()");
            exit(-1);
        }
        pthread_mutex_unlock(&lock);
        }
}

