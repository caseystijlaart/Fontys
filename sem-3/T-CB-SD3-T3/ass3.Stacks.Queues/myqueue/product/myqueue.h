#ifndef __MYQUE__
#define __MYQUE__
typedef struct queueMeta {
	StackMeta_t *stack_in;
	StackMeta_t *stack_out;
	int item_size;
}QueueMeta_t;

/* Cretae queue and return a handle to it, if error return NULL */
extern QueueMeta_t *myqueue_create(int item_size);
/* Delete queue and memory allocated for it */
extern void myqueue_delete(QueueMeta_t *queue);
/* Enqueue an element from *obj, return 0 when success, otherwise -1 */
extern int myqueue_enqueue(QueueMeta_t *queue, void *obj);
/* Dequeue an element to *obj, return 0 when success, otherwise -1otherwise -1 
 * Caller of this API is responsible for allocating memory for obj */
extern int myqueue_dequeue(QueueMeta_t *queue, void *obj);
#endif
