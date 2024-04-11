#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct element
{
  int addr;
  int size;
  struct element *next;
} ELEMENT;

int add_node(ELEMENT **list, int size, int addr)
{
  ELEMENT *item = (ELEMENT *)malloc(sizeof(ELEMENT));
  ELEMENT *al = *list;

  item->addr = addr;
  item->size = size;
  item->next = NULL;

  if (al == NULL)
  {
    item->next = NULL;
    *list = item;
  }
  else
  {
    if (item->addr < al->addr)
    {
      item->next = *list;
      *list = item;
    }
    else
    {
      while ((item->addr > al->addr) & (al->next != NULL))
      {
        al = al->next;
      }
      item->next = al->next;
      al->next = item;
    }
  }

  return 0;
}

void showList(ELEMENT *list)
/* pre : list = NULL or a linear linked list of ELEMENT structures; the next pointer of the last element is NULL
 * post: All data of the list are printed on the screen
 */
{
  ELEMENT *el = list;
  int i = 0;

  if (list == NULL)
  {
    printf("  <empty>\n");
  }

  /****************************************************************/
  /* Make use of the below described printf statement to print    */
  /* data of an ELEMENT of the list                               */
  /* printf ("%3d:  addr:%4d  size:%4d\n", i, el->addr, el->size);*/
  /****************************************************************/

  while (el != NULL)
  {
    printf("%3d:  addr:%4d  size:%4d\n", i, el->addr, el->size);
    el = el->next;
    i++;
  }
} /* showList */

int freeMemory(ELEMENT **freeList, ELEMENT **allocList, int addr)
/* pre:  Parameter addr must match the start of an allocatd memory block. Otherwhise return -1.
 *
 * post: If addr is a part of allocated memory of allocList then the memory from this address
 * 	     with registered nrofBytes should be removed from the *allocList and added to the *freeList
 *
 *       freeMemory(freeList,allocList,addr) returns number of bytes belongig to the address addr
 *       In all other cases freeMemory(freeList,allocList,addr) returns -1.
 */
{
  /******************************************************/
  /* Here the code to free the memory of address addr   */
  /******************************************************/
  ELEMENT *item = (ELEMENT *)malloc(sizeof(ELEMENT));
  ELEMENT *al = *allocList;
  ELEMENT *fr = *freeList;
  item->addr = addr;
  if ((al == NULL))
  {
    return -1;
  }

  if (al->addr == item->addr)
  {
    item->size = al->size;
    al = al->next;
  }
  else
  {
    if (al->next == NULL)
    {
      return -1;
    }
    else
    {
      while (al->next->addr != item->addr)
      {
        if (al->next->next == NULL)
        {
          return -1;
        }
        al = al->next;
      }
      item->size = al->next->size;
      al->next = al->next->next;
      al = *allocList;
    }
  }

  *allocList = al;

  if (fr->size == 0)
  {
    item->next = NULL;
    *freeList = item;
  }
  else if (item->addr < fr->addr)
  {
    item->next = *freeList;
    *freeList = item;
  }
  else
  {
    if (fr->next != NULL)
    {
      while (item->addr > fr->next->addr)
      {
        fr = fr->next;
        if (fr->next == NULL)
        {
          break;
        }
      }
    }
    item->next = fr->next;
    fr->next = item;
  }

  if ((item->size + item->addr) == fr->addr)
  {
    fr->size += item->size;
    fr->addr = item->addr;
    *freeList = fr;
  }
  if ((fr->size + fr->addr) == item->addr)
  {
    fr->size += item->size;
    fr->next = fr->next->next;
  }
  if (fr->next != NULL)
  {
    if ((fr->size + fr->addr) == fr->next->addr)
    {
      fr->size += fr->next->size;
      fr->next = fr->next->next;
    }
  }
  return item->size;
} /* freeMemory */

int claimMemory(ELEMENT **freeList, ELEMENT **allocList, int nrofBytes)
/* pre : nrofBytes > 0
 * post: If no memory block of nrofBytes available return -1
 *       Otherwise claimMemory(freeList,allocList,nrofBytes) returns the first
 *       address in the *freeList where a memory block of at least nrofBytes is present.
 *       This memory is allocated and not any more available.
 *       The newly allocated block is added to the *allocList and addr of the newly
 *       allocated block is returned
 */
{

  /*********************************************/
  /* Here the code to claim memory             */
  /*********************************************/
  ELEMENT *item = (ELEMENT *)malloc(sizeof(ELEMENT));
  ELEMENT *al = *allocList;
  ELEMENT *fr = *freeList;
  item->size = nrofBytes;

  if (fr == NULL)
  {
    return -1;
  }

  if (nrofBytes > 0)
  {
    if(item->size < fr->size)
    {
    	item->addr = fr->addr;
    	fr->addr += item->size;
    	fr->size = (fr->size) - (item->size);
    } else 
    {
    	if(fr->next != NULL)
    	{
    		if (item->size < fr->next->size) 
    		{
    			item->addr = fr->addr;
    			fr->addr += item->size;
    			fr->size = (fr->size) - (item->size);

				if(fr->next != NULL)
				{
    				if (fr->size == 0)
    				{
      					fr = fr->next;
      					*freeList = fr;
    				}
    			}
    		} else 
    		{
    			while (item->size > fr->next->size)
    			{
      				if (fr->next->next == NULL)
      				{
        				return -1;
      				}
      				fr = fr->next;
    			}
    			item->addr = fr->next->addr;
    			fr->next->addr += item->size;
    			fr->next->size = (fr->next->size) - (item->size);

				if(fr->next->next != NULL)
				{
    				if (fr->next->size == 0)
    				{
      					fr->next = fr->next->next;
    				}
    			}
    			
    		}
    	} else {
    		item->addr = fr->addr;
    		fr->addr += item->size;
    		fr->size = (fr->size) - (item->size);
    	}
    }

    if (al == NULL)
    {
      item->next = NULL;
      *allocList = item;
    }
    else
    {
      if (item->addr < al->addr)
      {
        item->next = *allocList;
        *allocList = item;
      } else
      {
      	if(al->next == NULL)
      	{	
      		al->next = item;
      	} else if ( al->next->next == NULL)
      	{
      		if(item->addr > al->next->addr)
      		{
      			al->next->next = item;
      		} else 
      		{
      			item->next = al->next;
      			al->next = item;
      		}
      	} else 
      	{
        	while ((item->addr > al->next->addr) & (al->next->next != NULL))
        	{
          	al = al->next;
        	}
        	item->next = al->next;
        	al->next = item;
        }
      }
    }
  }
  return item->addr;
} /* claimMemory */

void cleanList(ELEMENT **list)
/* pre : list = NULL or a linear linked list of ELEMENT structures; the next pointer of the last element is NULL
 * post: the memory of the linked list associated with **list is cleaned
 */
{
  /***************************************/
  /* Here the code to clean the list     */
  /***************************************/
  ELEMENT *el = *list;
  ELEMENT *af = NULL;

  while (el != NULL)
  {
    af = el->next;
    free(el);
    el = af;
  }

  el = NULL;
}
