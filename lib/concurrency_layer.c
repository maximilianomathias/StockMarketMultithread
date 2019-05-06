#include "../include/concurrency_layer.h"

// GLOBAL VARIABLES
pthread_mutex_t lock; 
pthread_cond_t BrokercCond; 
pthread_cond_t OperationsCond;
pthread_cond_t ReaderCond;
int QueueBusy = 0; // this will determine wather the queue is availabe for the reader to read. 


/* Extract information from data received in the pointer void * args -> OK
Create the iterator on the batch file (new_iterator)
While there are pending file operations
Read a new operation from the file with the iterator (next_operation)
Create a new operation with the information returned by the file (new_operation)
Queue the new operation in the operation queue (enqueue_operation) 
Destroy the iterator (destroy_iterator) */

void* broker(void* args){

    // Declaration of the main data structures that the broker need to interact with
    struct operation* _operation = (operation *) malloc(sizeof(operation));
    struct operation* _newOperation = (operation *) malloc(sizeof(operation));
    struct broker_info* _broker = (broker_info*) args;
    struct stock_market* _stock_marquet = _broker->market;
    struct iterator* _iterator;

    // Declaration of variables 
    char* broker_batch = _broker->batch_file;
    int operation_check;

    //Create an iterator
    if((_iterator = new_iterator(broker_batch))!= NULL){
       
        // Retruns the number of successfull arguments manipulated through fscanf 
        operation_check = next_operation( _iterator, _operation->id, &_operation->type, 
                                            &_operation->num_shares, &_operation->share_price);

        while(operation_check != EOF){ // Reading the batch file until we reach the end of the file 
            new_operation(  _newOperation, _operation->id, _operation->type, 
                            _operation->num_shares, _operation->share_price);
            
            //****************************************************************************************+  CRITICAL AREA START
            pthread_mutex_lock(&lock);
            
            // Waiting while the queue is full. 
            while ( operations_queue_full(_stock_marquet->stock_operations) == 1 )
                pthread_cond_wait(&OperationsCond,&lock);
            QueueBusy = 1; // let the predicate know that the queue is being used.
            // Filling the queue with a new element in the back. Exit he program if returns a negative value
            if((enqueue_operation(_stock_marquet->stock_operations, _newOperation))<0)
                exit(-1);
            
            pthread_cond_signal(&BrokercCond); // Broker conditional variable 
            pthread_cond_signal(&ReaderCond); // Let the reader now that the operation finished 
            pthread_mutex_unlock(&lock); // Release the main mutex. 
            QueueBusy = 0;
            //****************************************************************************************+ CRITICAL AREA FINISH
            operation_check =  next_operation(  _iterator, _operation->id, &_operation->type, 
                                                &_operation->num_shares, &_operation->share_price);
        }//while
    }// if
    // Clore the file and free resoruces
    destroy_iterator(_iterator);
}// broker 


/* Extract information from data received in the pointer void * args 
While the exit flag is not active
View stock market statistics (print_market_status)
Sleep until the next round of information analysis (usleep (frequency)) */
void* stats_reader(void * args){

    //Declaration of the main data structures and variables
    struct reader_info * reader = (reader_info*) args;
    struct stock_market * curr_market = reader->market;
    int *readerExit  = reader->exit;

    pthread_mutex_lock(reader->exit_mutex);
    
    while( *readerExit == 0 ){
        
        pthread_mutex_lock(&lock);
        pthread_mutex_unlock(reader->exit_mutex);

        while(QueueBusy == 1){
            pthread_cond_wait(&ReaderCond,&lock);
        }
        pthread_mutex_unlock(&lock);
        // Execute the query
        print_market_status(curr_market);
        usleep(reader->frequency);

        pthread_mutex_lock(&lock);
        pthread_cond_signal(&BrokercCond); // Notify the Broker that the reading finished
        pthread_cond_signal(&OperationsCond); // Notify the Operation Exec that the reading finished
        pthread_mutex_unlock(&lock); // release the mutex
    }
}



// Extract information from data received in the pointer void * args 
// While the exit flag is not active
// Dequeue operation from the operations queue (dequeue_operation) 
// Processing the operation (process_operation)
void* operation_executer(void* args){

    // we declare the main data structures that the broker need to interact with
    struct operation* _operation = (operation *) malloc(sizeof(operation));
    struct exec_info* _operationExec = (exec_info*) args;
    struct stock_market* _stockMarketEx = _operationExec->market;

    //variables
    int* exit_OpExe = _operationExec->exit;

    
    //****************************************************************************************+ CRITICAL ARE START
    pthread_mutex_lock(_operationExec->exit_mutex); // we lock this mutex in order to have one operation running at the time
    
    while(*exit_OpExe != 1){ // while tehe xit flag is not active
        
        pthread_mutex_unlock(_operationExec->exit_mutex);
        pthread_mutex_lock(&lock);
        
        while( operations_queue_empty(_stockMarketEx->stock_operations) == 1 )
            pthread_cond_wait(&BrokercCond, &lock);
        QueueBusy = 1;

        if( dequeue_operation(_stockMarketEx->stock_operations, _operation)<0)
            exit(-1);
        if( process_operation(_stockMarketEx, _operation)<0)
            exit(-1);
        
        QueueBusy = 0;
        pthread_cond_signal(&ReaderCond);
        pthread_cond_signal(&BrokercCond);
        pthread_mutex_unlock(&lock);
        pthread_mutex_lock(_operationExec->exit_mutex);

        //****************************************************************************************+ CRITICAL AREA FINISH
    }
    pthread_mutex_unlock(_operationExec->exit_mutex);
    
    // Checks if there´s remainin operation to be executed before finishing.
    while(operations_queue_empty(_stockMarketEx->stock_operations) == 0){
        pthread_mutex_lock(&lock);
        
        // Dequeue operation followed process_operation execution, if it fails, we exit the program. 
        if( (dequeue_operation(_stockMarketEx->stock_operations, _operation)<0) || (process_operation(_stockMarketEx, _operation)<0) )
            exit(-1);    
        
        pthread_mutex_unlock(&lock);
    }
}//operation_executer

// initialization of the mutex and condition variables
void init_concurrency_mechanisms(){
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&BrokercCond , NULL);
    pthread_cond_init(&OperationsCond, NULL);
    pthread_cond_init(&ReaderCond, NULL);
}

// Deallocate memory for the mutex and condition variables 
void destroy_concurrency_mechanisms(){
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&BrokercCond);
    pthread_cond_destroy(&OperationsCond);
    pthread_cond_destroy(&ReaderCond);
}
