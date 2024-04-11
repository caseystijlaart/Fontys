/**
 * @file myStack.ccp
 * @author Casey Stijlaart
 * @brief Source file for the myStack.h file.
 * @version 0.1
 * @date 2021-11-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

using namespace std;

#include "myStack.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int Stack::assignSize(size_t size)
{
    if (size <= 0)
    {
        return -1;
    }
    objsize = size;
    return objsize;
}

int Stack::push(void *data)
{
    if (data == 0)
    {
        return -1;
    }
    StackObject_t *ptr = new StackObject_t();
    ptr->data = data;
    ptr->next = stack;
    stack = ptr;
    numelem++;
    return 0;
}

int Stack::pop(void *obj)
{
    if (stack == 0 || stack == nullptr)
    {
        return -1;
    }

    
    obj = malloc(objsize);
    memcpy(obj, stack->data, objsize);
    stack = stack->next;
    free (obj);
    numelem--;
    return 0;
}

int Stack::empty()
 {
    StackObject_t *obj = NULL;
    if (stack == 0)
    {
        return -1;
    }

    StackObject_t *item = NULL;
    while (stack != 0)
    {
        pop(&item);
    }
    return 0;
}

int Stack::numElem()
{
    return numelem;
}
