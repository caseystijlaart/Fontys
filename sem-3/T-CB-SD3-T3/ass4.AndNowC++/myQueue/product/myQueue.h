/**
 * @file myQueue.h
 * @author Casey Stijlaart
 * @brief Header file for the c++ Queue.
 * @version 0.1
 * @date 2021-11-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

using namespace std;

#ifndef __MYQUEUEH__
#define __MYQUEUEH__

#include <iostream>
#include "myStack.h"

class Queue
{
private:
    int item_size;
    Stack stackIn;
    Stack stackOut;

    public :
    /* Assigns the size of all the elements withing the queue, returns size 
    when succes, returns -1 otherwise, or when size is to smaller then 0.*/
    int assignSize(int item_size);

    /* Enqueue an element from *obj, return 0 when success, otherwise -1 */
    int enqueue(void *data);

    /* Dequeue an element to *obj, return 0 when success, otherwise -1 */ 
    int dequeue(void * obj);

    /* Delete queue and memory allocated for it. Returns 0 when succes, else -1. */
    int destroy();

    Queue()
    {
    }
};
#endif
