//Name: Brian Tagle
//Email: taglebrian@gmail.com
//ID:604907076

#include "SortedList.h"
#include <string.h>
#include <sched.h>


void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
  if(element == NULL)
    {
      return;
    }
  SortedListElement_t* current = list->next;

  while(current != list)
    {
      if(strcmp(element->key, current->key) <= 0)
	{
	  if(opt_yield && INSERT_YIELD)
	    {
	      sched_yield();
	    }
	  break;
	}
      current = current->next;
    }
  element->next = current;
  element->prev = current->prev;
  current->prev = element;
  element->prev->next = element;
}

int SortedList_delete( SortedListElement_t *element)
{
  if(element->key == NULL) //deleting head node is a no no
    {
      return 1;
    }
  if(element->next->prev == element && element->prev->next == element)
    {
      if(opt_yield && DELETE_YIELD)
	{
	  sched_yield();
	}
      element->next->prev = element->prev;
      element->prev->next = element->next;
      return 0;
    }

  return 1;


}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
  SortedListElement_t* current = list->next;

    while(current != list)
    {
      if(strcmp(key, current->key) == 0)
	{
	  return current;
	}
      current = current->next;
      if(opt_yield && LOOKUP_YIELD)
	{
	  sched_yield();
	}
    }
    return NULL;
}

int SortedList_length(SortedList_t *list)
{
  int listLength=0;
  SortedListElement_t* current = list->next;

    while(current != list)
    {
        if(current->next->prev != current && current->prev->next != current)
	  {
	    return -1;
	  }
	listLength++;
	if(opt_yield && LOOKUP_YIELD)
	  {
	    sched_yield();
	  }
	current =current->next;
    }
    return listLength;
}
