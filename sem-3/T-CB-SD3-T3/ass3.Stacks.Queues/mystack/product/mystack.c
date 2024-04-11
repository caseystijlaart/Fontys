
#include "mystack.h"
#include "logging.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
/* The stack is an abstract data type.
* this means that the internal structures are
* never exposed to the users of the library.
*/

/* Note: the stacks have no knowledge of what types
* they are storing. This is not a concern of the stack
*/

StackMeta_t *mystack_create(size_t objsize)
{
	if((objsize < 0) )
	{
		return NULL;
	}
	
    StackMeta_t *st = (StackMeta_t *)malloc(sizeof(StackMeta_t));
     st->objsize = objsize;
     st->numelem = 0;    
     st->stack = NULL;
     return st;
}

int mystack_push(StackMeta_t *stack, void* obj)
{
    if((obj == NULL) || (stack == NULL))
    {
        return -1;
    }
    
    StackObject_t *item = (StackObject_t *)malloc(sizeof(StackObject_t));
    item->obj = malloc(stack->objsize);
    memcpy(item->obj, obj, stack->objsize);
    
    if(stack->stack == NULL)
    {
        stack->stack = item;
    } else {
		item->next = stack->stack;
        stack->stack = item;
    }
    stack->numelem++;
    return 0;
}

int mystack_pop(StackMeta_t *stack, void* obj)
{
    if((stack == NULL) || (stack->stack == NULL))
    {
        return -1;
    }

    obj = malloc(stack->objsize);
    memcpy(obj, stack->stack->obj, stack->objsize);
    
    stack->stack = stack->stack->next;
    free(obj); 
    stack->numelem--;
    return 0;
}

void mystack_destroy(StackMeta_t *stack)
{
	if(stack == NULL)
	{
		return;
	}
	StackObject_t *item = stack->stack;
    while(item != NULL)
    {
        stack->stack = stack->stack->next;
        free(item->obj);
        free(item);
        if(stack->stack == NULL)
        {
          break;
        }
        item = stack->stack;
        stack->numelem--;
    }
    free(stack->stack);
    free(stack);
}

int mystack_nofelem(StackMeta_t *stack)
{
	return stack->numelem;
}
