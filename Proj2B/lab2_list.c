//NAME: Brian Tagle
//EMAIL: taglebrian@gmail.com
//ID: 604907076
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "SortedList.h"


typedef struct {
  pthread_mutex_t mutex_lock;
  int spin_lock;
  SortedList_t list;
} sub_list_t;

int opt_yield = 0;
int numIterations= 1;
int numThreads=1;
int numLists =1;

char sync_lock= 'n';

long long* threadWaitTimes;
SortedListElement_t *element_array;

sub_list_t* sorted_lists;

void handler(int signal)
{
  if (signal == SIGSEGV)
    {
      fprintf(stderr, "got SIGSEGV\n");
      exit(2);
    }
}
void initializeSortedLists() {
  sorted_lists = malloc(sizeof(sub_list_t) * numLists);
  if (sorted_lists == NULL)
    {
      fprintf(stderr,"Error, malloc failure");
      exit(2);
    }

  for (int i = 0; i < numLists; i++)
    {

      SortedList_t* sorted_list = &sorted_lists[i].list;

      sorted_list->key = NULL;
      sorted_list->prev = sorted_list;
      sorted_list->next = sorted_list;

      
      switch(sync_lock) //initialize locks
	{
	case 's':
	  sorted_lists[i].spin_lock = 0;
	  break;
	case 'm':
	  pthread_mutex_init(&sorted_lists[i].mutex_lock, NULL);
	  break;
	default:
	  break;
	}
    }
  return;
}
void initializeElementArray()
{
  element_array = malloc(sizeof(SortedListElement_t) * numThreads*numIterations);
  if (element_array == NULL)
    {
      fprintf(stderr,"Error, malloc failure");
      exit(2);
    }

  //Source: https://www.geeksforgeeks.org/program-generate-random-alphabets/
  srand((unsigned int) time(NULL));
  for ( int i = 0; i < (numThreads*numIterations); i++)
    {
      char *key = malloc(sizeof(char) * (4));
      
      for (int i = 0; i < 3; i++)
	key[i] = (char)rand()% 26 + 'A'; //generate random alphabetic string for the key
      key[3] = '\0';
      element_array[i].key = key;
    }


  return;

}

//Source: https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long hash(const char *str)
{
  unsigned long hash = 5381;
  int c;

  while ( (c = *str++) )
    hash = ((hash << 5) + hash) + c; // hash * 33 + c 

  return ( hash );
}



