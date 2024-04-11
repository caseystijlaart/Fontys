/* SD exercise 1: simple linked list API */
/* Author : R. Frenken                   */
/* Version : 1.0			 */
#include "linkedlist.h"
#include <stdlib.h>

/***************************************************/
/* If *list == NULL, a new list should be created, */
/* otherwise prepend a new ITEM with value value   */
/* to the existing list                            */
/* Returns -1 if not enough memory, otherwise 0    */
/***************************************************/
int add_first(ITEM **list, int value)
{
	ITEM *item = (ITEM *) malloc(sizeof(ITEM));
	
	item->value = value;
	item->next = NULL;
	
	if((item == NULL) || (list == NULL))
	{
		return -1;
	}
	
	
	item->next = *list;
	*list = item;
	
	return 0;
}

/***************************************************/
/* If *list == NULL, a new list should be created, */
/* otherwise append a new ITEM with value value to */
/* the existing list                               */
/* Returns -1 if not enough memory, otherwise 0    */
/***************************************************/

int add_last(ITEM **list, int value)
{
	ITEM *item = (ITEM *) malloc(sizeof(ITEM));
	
	item->value = value;
	item->next = NULL;
	
	if((item == NULL) || (list == NULL))
	{
		return -1;
	}
		
	
	if(*list == NULL)			// create new list, when list is empty
	{
		*list = item;
	} else {					// list exists, add item to end of list
		ITEM *lastItem = *list;
		while(lastItem->next != NULL) // move trough list until end list
		{
			lastItem = lastItem->next;
		}
		lastItem->next = item;	// lastitem is now the inserted item
	}
	
	return 0;
}

/****************************************************/
/* Insert a new item after current item c_item      */
/* Returns -1 if not enough memory, c_item == NULL, */
/* *list == NULL or c_item not found, otherwise 0   */
/****************************************************/

int add_after(ITEM *list, ITEM *c_item, int value)
{
    ITEM *item = (ITEM *) malloc(sizeof(ITEM));
    item->value = value;
    
    if((item == NULL) || (c_item == NULL) || (list == NULL))
    {
        return -1;
    } 
    ITEM *lastItem = list;
    while(lastItem != c_item)    // move trough list until c_item is found
    {
        
        if(lastItem->next == NULL)     // if c_item doesn't exists
        { 
            return -1;
        }
        
        lastItem = lastItem->next;
    }
    
    item->next = lastItem->next;
    lastItem->next = item;
            
    return 0;
}


/***************************************************/
/* Remove first item of the list                   */
/* Returns -1 if list==NULL, otherwise 0           */
/***************************************************/

int rem_first(ITEM **list)
{
	ITEM *item = *list;
	
	if(item == NULL)
	{
		return -1;
	}
	 
	*list = item->next;
	
	return 0;
}

/***************************************************/
/* Remove last item of the list                    */
/* Returns -1 if list==NULL, otherwise 0           */
/***************************************************/

int rem_last(ITEM **list)
{
	ITEM *item = *list;
	
	if(item == NULL)
	{
		return -1;
	}
	
	
	while(item->next->next != NULL)    // search trough list, to check if
                                    // c_item exists
    {
        item = item->next;
        
    }
 
    item->next = NULL;            // assign new last item as last item
    
	return 0;
}

/***************************************************/
/* Remove item after c_item of the list            */
/* Returns -1 list==NULL, c_item not found or      */
/* c_item is the last_element                      */
/***************************************************/

int rem_after(ITEM *list, ITEM *c_item)
{
	ITEM *item = list;
	
	if(item == NULL)
	{
		return -1;
	}
	
	while(item != c_item)	// move trough list until c_item is found
	{
		item = item->next;
		if(item->next == NULL) 	// if c_item doesn't exists
		{ 
			return -1;
		}
	}
		
	item->next = item->next->next; 	//assign the next item to item
	list = item->next;
	
	return 0;
}

/*********************************************************/
/* All dynamic memory allocated to the list is freed     */
/*********************************************************/

void clean(ITEM *list)
{
	free(list);
	return;
}

