include "SortedList.h"
#include <string.h>


/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *    The specified element will be inserted in to
 *    the specified list, which will be kept sorted
 *    in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void
SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    
    /** start a list head */
    struct SortedListElement *lookup;
    lookup = list;

    
    /** The list head is also recognizable by its NULL key pointer. */
    while ( (lookup->next->key) != NULL ) {
        
        int cmp_val;
        cmp_val = strcmp(element->key, lookup->key);
        
        /** element key is equal to or less than node
         pointed to by lookup: update prev and next ptrs
         and return */
        if (cmp_val < 1) {
            /** critical section begin */
            element->prev      = lookup->prev;
            element->next      = lookup;
            lookup->prev->next = element;
            lookup->prev       = element;
            /** critical section end */
            return;
        } else {
            lookup = lookup->next;
        }
    }
    
    /** If we exit while-loop, we have traverse the whole list and the
     element key is the largest key, so insert at rear of list  OR
     the list is empty */
    /** critical section begin */
    element->prev      = lookup->prev;
    element->next      = lookup;
    lookup->prev->next = element;
    lookup->prev       = element;
    /** critical section end */
    return; 
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *    The specified element will be removed from whatever
 *    list it is currently in.
 *
 *    Before doing the deletion, we check to make sure that
 *    next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int
SortedList_delete( SortedListElement_t *element) {
    
    /** start a list head */
    struct SortedListElement *lookup;
    lookup = list;
    
    while ( (lookup->key != element->key) && ( (lookup->next->key) != NULL) ) {
        lookup = lookup->next;
    }
    
    /** corner cases:  list is empty OR element not in list */
    if ( (lookup->next-key) == NULL) {
        return 0;
    }
    
    if ( (lookup->next->prev) != element ) { return 1; }
    if ( (lookup->prev->next) != element ) { return 1; }
    
    /** critical section begin */
    lookup->next->prev = lookup->prev;
    lookup->prev->next = lookup->next;
    /** critical section end  */
    
    return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *    The specified list will be searched for an
 *    element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t*
SortedList_lookup(SortedList_t *list, const char *key) {
    
    /** start a list head */
    struct SortedListElement *lookup;
    lookup = list;
    
    /** critical section begin */
    while ( (lookup->next->key) != NULL) {
        /** return lookup if lookup key matches search key */
        if ( strcmp(key, (lookup->key)) == 0 )
        {
            return lookup;
        } else {
            lookup = lookup->next;
        }
    }
    /** critical section end */
    /** return NULL if no matching elemetn is found */
    return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *    While enumerating list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *       -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list) {
    
    /** start a list head */
    struct SortedListElement *lookup;
    int count = 0;
    lookup = list;
 
    while ( (lookup->next->key) != NULL ) {
        if ( (lookup->next->prev) != element ) { return -1; }
        if ( (lookup->prev->next) != element ) { return -1; }
        lookup = lookup->next; /** critical section */
        count++
    }
    
    return count;
    
}