void* thread_function(void* lower_b) {
    
    int lower_bound =*((int *) lower_b);
    int upper_bound = lower_bound +numIterations -1;
    
    sub_list_t *current_sublist;
    
    
    switch(sync_lock)
      {
      ////////////////////////////////////////////////////////////
      //////////////////////NO SYNC////////////////////////////////
      case 'n': //"no sync"
	{
	  ////INSERT////
	  for ( int i = lower_bound; i <=upper_bound; i++)
	    {

	      current_sublist = &sorted_lists[hash(element_array[i].key) % numLists];
	      SortedList_insert(&current_sublist->list, &element_array[i]);
	    }
	  ////LENGTH////
	  
	  for(int i=0; i<numLists; i++)
	    {
	      if(SortedList_length( &sorted_lists[i].list )  < 0) 
		{
		  fprintf(stderr, "Error, list was found to be corrupted\n"); 
		  exit(2);
		}
	    }
            
	  ////LOOKUP AND DELETE////
	  for ( int i = lower_bound; i <=  upper_bound; i++)
	    {
	      current_sublist = &sorted_lists[hash(element_array[i].key) % numLists];

	      SortedListElement_t *lookup = SortedList_lookup(&current_sublist->list, (&element_array[i])->key);
	      if (lookup == NULL)
		{
		  fprintf(stderr, "Error, expected element not found\n");
		  exit(2);
		}
	      if (SortedList_delete(lookup) == 1)
		{
		  fprintf(stderr, "Error, list was found to be corrupted\n");
		  exit(2);
		}
	    }
	  break;
	}
	//////////////////////////////////////////////////////////
	//////////////////////MUTEX LOCK/////////////////////////////
      case 'm':
	{
	  struct timespec wait, acquired;
	  pthread_mutex_t *mutex_lock;
	  int threadnum = lower_bound / numIterations;
	  ////INSERT////
	  for (int i = lower_bound; i<= upper_bound; i++)
	    {

	      current_sublist = &sorted_lists[hash(element_array[i].key) % numLists];
	      mutex_lock = &current_sublist->mutex_lock;
	      clock_gettime(CLOCK_MONOTONIC, &wait);
	      pthread_mutex_lock(mutex_lock);
	      clock_gettime(CLOCK_MONOTONIC, &acquired);
	     
	      SortedList_insert(&current_sublist->list, &element_array[i]);
	      pthread_mutex_unlock(mutex_lock);
	      
	      threadWaitTimes[threadnum] += ((acquired.tv_sec - wait.tv_sec) * 1000000000)+ acquired.tv_nsec -wait.tv_nsec;
	    }
	
	  ////LENGTH////
	  for(int i=0; i<numLists; i++)
	    {
	      clock_gettime(CLOCK_MONOTONIC, &wait);
	      pthread_mutex_lock(&sorted_lists[i].mutex_lock);
	      clock_gettime(CLOCK_MONOTONIC, &acquired);

	      if(SortedList_length( &sorted_lists[i].list )  < 0) 
		{
		  fprintf(stderr, "Error, list was found to be corrupted\n"); 
		  exit(2);
		}
	      pthread_mutex_unlock(&sorted_lists[i].mutex_lock);

	      threadWaitTimes[threadnum] += ((acquired.tv_sec - wait.tv_sec) * 1000000000)+ acquired.tv_nsec -wait.tv_nsec;
	    }

	  ////LOOKUP AND DELETE////
	  for ( int i = lower_bound; i <= upper_bound; i++)
	    {

	      current_sublist = &sorted_lists[hash(element_array[i].key) % numLists];
	      mutex_lock = &current_sublist->mutex_lock;
	      clock_gettime(CLOCK_MONOTONIC, &wait);
	      pthread_mutex_lock(mutex_lock);
	      clock_gettime(CLOCK_MONOTONIC, &acquired);
	      SortedListElement_t *lookup = SortedList_lookup(&current_sublist->list, (&element_array[i])->key);
	      if (lookup == NULL)
		{
		  fprintf(stderr, "Error, expected element not found\n");
		  exit(2);
		}
	      if (SortedList_delete(lookup) == 1)
		{
		  fprintf(stderr, "Error, list was found to be corrupted\n");
		  exit(2);
		}
	      pthread_mutex_unlock(mutex_lock);

	      threadWaitTimes[threadnum] += ((acquired.tv_sec - wait.tv_sec) * 1000000000)+ acquired.tv_nsec -wait.tv_nsec;
	    }
	  break;
	}
	//////////////////////////////////////////////////////////
      //////////////////////SPIN LOCK/////////////////////////////
      case 's':
	{
	  int *spin_lock;
	  ////INSERT////
	  for (int i =lower_bound; i <= upper_bound; i++)
	    {
	      current_sublist = &sorted_lists[hash(element_array[i].key) % numLists];
	      spin_lock = &current_sublist->spin_lock;
	    
	      while (__sync_lock_test_and_set(spin_lock, 1)){}
	      SortedList_insert(&current_sublist->list, &element_array[i]);
	      __sync_lock_release(spin_lock);
	    }
	
	  ////LENGTH////
	  for(int i=0; i<numLists; i++)
	    {
	      while (__sync_lock_test_and_set(&sorted_lists[i].spin_lock, 1)){}
	      if(SortedList_length( &sorted_lists[i].list )  < 0) 
		{
		  fprintf(stderr, "Error, list was found to be corrupted\n"); 
		  exit(2);
		}
	      __sync_lock_release(&sorted_lists[i].spin_lock);
	    }

	
	  ////LOOKUP AND DELETE////
	  for (int i = lower_bound; i <= upper_bound; i++)
	    {
	      current_sublist = &sorted_lists[ hash(element_array[i].key) % numLists];
	      spin_lock = &current_sublist->spin_lock;
	    
	      while (__sync_lock_test_and_set(spin_lock, 1)){}
	      SortedListElement_t *lookup = SortedList_lookup(&current_sublist->list, (&element_array[i])->key);
	      if (lookup == NULL)
		{
		  fprintf(stderr, "Error, expected element not found\n");
		  exit(2);
		}
	      if (SortedList_delete(lookup) == 1)
		{
		  fprintf(stderr, "Error, list was found to be corrupted\n");
		  exit(2);
		}
	      __sync_lock_release(spin_lock);
	    }
	  break;
	}
      }
    ///////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    return NULL;
}
void freeResources()
{
  if(sync_lock == 'm')
    {
      free(threadWaitTimes);
    }
  free(element_array);
}

