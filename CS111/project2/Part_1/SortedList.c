#include "SortedList.h"
#include <string.h>
#include <sched.h>
#include <stdio.h>

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
    lookup = list->next;
    
    while ( (lookup != list) && (strcmp(element->key, lookup->key) < 1) ) {
        lookup = lookup->next;
    }
    /** Cases:
     (1)  element key is equal to or less than node pointed to by lookup:
     (2)  element key is largest key in list
     (3)  list is empty  */
    /** critical section begin */
    if (opt_yield & INSERT_YIELD) { sched_yield(); }
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
    
    /** The list head is also recognizable by its NULL key pointer. */
    if ((element->key) == NULL) {
        return 1;
    }
    
    // detect corrupted pointer
    if ( (element->next->prev) != element ) { return 1; }
    if ( (element->prev->next) != element ) { return 1; }
    
    /** critical section begin */
    element->next->prev = element->prev;
    if (opt_yield & DELETE_YIELD) { sched_yield(); }
    element->prev->next = element->next;
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
    lookup = list->next;
    
    while ( (lookup != list) && (strcmp(key, lookup->key) < 1) ) {
        /** critical section begin */
        if (opt_yield & LOOKUP_YIELD) { sched_yield(); }
        
        if ( strcmp(key, (lookup->key)) == 0 ) { return lookup; }
        
        lookup = lookup->next;
    }
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
int
SortedList_length(SortedList_t *list) {
    
    /** start a list head */
    struct SortedListElement *lookup;
    lookup = list->next;
    if ((lookup->prev) != list) {
        return -1;
    }
    
    int count = 0;
    
    while ( lookup != list ) {
        if ( (lookup->next->prev) != lookup ) { return -1; }
        lookup = lookup->next; /** critical section */
        count++;
        if (opt_yield & LOOKUP_YIELD) { sched_yield(); }
    }
    
    return count;
    
}
