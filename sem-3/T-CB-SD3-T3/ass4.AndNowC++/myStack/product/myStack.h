/**
 * @file myStack.h
 * @author Casey Stijlaart
 * @brief Header file for the c++ stack.
 * @version 0.1
 * @date 2021-11-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

using namespace std;

#ifndef __MYSTACKH__
#define __MYSTACKH__

#include <iostream>

struct StackObject_t
{
    void * data;
    struct StackObject_t *next;
};

class Stack
{
private:
    size_t objsize;
    int numelem;

public:
    Stack()
    {
        numelem = 0;
        stack = NULL;
    }
    StackObject_t *stack;

    /* Assigns the size of all the elements withing the stack 
    returns objsize when succes, returns -1 otherwise, or when size 
    is to smaller then 0.*/
    int assignSize(size_t size);

    /* Pushes an object to the stack identified by its handle
    returns 0 on success and -1 on an error. */
    int push(void * data);

    /* Pops an object from the stack identified by its handle
    returns 0 on success and -1 on an error.*/
    int pop(void *obj);

    /* Destroys the stack. Returns 0 when succes, else -1. */
    int empty();

    /* Returns the number of the elements of the stack. */
    int numElem();
};
#endif
