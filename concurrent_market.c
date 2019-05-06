#include "../include/concurrency_layer.h"

#define BROKERS 0
#define OPERATORS 1
#define STATS_READERS 1

int main(int argc, char * argv[]){

	pthread_t tidBrokers[BROKERS];
	pthread_t tidOpExec[OPERATORS];
	pthread_t tidStatReaders[STATS_READERS];
	pthread_mutex_t exit_mutex;

	stock_market market_madrid;
	int exit = 0;

	// Init market and concurrency mechanisms
	init_market(&market_madrid, "stocks.txt"); // sets all the values to zero. 
	init_concurrency_mechanisms(); // initialization of the mutex and condition variables
	pthread_mutex_init(&exit_mutex,NULL); 
	
	// Init broker_info structure for the broker thread
	broker_info info_b1;
	strcpy(info_b1.batch_file, "batch_operations.txt");
	info_b1.market = &market_madrid;

	// Init exec_info structure for the operation_executer thread
	exec_info info_ex1;
	info_ex1.market = &market_madrid;
	info_ex1.exit = &exit;
	info_ex1.exit_mutex = &exit_mutex;
	
	// Init reader_info for the stats_reader thread
	reader_info info_re1;
	info_re1.market = &market_madrid;
	info_re1.exit = &exit;
	info_re1.exit_mutex = &exit_mutex;
	info_re1.frequency = 100000;

	
	// Create broker 
	for(int i = 0; i < BROKERS; i++)
		pthread_create(&(tidBrokers[i]), NULL, &broker, (void*) &info_b1);

	// Create Operation Exec 
	pthread_create(&(tidOpExec[0]), NULL, &operation_executer, (void*) &info_ex1);

	// Create Stats Readers
	for(int i = 0; i < STATS_READERS; i++)
		pthread_create(&(tidStatReaders[i]), NULL, &stats_reader, (void*) &info_re1);

	void * res;
	
	for(int i = 0; i < BROKERS; i++) // Join broker threads
		pthread_join(tidBrokers[i],&res);
	
	pthread_mutex_lock(&exit_mutex);
	exit = 1;	// Put exit flag = 1 after brokers completion
	pthread_mutex_unlock(&exit_mutex);
	
	pthread_join(tidOpExec[0],&res); // Join the Operation executer threads

	for(int i = 0; i < STATS_READERS; i++) // Join readers threads
		pthread_join(tidStatReaders[i], &res);

	print_market_status(&market_madrid); // Print final statistics of the market
	
	delete_market(&market_madrid); // Destroy market and concurrency mechanisms
	destroy_concurrency_mechanisms(); // Destroy the threads
	pthread_mutex_destroy(&exit_mutex); // Destroy the mutex used in the main function
	

	return 0;
}
