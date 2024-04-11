#ifndef LINKEDLISTH
#define LINKEDLISTH
/* SD exercise 1: simple linked list API */
/* Author : R. Frenken                   */
/* Version : 1.0			 */

typedef struct ITEM {
	int value;
	struct ITEM *next;
}ITEM;

/***************************************************/
/* If *list == NULL, a new list should be created, */
/* otherwise prepend a new ITEM with value value   */
/* to the existing list                            */
/* Returns -1 if not enough memory, otherwise 0    */
/***************************************************/
extern int add_first(ITEM **list, int value);

/***************************************************/
/* If *list == NULL, a new list should be created, */
/* otherwise append a new ITEM with value value to */
/* the existing list                               */
/* Returns -1 if not enough memory, otherwise 0    */
/***************************************************/
extern int add_last(ITEM **list, int value);

/****************************************************/
/* Insert a new item after current item c_item      */
/* Returns -1 if not enough memory, c_item == NULL, */
/* *list == NULL or c_item not found, otherwise 0   */
/****************************************************/
extern int add_after(ITEM *list, ITEM *c_item, int value);

/***************************************************/
/* Remove first item of the list                   */
/* Returns -1 if list==NULL, otherwise 0           */
/***************************************************/
extern int rem_first(ITEM **list);

/***************************************************/
/* Remove last item of the list                    */
/* Returns -1 if list==NULL, otherwise 0           */
/***************************************************/
extern int rem_last(ITEM **list);

/***************************************************/
/* Remove item after c_item of the list            */
/* Returns -1 list==NULL, c_item not found or      */
/* c_item is the last_element                      */
/***************************************************/
extern int rem_after(ITEM *list, ITEM *c_item);


/*********************************************************/
/* All dynamic memory allocated to the list is freed     */
/*********************************************************/

extern void clean(ITEM *list);
#endif
