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
            
            //*****************************+
            //*****************************+ CRITICAL AREA FINISH
            //*****************************+
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&lock);
            operation_check =  next_operation(_iterator, _operation->id, &_operation->type, &_operation->num_shares, &_operation->share_price);
        }
    }
    destroy_iterator(_iterator);
}

void* operation_executer(void* args){
    
}

/*
void* operation_executer(void * args){

  //Variables
  struct operation * deqop =(operation *) malloc(sizeof(operation));
  int procheck, deqcheck;
  exec_info * operexec = (exec_info*) args;
  stock_market* curr_market = operexec->market;
  //operexec->exit_mutex;
  //While flag exit not active, start executing operations in the queue
  pthread_mutex_lock(operexec->exit_mutex);
  int* exitop = operexec->exit;
  while(*exitop != 1){
    pthread_mutex_unlock(operexec->exit_mutex);
    //puede que haya un mutex_lock del exit
    pthread_mutex_lock(&queuemutex);
    while(operations_queue_empty(curr_market->stock_operations) == 1){
      pthread_cond_wait(&dequeuecond, &queuemutex);
    }
      //First we dequeue the operation in the top of the queue
      //And we store it in a operation variable "deqop"
      deqcheck = dequeue_operation(curr_market->stock_operations, deqop);
      //Check
      if(deqcheck == -1){
        perror("Error in dequeue_operation()");
        exit(-1);
      }
      //Once we have the operation, we process it
      procheck =  process_operation(curr_market, deqop);
      //Check
      if(procheck == -1){
        perror("Error in process_operation()");
        exit(-1);
      }
      pthread_cond_signal(&enqueuecond);
      pthread_cond_signal(&dequeuecond);
      pthread_mutex_unlock(&queuemutex);
      pthread_mutex_lock(operexec->exit_mutex);
  }
  pthread_mutex_unlock(operexec->exit_mutex);

      //When the flag exit is active, we have to close the Op_Ex,
      //Before closing, we need to process all operations remaining in the queue
  while(operations_queue_empty(curr_market->stock_operations) == 0){
    pthread_mutex_lock(&queuemutex);
      dequeue_operation(curr_market->stock_operations, deqop);
      if(deqcheck == -1){
        perror("Error in dequeue_operation()");
        exit(-1);
      }
      procheck =  process_operation(curr_market, deqop);
      if(procheck == -1){
        perror("Error in process_operation()");
        exit(-1);
      }
      pthread_mutex_unlock(&queuemutex);
    }

}
*/