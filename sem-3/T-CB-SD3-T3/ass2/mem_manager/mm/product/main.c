/* author : P.A.M. Maessen, J.H.J. Geurts
 * small changes like removal of resource detector and better API description by R. Frenken
 * date  : October 8 2018
 * version : 4
 */

#include "mem_manager.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define MEMORY_SIZE     100
#define MEMORY_ADDR     1000

static bool             with_menu = true;

 static int get_int (const char * message)
 {
     char 	line[80];
     int 	value;

     if (with_menu == true)
     {
         printf ("%s", message);
     }
     fgets (line, sizeof(line), stdin);
     sscanf (line, "%d", &value);

     return (value);
 }

int main (int argc, char * argv[])
{
    ELEMENT * 	allocList = NULL;
    ELEMENT * 	freeList = NULL;
    int 	nrofBytes;
    int 	addr;
    int 	choice;
    bool 	quit = false;

    printf ("\n"
        "PRC 'MemoryC' (version 4)\n"
        "-------------------------\n");

    if (argc != 1)
    {
        fprintf (stderr, "%s: argc=%d\n", argv[0], argc);
    }

    /* initialisate van lijsten dus freeList bevat alle geheugen en allocList is leeg */
    freeList = (ELEMENT *) malloc (sizeof(ELEMENT));
    freeList->size = MEMORY_SIZE;
    freeList->addr = MEMORY_ADDR;
    freeList->next = NULL;

    allocList = NULL;

    while (quit == false)
    {
        if (with_menu == true)
        {
            printf ("\n"
                "MENU\n"
                "===================\n"
                "[1] allocate memory\n"
                "[2] free memory\n"
                "[3] show lists\n"
                "[8] show/hide menu\n"
                "[9] quit\n\n");
        }
        choice = get_int ("choice: ");

        switch (choice)
        {
            case 1:
                nrofBytes = get_int ("Give nr of bytes: ");
                addr = claimMemory (&freeList, &allocList, nrofBytes);
                if (addr == -1)
                {
                    printf ("[ALLOC] not enough memory for %d byte\n", nrofBytes);
                }
                else
                {
                    printf ("[ALLOC] address: %4d (%d byte)\n", addr, nrofBytes);
                }
                break;
            case 2:
                addr = get_int ("Gve address to be freed: ");
                nrofBytes = freeMemory (&freeList, &allocList, addr);
                if (nrofBytes == -1)
                {
                    printf ("[FREE]  address: %4d was not allocated\n", addr);
                }
                else
                {
                    printf ("[FREE]  address: %4d (%d byte)\n", addr, nrofBytes);
                }
                break;
            case 3:
                printf ("FreeList:\n"
                    "---------\n");
                showList (freeList);
                printf ("AllocList:\n"
                    "----------\n");
                showList (allocList);
                break;
            case 8:
                if (with_menu == true)
                {
                    with_menu = false;
                    printf ("printing of MENU is diabled\n");
                }
                else
                {
                    // printing enabled
                    with_menu = true;
                }
                break;
            case 9:
                quit = true;
                break;
            default:
                printf ("invalid choice: %d\n", choice);
                break;
        }
    }

    /* opruimen van de lijsten */
    cleanList (&freeList);
    cleanList (&allocList);

    return (0);
}
