#ifndef __MYSTACK_H__
#define __MYSTACK_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* The stack is an abstract data type.
 * this means that the internal structures are
 * never exposed to the users of the library.
 * 
 * The users are only allowed to use the stack API
 * that is defined below. 
 */

typedef struct stackObject
{
	void* obj;
	struct stackObject *next;
} StackObject_t;

typedef struct stackMeta
{
	StackObject_t *stack;
	size_t objsize;
	int numelem;
} StackMeta_t;

/* Creates a stack of c objects of size objsize and returns
 * a handle (pointer) to the just created stack. On error, 
 * NULL is returned.
 */
extern StackMeta_t *mystack_create(size_t objsize);
/* Pushes an object to the stack identified by its handle
 * returns 0 on success and -1 on an error
 */
extern int mystack_push(StackMeta_t *stack, void* obj);
/* Pops an object from the stack identified by its handle
 * returns 0 on success and -1 on an error
 */
extern int mystack_pop(StackMeta_t *stack, void* obj);
/* Destroys and cleans the stack handle */
extern void mystack_destroy(StackMeta_t *stack);
/* Returns number of elements of the stack or -1 if invalid handle
 */
extern int mystack_nofelem(StackMeta_t *stack);

#endif /* __MSTACK_H__ */
