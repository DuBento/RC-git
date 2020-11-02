#include "list.h"


// the structure that stores each node of the list
struct list_node_t 
{
  void *_data;          // the data stored on the node
  ListNode_t _prev;     // the previous node of the list
  ListNode_t _next;     // the next node of the list
};


// the structure that is going to be used to manipulate the list
struct list_t
{
  ListNode_t _head;     // the fist element of the list
  int _size;            // the number of element of the list
};



// creates a new list instance
List_t listCreate()
{
  List_t list = (List_t) malloc(sizeof(struct list_t));
  if (!list)
    FATAL("Unable to allocate memory for the list");
    
  list->_size = 0;
  list->_head = NULL;
  return list;
}


// destroys a list instance
void listDestroy(List_t list, void (*free)(void*))
{
  ListNode_t temp;
  while((temp = list->_head) != NULL)
    listRemove(list, temp, free);
  free(list);
}


// inserts a new element at the beggining of the list
ListNode_t listInsert(List_t list, void *data)
{
  ListNode_t node = (ListNode_t) malloc (sizeof(struct list_node_t));
  node->_data = data;
  node->_prev = NULL;
  node->_next = list->_head;
  list->_head = node;
  list->_size++;
  return node;
}


// removes the given element from the list
void listRemove(List_t list, ListNode_t node, void (*clean)(void*))
{
  if (node->_prev == NULL)
  {
    list->_head = node->_next;
    if (node->_next != NULL)
      (node->_next)->_prev = NULL;
  }
  else
  {
    (node->_prev)->_next = node->_next;
    if (node->_next != NULL)
      (node->_next)->_prev = node->_prev;
  } 

  clean(node->_data);
  free(node);
  list->_size--;
}


// returns the data stored on the specified node
void* listValue(ListNode_t node)
{
  return node->_data;
}


/* Returns the size of the list. */
int listSize(List_t list)
{
  return list->_size;
}




// the structure that represents an iterator of the list.
struct list_iterator_t
{
  ListNode_t current;     // the current element of the iteration
};



// creates an iterator for the specified list
ListIterator_t listIteratorCreate(List_t list)
{
  ListIterator_t iterator = (ListIterator_t) 
    malloc(sizeof(struct list_iterator_t));
  if (!list)
    FATAL("Unable to allocate memory for the list iterator");
  
  iterator->current = list->_head;
  return iterator;
}


// returns the emptyness flag of the iterator
int listIteratorEmpty(ListIterator_t *iterator)
{
  if ((*iterator)->current == NULL)
  {
    free(*iterator);
    (*iterator) = NULL;
    return 1;
  }
  else
    return 0; 
}


// jumps on to the next element of the list and returns its value
void* listIteratorNext(ListIterator_t *iterator)
{
  ListNode_t node = (*iterator)->current;
  (*iterator)->current = node->_next;
  return node->_data;  
}