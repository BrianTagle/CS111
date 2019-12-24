//Name: Brian Tagle
//EMAIL: taglebrian@gmail.com
//ID: 604907076
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "SortedList.h"


int opt_yield = 0;
int numIterations= 1;
int numThreads=1;

int spin_lock= 0;
char sync_lock= 'n';
pthread_mutex_t mutex_lock;
SortedList_t sorted_list;
SortedListElement_t *element_array;


void initializeSortedList() {
  //initialize head of list
  sorted_list.key = NULL;
  sorted_list.prev = &sorted_list;
  sorted_list.next = &sorted_list;

  //create random keys to be inserted
  srand((unsigned int) time(NULL));
  for ( int i = 0; i < (numThreads*numIterations); i++)
    {
      char *key = malloc(sizeof(char) * (4));
      
      for (int i = 0; i < 3; i++)
	key[i] = (char)rand()% 26 + 'A'; //generate random alphabetic string for the key
      key[3] = '\0';
      element_array[i].key = key;
    }

}


void* thread_function(void* lower_b) {

  unsigned int lower_bound = *((int *) lower_b);
  unsigned int upper_bound = lower_bound + numIterations - 1;
  switch (sync_lock)
    {
      ////////////////////////////////////////////////////////////
      //////////////////////NO SYNC////////////////////////////////
    case 'n': //"no sync"
      ////INSERT////
      for (unsigned int i = lower_bound; i <= upper_bound; i++)
	{
	  SortedList_insert(&sorted_list, &element_array[i]);
	}
      ////LENGTH////
      if(SortedList_length(&sorted_list) < 0) 
	{
	  fprintf(stderr, "Error, list was found to be corrupted\n"); 
	  exit(2);
	}
      ////LOOKUP AND DELETE////
      for (unsigned int i = lower_bound; i <= upper_bound; i++)
	{
	  SortedListElement_t *lookup = SortedList_lookup(&sorted_list, element_array[i].key);
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
      
      //////////////////////////////////////////////////////////
      //////////////////////MUTEX LOCK/////////////////////////////
    case 'm': //mutex lock
      ////INSERT////
      for (unsigned int i = lower_bound; i <= upper_bound; i++) 
	{
	  pthread_mutex_lock(&mutex_lock);
	  SortedList_insert(&sorted_list, &element_array[i]);
	  pthread_mutex_unlock(&mutex_lock);
	}

      ////LENGTH////
      pthread_mutex_lock(&mutex_lock);
      if(SortedList_length(&sorted_list) < 0)
	{
	  fprintf(stderr, "Error, list was found to be corrupted\n");
	  exit(2);
	}
      pthread_mutex_unlock(&mutex_lock);
	
      ////LOOKUP AND DELETE////
      for (unsigned int i = lower_bound; i <= upper_bound; i++) 
	{
	  pthread_mutex_lock(&mutex_lock);
	  //LOCKED
	  SortedListElement_t *lookup = SortedList_lookup(&sorted_list, element_array[i].key);
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
	  //UNLOCK
	  pthread_mutex_unlock(&mutex_lock);
	}
      
      break;
      
      //////////////////////////////////////////////////////////
      //////////////////////SPIN LOCK/////////////////////////////
    case 's': //spin lock
      ////INSERT////
      for (unsigned int i = lower_bound; i <= upper_bound; i++) 
	{
	  while (__sync_lock_test_and_set(&spin_lock, 1)){}
	  SortedList_insert(&sorted_list, &element_array[i]);
	  __sync_lock_release(&spin_lock);
	}
      
      ////LENGTH////
      while (__sync_lock_test_and_set(&spin_lock, 1)) {}
      if(SortedList_length(&sorted_list) < 0) 
	{
	  fprintf(stderr, "Error, list was found to be corrupted\n");
	  exit(2);
	}
      __sync_lock_release(&spin_lock);


      ////LOOKUP AND DELETE////
      for (unsigned int i = lower_bound; i <= upper_bound; i++) 
	{
	  while (__sync_lock_test_and_set(&spin_lock, 1)){}

	  SortedListElement_t *lookup = SortedList_lookup(&sorted_list, element_array[i].key);
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
	  
	  __sync_lock_release(&spin_lock);
	}
      break;
      ////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////
    }

  return NULL;
}

int main(int argc, char *argv[]) {

  
    static struct option list_options[] =
    {
     {"threads", required_argument, 0, 't'},
     {"iterations", required_argument, 0, 'i'},
     {"yield", required_argument, 0, 'y'},
     {"sync", required_argument, 0, 's'},
     {0,0,0,0}
    };

    //int numThreads =1;
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
	pthread_mutex_init(&mutex_lock, NULL);
	break;
      case 's':
	strcat(test_name, "-s");
	break;
      }


    element_array = malloc(sizeof(SortedListElement_t) * numThreads*numIterations);


    initializeSortedList();

    int bound_list[numThreads];
    
    for (int i = 0, val = 0; i < numThreads; i++, val += numIterations)
        bound_list[i] = val;
 
    struct timespec begin;
    clock_gettime(CLOCK_MONOTONIC, &begin);


    pthread_t thread_id[numThreads];
    
    //CREATE THREADS
    for (int i = 0; i < numThreads; i++)
      {
        
        if (pthread_create(&thread_id[i], NULL, thread_function, (void *) &bound_list[i]))
	  {
	    fprintf(stderr, "Error with pthread_create\n");
  
            free(element_array);
            exit(2);
	  }
      }

    //JOIN THREADS
    for (int i = 0; i < numThreads; i++)
      {
        if (pthread_join(thread_id[i], NULL))
	  {
            fprintf(stderr, "Error with pthread_create\n");
;
            free(element_array);
            exit(2);
	  }
      }


    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    
 

    if (SortedList_length(&sorted_list) != 0)
      {
	fprintf(stderr, "Error, length of the list is not zero\n");

        free(element_array);
        exit(2);
      }


    int numLists= 1;
    long long numOperations = numThreads* numIterations* 3;

    long long total_time  = ((end.tv_sec - begin.tv_sec) * 1000000000)+ end.tv_nsec -begin.tv_nsec;

    printf("%s,%d,%d,%d,%lld,%lld,%lld\n", test_name, numThreads, numIterations, numLists, numOperations, total_time, total_time / numOperations);
;
    free(element_array);
    exit(0);
}