int main(int argc, char *argv[]) {
  static struct option list_options[] =
    {
     {"threads", required_argument, 0, 't'},
     {"iterations", required_argument, 0, 'i'},
     {"yield", required_argument, 0, 'y'},
     {"sync", required_argument, 0, 's'},
     {"lists", required_argument, 0, 'l'},
     {0,0,0,0}
    };

  char test_name[20] = "list"; //used to concatenate on other options to get the name of this test
  int optyieldlen;
  
  while(1)
    {
      int arg = getopt_long(argc, argv, "", list_options, NULL);
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
	case 'l':
	  numLists = atoi(optarg);
	  break;
	case 'y':
	  
	  optyieldlen = strlen(optarg);
	  if(optyieldlen > 3)
	    {
	      fprintf(stderr,"Unrecognized argument '%s' provided to --yield, valid arguments are [idl]\n", optarg);

	      exit(1);
	    }
	  for( int i =0; i<optyieldlen; i++ )
	    {
	      switch(optarg[i])
		{
		case 'i':
		  opt_yield += INSERT_YIELD;
		  break;
		case 'd':
		  opt_yield += DELETE_YIELD;
		  break;
		case 'l':
		  opt_yield += LOOKUP_YIELD;
		  break;
		default:
		  fprintf(stderr,"Unrecognized argument '%c' provided to --yield\n", optarg[i]);
		  exit(1);
		}
	    }
	  break;
	case 's':
	  sync_lock = optarg[0];

	  if( ( strcmp(optarg, "m") != 0 ) && ( strcmp(optarg, "s") != 0  ) )
	    {
	      fprintf(stderr,"Unrecognized argument '%s' provided to --sync\n", optarg);
	      exit(1);
	    }

	  break;
	  
	default:
	  fprintf(stderr, "Usage: %s --threads=numThreads --iterations=numIterations --yield=[idl] --sync=[m|s]\n", argv[0]);
	  exit(1);
	  break;
	}
    }
  signal(SIGSEGV, handler);
  
  switch(opt_yield)
    {
    case 0:
      strcat(test_name, "-none");
      break;
    case 1:
      strcat(test_name, "-i");
      break;
    case 2:
      strcat(test_name, "-d");
      break;
    case 3:
      strcat(test_name, "-id");
      break;
    case 4:
      strcat(test_name, "-l");
      break;
    case 5:
      strcat(test_name, "-il");
      break;
    case 6:
      strcat(test_name, "-dl");
      break;
    case 7:
      strcat(test_name, "-idl");
      break;
    }
 
  switch(sync_lock)
    {
    case 'n':
      strcat(test_name, "-none");
      break;
    case 'm':
      strcat(test_name, "-m");

      threadWaitTimes = malloc(sizeof(long long) * numThreads);
      for (int i = 0; i < numThreads; i++)
	{
	  threadWaitTimes[i] = 0;
	}
      
      break;
    case 's':
      strcat(test_name, "-s");
      break;
    }


  
    
    initializeSortedLists();
    initializeElementArray();
    atexit(freeResources);
    
    int bound_list[numThreads];
    for (int i = 0, val = 0; i < numThreads; i++, val += numIterations)
      {
        bound_list[i] = val;
      }
    
    pthread_t thread_id[numThreads];
    
    struct timespec begin;
    clock_gettime(CLOCK_MONOTONIC, &begin);
    
    //CREATE THREADS
    for (int i = 0; i < numThreads; i++)
      {
        
        if (pthread_create(&thread_id[i], NULL, thread_function,  (void *) &bound_list[i]))
	  {
	    fprintf(stderr, "Error with pthread_create\n");

            exit(2);
	  }
      }

    //JOIN THREADS
    for (int i = 0; i < numThreads; i++)
      {
        if (pthread_join(thread_id[i], NULL))
	  {
            fprintf(stderr, "Error with pthread_create\n");

            exit(2);
	  }
      }


    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    for(int i=0; i<numLists; i++)
      {
	if (SortedList_length(&sorted_lists[i].list) != 0)
	  {
	    fprintf(stderr, "Error, length of a list is not zero\n");

	    exit(2);
	  }
      }
    
    long long total_time  = ((end.tv_sec - begin.tv_sec) * 1000000000)+ end.tv_nsec -begin.tv_nsec;

    long long numOperations = numThreads * numIterations * 3;

    long long totalThreadWaitTime=0;
    if(sync_lock == 'm')
      {
	for(int i=0; i<numThreads; i++)
	  {
	    totalThreadWaitTime += threadWaitTimes[i];
	  }
      }
      
    
    printf("%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", test_name, numThreads, numIterations, numLists, numOperations, total_time, total_time / numOperations, totalThreadWaitTime / numThreads * ((numIterations*2 +1)) );

    exit(0);
}
