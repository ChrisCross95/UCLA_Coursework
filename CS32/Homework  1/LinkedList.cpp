

#include "LinkedList.h"


LinkedList::LinkedList(const LinkedList& rhs) {
    
    this->head = nullptr;
    Node* srcPtr = rhs.head;
    // If the copied LinkedList is empty
    if (!srcPtr) {
        return;
    } else {
        while (srcPtr) {
            this->insertToRear(srcPtr->value);
            srcPtr = srcPtr->next;
        }
    }
};

// assignment operator
const LinkedList& LinkedList::operator=(const LinkedList& rhs) {
    
    Node* destPtr = this->head;
    Node* srcPtr = rhs.head;
    // If the LinkedList to be assigned is empty
    if (!srcPtr) {
        Node* traversePtr = head;
        while (traversePtr) {
            Node* placePtr = traversePtr->next;
            delete traversePtr;
            traversePtr = placePtr;
        }
        return *this;
    }
    // If you are trying to assign a LinkedList to itself (Bad form, bro!)
    if (&(destPtr->next) == &(srcPtr->next))
        return *this;
    // Delete the destination LinkedList if it is non-empty
    if (destPtr) {
        Node* traversePtr = head;
        while (traversePtr) {
            Node* placePtr = traversePtr->next;
            delete traversePtr;
            traversePtr = placePtr;
        }
    }
    this->head = nullptr;
    srcPtr = rhs.head;
    // Copy the source LinkedList into the destination LinkedList node by node
    while (srcPtr)
    {
        this->insertToRear(srcPtr->value);
        srcPtr = srcPtr->next;
    }
    return *this;
};


LinkedList::~LinkedList() {
    Node* traversePtr = head;
    while (traversePtr) {
        Node* placePtr = traversePtr->next;
        delete traversePtr;
        traversePtr = placePtr;
    }
}

// Inserts val at the front of the list. Create a new node, pass the value into the new node, set the new node's pointer to the address of the head; set the head to the address of the new node. Increment size by one. 
void LinkedList::insertToFront(const ItemType &val) {
    
    Node* newNode = new Node;
    newNode->value = val;
    newNode->next = head; // will still point to nullptr if the list is empty because head points to mullptr in empty list
    head = newNode;
};

void LinkedList::insertToRear(const ItemType &val) {
    
    //If we are adding to an empty LinkedList
    if (this->isEmpty())
        this->insertToFront(val);
    //Else if the LinkedList is not empty
    else {
        Node* traversePtr = head;
        //Traverse until we get to the last NODE of the LinkedList, not the last pointer
        while (traversePtr->next)
            traversePtr = traversePtr->next;
        Node* newNode = new Node;
        newNode->value = val;
        newNode->next = nullptr;
        traversePtr->next = newNode;
    }
}

void LinkedList::addItem(ItemType val) {
    
    // If the LinkedList is empty
    if (this->isEmpty())
        this->insertToFront(val);
    // If the item should go at the front of the list
    else if (val < head->value)
        this->insertToFront(val);
    else {
        Node* traversePtr = head;
        while (traversePtr->next) {
            if (traversePtr->value < val && traversePtr->next->value >= val)
                break;
            Node* newNode = new Node;
            newNode->value = val;
            newNode->next = traversePtr->next;
            traversePtr->next = newNode;
        }
    }
}

void LinkedList::deleteItem(ItemType &val) {
    
    // Cannot delete something from a LinkedList
    if (this->isEmpty())
        return;
    // If the item to be deleted is the first item 
    if (head->value == val) {
        Node* traversePtr = head;
        head = traversePtr->next;
        delete traversePtr;
        return;
    }
    
    Node* traversePtr = head;
    while (traversePtr) {
        // If we are above/before the node we want to delete
        if (traversePtr->next->value == val)
            break;
        traversePtr = traversePtr->next;
    }
    if (traversePtr) {
        Node* placePtr = traversePtr->next;
        traversePtr->next = placePtr->next;
    }
}

// Prints the LinkedList. Declare a pointer to traverse the list and initalize it to the head. While the traversing pointder does not equal the nullptr, print out the valaue of the node it points to and increment the pointer.
void LinkedList::printList() const {
    
    Node* traversePtr = head;
    while (traversePtr) {
        cout << traversePtr->value << " ";
        traversePtr = traversePtr->next;
    }
    cout << endl; 
};

// Sets item to the value at position i in this LinkedList and return true, returns false if there is no element i. Checks to see if i is greater than the size of the list.
bool LinkedList::get(int i, ItemType& item) const {
    
    if (this->isEmpty())
        return false; 
    Node* traversePtr = head;
    if (size() == 1 && i == 0) {
        item = traversePtr->value;
        return true;
    }
    // tricky code here: the local copy of int i is serving as a counter 
    while (i) {
        if (traversePtr->next == nullptr)
            return false;
        traversePtr = traversePtr->next;
        i--;
    }
    item = traversePtr->value;
    return true; 
};

void LinkedList::set(int i, ItemType& item) const {
    
    if (i >= size())
        return;
    Node* traversePtr = head;
    while (i) {
        traversePtr = traversePtr->next;
        i--;
    }
    traversePtr->value = item;
    return;
}

// Reverses the LinkedList. Declare two node pointers. Move one of the to the end of the linked list. For half the linked list's size, store the value of the front pointer's node as tempValue, set the front pointer's node-value to the end pointer's node-value, set the end pointer's node-value to the tempValue. Traverse the list by one using the front pointer; set the end pointer to the new position of the front pointer;
void LinkedList::reverseList() {
    
    int k = size()-1;
    Node* FrontPtr = head;
    while (k >= size()/2) {
        ItemType x;
        ItemType y = FrontPtr->value;
        get(k, x);
        set(k, y);
        FrontPtr->value = x;
        FrontPtr = FrontPtr->next;
        k--;
    }
};

// Prints the LinkedList in reverse order
void LinkedList::printReverse() const {
    
    int k = size() - 1;
    while (k >= 0) {
        ItemType x;
        get(k, x);
        cout << x << " ";
        k--;
    }
    cout << endl;
};

// Appends the values of other onto the end of this LinkedList.
void LinkedList::append(const LinkedList &other) {
    
    Node* endPtr = this->head;
    Node* otherPtr = other.head;
    
    while (endPtr->next)
        endPtr = endPtr->next;
    while (otherPtr) {
        endPtr->next = new Node;
        endPtr->next->value = otherPtr->value;
        endPtr->next->next = nullptr;
        endPtr = endPtr->next;
        otherPtr = otherPtr->next;
    }
};

// Exchange the contents of this LinkedList with the other
// one.
void LinkedList::swap(LinkedList &other) {
    
    LinkedList tempList(other);
    other = *this;
    *this = tempList;
};


bool LinkedList::isEmpty() const {
    if (head == nullptr)
        return true;
    else
        return false; 
}

int LinkedList::size() const {
    Node* traversePtr = head;
    int i = 0;
    while (traversePtr) {
        traversePtr = traversePtr->next;
        i++;
    }
    return i;
}
