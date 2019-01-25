//Maharshi Shah
//U13381853
//COP 4600 Operating Systems
//Proj 3: October 14, 2018

#define _REENTRANT
#define BUFFER_SIZE 15
#define SHMKEY ((key_t) 2700)

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

void* produce(void *arg);
void* consume(void *arg);

typedef struct { char* value; } sbuffer;
typedef struct { int value; } scounter;

//declaring three semaphores for empty, full and critical section of threads/process
sem_t empty;
sem_t full;
sem_t critical;

int start = 0;
int end = 0;
char newChar;

FILE* infile;

sbuffer *buffer;
scounter *counter;


int main(void)
{
	infile = fopen("mytest.dat", "r"); //open file for read operation
	
	int i;
	int shmid;
	pthread_t producer[1];
	pthread_t consumer[1];
	pthread_attr_t attr;
	
	char *shmadd;
	shmadd = (char *) 0;
	
	if ((shmid = shmget (SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
	{
		perror("shmget");
		return 1;

	}
	
	if ((buffer = (sbuffer *) shmat (shmid, shmadd, 0)) == (sbuffer *) - 1)	    
	{
		perror("shmat");
		return 0;
	}
	
	char buffer_array[15]; //circular buffer with 15 positions which will be filled one at a time
	buffer->value = buffer_array;

	counter = (scounter *) malloc(sizeof(scounter));
	counter->value = 0;
	
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	sem_init(&critical,0,1);
	
	fflush(stdout);
	
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	
	pthread_create(&producer[0], &attr, produce, 0);
	pthread_create(&consumer[0], &attr, consume, 0);

	pthread_join(producer[0], 0);
	pthread_join(consumer[0], 0);

	sem_destroy(&empty);//destroying semaphores after buffer full and file has been read completely
	sem_destroy(&full);
	sem_destroy(&critical);

	if ((shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0)) == -1)
	{
		perror("shmctl");
		return -1;
	}

	fclose(infile);
	
	printf("Parent Counter: %d\n\n", counter->value);

	printf("\n\t\t End of Simulation\n");


	return 0;
}

//Thread 1: Producer thread with binary semaphores to release and lock. Fills an empy slot in the bounded buffer
void* produce(void *arg)
{
	bool done = false;
	char value;
	
	while(!done)
	{
		sem_wait(&empty);
		sem_wait(&critical);
		end++;
	
	if(fscanf(infile, "%c", &newChar) != EOF)
	{
		buffer->value[(end) % 15] = newChar;
		printf("Produced: %c\n", newChar);
	}

	else
	{
		buffer->value[(end) % 15] = '*';
		done = true;
	}
	
	sem_post(&critical);
	sem_post(&full);
	}
}

//Thread 2: Consumer thread with binary semaphores that remove data from the bounded buffer		
void* consume(void *arg)
{
	bool done = false;
	char value;
	
	while(!done)
      {
	sem_wait(&full);
	sem_wait(&critical);
	start++;
	
	sleep(1);

	if((value = buffer->value[(start) % 15]) != '*')
	{
		printf("Consumed: %c\n", value);
		counter->value++;
	}
	else
	{
		done = true;
	}
	sem_post(&critical);
	sem_post(&empty);
      }
}
	

	

	
