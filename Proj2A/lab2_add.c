//Name: Brian Tagle
//Email: taglebrian@gmail.com
//ID: 604907076

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>


int opt_yield=0;
//char sync_lock = 'n';
int numIterations = 1;
long long counter = 0;

pthread_mutex_t mutex_lock;
int spin_lock= 0;

void add(long long *pointer, long long value)
{
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}

void addfor_numIterations(int value, void* sync_lock) {
  int int_sync_lock = *((int*)sync_lock);
  long long new, old;
  
  for (int i = 0; i < numIterations; i++) {
    switch (int_sync_lock) {
    case 'm':
      pthread_mutex_lock(&mutex_lock);
      add(&counter, value);
      pthread_mutex_unlock(&mutex_lock);
      break;
    case 's':
      while (__sync_lock_test_and_set(&spin_lock, 1)){}
      add(&counter, value);
      __sync_lock_release(&spin_lock);
      break;
    case 'c':
      do
	{
	  old = counter;
	  new = counter + value;
	  if (opt_yield)
	    sched_yield();
	} while (__sync_val_compare_and_swap(&counter, old, new) != old);
      break;
    case 'n':
      add(&counter, value);
      break;

    }
  }
  return;
}

void* thread_function(void* sync_lock)
{
  //int sync_lock = atoi(lock);
  //fprintf(stderr, "got to run_thread");
  addfor_numIterations(1, &sync_lock);
  addfor_numIterations(-1, &sync_lock);
  return NULL;
}

int main(int argc, char** argv)
{
  
  static struct option add_options[] =
    {
     {"threads", required_argument, 0, 't'},
     {"iterations", required_argument, 0, 'i'},
     {"yield", no_argument, 0, 'y'},
     {"sync", required_argument, 0, 's'},
     {0,0,0,0}
    };

    

  int numThreads = 1;
  char test_name[20] = "add";
  char sync_lock = 'n';
  //int numIterations = 1;
  //long long counter = 0;
  while(1)
    {
      int arg = getopt_long(argc, argv, "", add_options, NULL);
      if (arg == -1)
	{
	  break;
	}
      switch(arg)
	{
	case 't':
	  numThreads = atoi(optarg);
	  break;
	case 'i':
	  numIterations = atoi(optarg);
	  break;
	case 'y':
	  opt_yield = 1;

	  break;
	case 's':
	  sync_lock = optarg[0];

	  if(( strcmp(optarg, "m") != 0 ) && ( strcmp(optarg, "s") != 0 ) && ( strcmp(optarg, "c") != 0 ))
	    {
	      fprintf(stderr,"Unrecognized argument '%s' provided to --sync\n", optarg);
	      exit(1);
	    }

	  break;
	default:
	  fprintf(stderr, "Usage: %s --threads=numThreads --iterations=numIterations --yield --sync=[m|s|c]\n", argv[0]);
	  exit(1);
	}
    }

  if (opt_yield)
    strcat(test_name, "-yield");
  switch(sync_lock)
    {
    case 'n':
      strcat(test_name, "-none");
      break;
    case 'm':
      strcat(test_name, "-m");
      pthread_mutex_init(&mutex_lock, NULL);
      break;
    case 's':
      strcat(test_name, "-s");
      break;
    case 'c':
      strcat(test_name, "-c");
      break;
    }
  /*
  if (sync_lock == 'n')
    strcat(test_name, "-none");
  else if (sync_lock == 'm')
    {
    pthread_mutex_init(&mutex_lock, NULL); 
    strcat(test_name, "-m");
    }
  else if (sync_lock == 's')
    strcat(test_name, "-s");
  else if (sync_lock == 'c')
    strcat(test_name, "-c");
  */

  
  struct timespec begin;
  clock_gettime(CLOCK_MONOTONIC, &begin);

  pthread_t thread_id[numThreads];
  //void* parameter_lock = &sync_lock;
  for( int i=0; i < numThreads; i++)
    {
      if(pthread_create(&thread_id[i], NULL, thread_function, (void*)&sync_lock))
	{
	  fprintf(stderr, "Error with pthread_create\n");
;
	  exit(1);
	}   
    }
  
  for( int i=0; i < numThreads; i++)
    {
      if(pthread_join(thread_id[i], NULL))
	{
	  fprintf(stderr, "Error with pthread_join\n");

	  exit(1);
	}
    }

  struct timespec end;
  clock_gettime(CLOCK_MONOTONIC, &end);
  long long numOperations = numThreads* numIterations* 2;

  long long total_time  = ((end.tv_sec - begin.tv_sec) * 1000000000)+ end.tv_nsec -begin.tv_nsec;

  printf("%s,%d,%d,%lld,%lld,%lld,%lld\n", test_name, numThreads, numIterations, numOperations, total_time, total_time / numOperations, counter);

  exit(0);
}


