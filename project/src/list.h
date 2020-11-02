#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>

#include "common.h"


/* The structure that is going to be used to manipulate the list. */
typedef struct list_t *List_t;

/* The structure correspondent to each node of the list. */
typedef struct list_node_t *ListNode_t;



/**
 * Creates a new list instance.
 * 
 * @return The new list instance.
 */
List_t listCreate();


/**
 * Destroys a list instance.
 * 
 * @param list
 *          The list that is going to be destroyed.
 * @param free
 *          The function to free the memory associated with the stored data.
 */
void listDestroy(List_t list, void (*free)(void*));


/**
 * Inserts a new element at the beggining of the list.
 * 
 * @param list
 *          The list instance.
 * @param data
 *          The data that will be stored on the list.
 * @return The pointer to the new list node
 */
ListNode_t listInsert(List_t list, void *data);


/**
 * Removes the given element from the list.
 * 
 * @param list
 *          The list instance.
 * @param node
 *          The element that is going to be removed.
 * @param clean
 *          The function to free the memory associated with the stored data.
 * @return The list instance
 */
void listRemove(List_t list, ListNode_t node, void (*clean)(void*));


/**
 * Returns the data stored on the specified node.
 * 
 * @param node
 *          The node from the list.
 * @return The data stored on the node.
 */
void* listValue(ListNode_t node);


/**
 * Returns the size of the list.
 * 
 * @param list
 *          The list instance.
 * @return The size of the list
 */
int listSize(List_t list);





/* The structure that allows to iterate through a list. */
typedef struct list_iterator_t *ListIterator_t;



/**
 * Creates an iterator for the specified list.
 * 
 * @param list
 *          The list that is going to be iterated.
 * @return The iterator of the list.
 */
ListIterator_t listIteratorCreate(List_t list);


/**
 * Returns the emptyness flag of the iterator.
 * The iterator is automatically destroyed if there aren't elements to iterate
 * 
 * @param iterator
 *          The iterator of the list.
 * @return The emptyness flag of the iterator
 */
int listIteratorEmpty(ListIterator_t *iterator);


/**
 * Returns the current element of the iteration and jumps on to the next
 * 
 * @param iterator
 *          The iterator of the list.
 * @return The current element of the iteration.
 */
void* listIteratorNext(ListIterator_t *iterator);

#endif