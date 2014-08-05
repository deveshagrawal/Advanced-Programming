#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
//Message to pass arguments to pthread_create
struct Message
{
  int id;
  int pNum;
  int eNum;
};
// Message ptr
struct Message* msgPtr;
// Status whether chopstick[i] was occupied
int* status;
// mutex to provide mutual exclusion
pthread_mutex_t mutex;
// condition variable for chopstick
pthread_cond_t* chopstick;
// callback for each philosopher
void* philosopher(void*);
// Eat
void eat(int);
// Think
void think(int);
 
int main(int argc, char** argv)
{
  // Check arguments
  if(argc != 3)
  {
    printf("Please enter philosopher number and eat number\n");
    return 1;
  }
  // Philosopher number
  int threadNum = atoi(argv[1]);
  if(threadNum <= 2)
  {
    printf("Philosopher number must be larger than 2\n");
    return 1;
  }
  // Eat times
  int eatNum = atoi(argv[2]);
  if(eatNum <= 0)
  {
    printf("Philosopher eat number must be larger than 0\n");
    return 1;
  }
  // malloc a int array to record whether chopstick was used
  status = (int*)malloc(sizeof(int)*threadNum);
  if(!status)
  {
    printf("malloc error\n");
    return 1;
  }
  // Set chopstick flag to 1 which means chopstick[i] is available
  for(int i = 0; i < threadNum; i++)
  {
    status[i] = 1;
  }
  // malloc pthread_t array for different threads
  pthread_t* tidPtr = NULL; // thread ptr
  tidPtr = (pthread_t*)malloc(sizeof(pthread_t)*threadNum);
  if(!tidPtr)
  {
    printf("malloc error\n");
    return 1;
  }
  // malloc message struct for passing parameters of different threads
  msgPtr = (struct Message*)malloc(sizeof(struct Message)*threadNum);
  if(!msgPtr)
  {
    printf("malloc error\n");
    return 1;
  }
  // malloc pthread_cond_t for different threads
  chopstick = (pthread_cond_t*)malloc(sizeof(pthread_cond_t)*threadNum);
  if(!chopstick)
  {
    printf("malloc error\n");
    return 1;
  }
  // initialize mutex
  pthread_mutex_init(&mutex, NULL);
  for(int i = 0; i < threadNum; i++)
  {
     pthread_cond_init(&chopstick[i], NULL);
  }
  // Initialize message content
  for(int i = 0; i < threadNum; i++)
  {
    //Create thread
    msgPtr[i].id = i;
    msgPtr[i].pNum = threadNum;
    msgPtr[i].eNum = eatNum;
    pthread_create(&tidPtr[i], NULL, philosopher, (void*)&msgPtr[i]); 
  }
  // Wait for threads termination
  for(int i = 0; i < threadNum; i++)
  {
    pthread_join(tidPtr[i], NULL);
  }
  // Destory mutex and condition variables
  pthread_mutex_destroy(&mutex);
  for(int i = 0; i < threadNum; i++)
  {
    pthread_cond_destroy(&chopstick[i]);
  }
  free(tidPtr);
  
  return 0;
}
// Philosepher callback for each thread
void* philosopher(void* args)
{
  struct Message* m = (struct Message*)args;
  int id = m->id; // Philosepher id
  int count = m->pNum; // Philosepher numbers
  int num = m->eNum; // Eat times
  int nextId = (id+1+count)%count; // right chopstick id
  int sTime = 0; // sleep time
  
  srand(time(0));
  
  for(int i = 0; i < num; i++)
  {
    sTime = (rand()+id+count)%count;
    sleep(sTime); // sleep a randome time
    
    think(id); // think
    
    pthread_mutex_lock(&mutex); // lock mutex
    // Wait when the left chopstick is not available
    while(status[nextId] == 0) 
    {
       pthread_cond_wait(&chopstick[nextId], &mutex);
    }
    status[nextId] = 0;
    // Wait when the right chopstick is not available
    while(status[id] == 0)
    {
       pthread_cond_wait(&chopstick[id], &mutex);
    }
    status[id] = 0;
    // Unlock mutex for other philosepher to obtain the chopsticks
    pthread_mutex_unlock(&mutex); 
    // Eat
    eat(id);
    status[nextId] = 1;
    status[id] = 1;
    // Return chopsticks and notify other philosephers
    pthread_cond_signal(&chopstick[nextId]);
    pthread_cond_signal(&chopstick[id]);
    
    sTime = (rand()+id+count)%count;
    sleep(sTime);
  }
  
  return (void*)NULL;
}
// Eat
void eat(int id)
{
  printf("Philosopher %d eating\n", id);
}
// Think
void think(int id)
{
  printf("Philosopher %d thinking\n", id);
}