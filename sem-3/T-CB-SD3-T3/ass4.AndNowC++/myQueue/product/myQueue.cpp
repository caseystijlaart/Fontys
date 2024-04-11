/**
 * @file myQueue.ccp
 * @author Casey Stijlaart
 * @brief Source file for the myQueue.h file.
 * @version 0.1
 * @date 2021-11-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

using namespace std;

#include "myQueue.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>

int Queue::assignSize(int size)
{
    if (size <= 0)
    {
        return -1;
    }

    item_size = size;
    stackIn.assignSize(size);
    stackOut.assignSize(size);
    return item_size;
}

int Queue::enqueue(void *data)
{
    if (data == NULL)
    {
        return -1;
    }

    stackIn.push(data);
    return 0;
}

int Queue::dequeue(void * obj)
{
    if ((stackIn.stack == NULL) && (stackOut.stack == NULL))
    {
        return -1;
    }

    StackObject_t *item = NULL;
    obj = malloc(item_size);
    if (stackOut.stack == NULL)
    {
        while (stackIn.stack != NULL)
        {   
            StackObject_t *item = (StackObject_t *)malloc(sizeof(StackObject_t));
            item->data = malloc(item_size);
            memcpy(item->data, stackIn.stack, item_size);
            stackIn.pop(obj);
            stackOut.push(item->data);
        }
    }
    stackOut.pop(item);
    return 0;
}

int Queue::destroy()
{
    if ((stackIn.stack == NULL) && (stackOut.stack == NULL))
    {
        return -1;
    }

    Stack st;
    if (stackIn.stack != NULL)
    {

        st.empty();
    }

    if (stackOut.stack != NULL)
    {
        st.empty();
    }

    return 0;
}