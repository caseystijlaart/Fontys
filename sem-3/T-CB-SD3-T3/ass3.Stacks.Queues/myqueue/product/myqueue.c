#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "mystack.h"
#include "myqueue.h"

QueueMeta_t *myqueue_create(int item_size)
{
	if(item_size < 0)
	{
		return NULL;
	}
	
	QueueMeta_t *queue = (QueueMeta_t *)malloc(sizeof(QueueMeta_t));
	queue->item_size = item_size;
	StackMeta_t *stack1 = mystack_create(item_size);
	StackMeta_t *stack2 = mystack_create(item_size);
	queue->stack_in = stack1;
	queue->stack_out = stack2;
    return queue;
}

void myqueue_delete(QueueMeta_t *queue)
{
	mystack_destroy(queue->stack_in);
	mystack_destroy(queue->stack_out);
	free(queue);
}

int myqueue_enqueue(QueueMeta_t *que, void *obj)
{
	if(obj == NULL)
	{ 
		return -1;
	}
	
	StackObject_t *item = (StackObject_t *)malloc(sizeof(StackObject_t));
    item->obj = malloc(que->item_size);
    memcpy(item->obj, obj, que->item_size);
    mystack_push(que->stack_in, obj);
    return 0;
}

int myqueue_dequeue(QueueMeta_t *queue, void *obj)
{
	if(queue == NULL)
    {
		return -1;
	}
	
    if((queue->stack_in->stack == NULL)&&(queue->stack_out->stack == NULL))
    { 
        return -1;
    }
    
    if(queue->stack_out->stack == NULL)
    {
        while(queue->stack_in->stack != NULL)
        {
            StackObject_t *item = (StackObject_t *)malloc(sizeof(StackObject_t));
            item->obj = malloc(queue->item_size);
            memcpy(item->obj, queue->stack_in->stack->obj, queue->item_size);
            mystack_pop(queue->stack_in, obj);
            mystack_push(queue->stack_out, item->obj);
        }
    }
    
    mystack_pop(queue->stack_out, obj);
    
    return 0;
}




