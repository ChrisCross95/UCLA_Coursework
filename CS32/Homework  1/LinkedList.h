
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
#include <string>
using namespace std;

typedef string ItemType;

struct Node {
    ItemType value;
    Node *next;
};

class LinkedList {
public:
    // default constructor
    LinkedList() : head(nullptr) { }
    
    // copy constructor
    LinkedList(const LinkedList& rhs);
    
    // Destroys all the dynamically allocated memory
    // in the list.
    ~LinkedList();
    
    // assignment operator
    const LinkedList& operator=(const LinkedList& rhs);
    
    // Inserts val at the front of the list
    void insertToFront(const ItemType &val);
    
    // Inserts val at the read of the list
    void insertToRear(const ItemType &val);
    
    // Inserts val according to ordering
    void addItem(ItemType val);
    
    //
    void deleteItem(ItemType &val);
    
    // Prints the LinkedList
    void printList() const;
    
    // Sets item to the value at position i in this
    // LinkedList and return true, returns false if
    // there is no element i
    bool get(int i, ItemType& item) const;
    
    void set(int i, ItemType& item) const; 
    
    // Reverses the LinkedList
    void reverseList();
    
    // Prints the LinkedList in reverse order
    void printReverse() const;
    
    // Appends the values of other onto the end of this
    // LinkedList.
    void append(const LinkedList &other);
    
    // Exchange the contents of this LinkedList with the other
    // one.
    void swap(LinkedList &other);
    
    // Returns the number of items in the Linked List.
    int size() const;
    
    // Returns true if the list is empty; false otherwise. 
    bool isEmpty() const;
    
    
private:
    Node* head;
};

#endif
