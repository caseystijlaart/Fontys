#ifndef __MEM_MANAGER_H__
#define __MEM_MANAGER_H__

typedef struct element
{
    int                 addr;
    int                 size;
    struct element *    next;
} ELEMENT;

void showList (ELEMENT * list);
int add_node(ELEMENT **list, int size, int addr);
int freeMemory (ELEMENT ** freeList, ELEMENT ** allocList, int addr);
int claimMemory (ELEMENT ** freeList, ELEMENT ** allocList, int nrofBytes);
void cleanList (ELEMENT ** list);

#endif
