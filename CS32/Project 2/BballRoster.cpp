//
//  BballRoster.cpp
//  Project 2 (CS 31)
//
//  Created by Christopher Cross on 7/11/19.
//  Copyright © 2019 Christopher Cross. All rights reserved.
//

#include "BballRoster.h"

BballRoster::BballRoster(const BballRoster& src) {
    
    this->head = nullptr;
    this->tail = nullptr;
    Node* srcPtr = src.head;
    // If the copied Roster is empty
    if (!srcPtr)
        return;
    else
        while (srcPtr) {
            this->insertPlayerToRear(srcPtr->m_firstName, srcPtr->m_lastName, srcPtr->m_value);
            srcPtr = srcPtr->next;
        }
}


const BballRoster& BballRoster::operator=(const BballRoster& src) {
    
    Node* destPtr = this->head;
    Node* srcPtr = src.head;
    
    // If you are trying to assign a Roster to itself (Bad form, bro!)
    if (&(destPtr->next) == &(srcPtr->next))
        return *this;
    // If the roster to be assigned is empty
    if (!srcPtr) {
        // If the roster receiving the assignment is also empty
        if (!destPtr) {
            this->head = nullptr;
            this->tail = nullptr;
            return *this;
        }
        Node* traversePtr = head;
        while (traversePtr) {
            Node* placePtr = traversePtr->next;
            delete traversePtr;
            traversePtr = placePtr;
        }
        this->head = nullptr;
        this->tail = nullptr; 
        return *this;
    }

    // Delete the destination LinkedList if it is non-empty
    if (destPtr) {
        Node* traversePtr = head;
        while (traversePtr) {
            Node* placePtr = traversePtr->next;
            delete traversePtr;
            traversePtr = placePtr;
        }
    }
    this->tail = nullptr; 
    this->head = nullptr;
    srcPtr = src.head;
    // Copy the source LinkedList into the destination LinkedList node by node
    while (srcPtr)
    {
        this->insertPlayerToRear(srcPtr->m_firstName, srcPtr->m_lastName, srcPtr->m_value);
        srcPtr = srcPtr->next;
    }
    return *this;
}


BballRoster::~BballRoster() {
    Node* traversePtr = head;
    while (traversePtr) {
        Node* placePtr = traversePtr->next;
        delete traversePtr;
        traversePtr = placePtr;
    }
}


bool BballRoster::rosterEmpty() const {
    
    if (this->head == nullptr)
        return true;
    else
        return false;
}

int BballRoster::howManyPlayers() const {
    Node* traversePtr = head;
    int i = 0;
    while (traversePtr) {
        traversePtr = traversePtr->next;
        i++;
    }
    return i;
}

bool BballRoster::signPlayer(const std::string& firstName, const std::string&
                             lastName, const SomeType& value)  {
    
    Node* traversePtr = head;
    Node* nextToLast = nullptr; // For a corner-case involving matching lastnames and different firstnames
    // If the Roster is empty
    if (this->rosterEmpty()) {
        this->insertPlayerToFront(firstName, lastName, value);
        return 1;
    }
    
    // If a player with the same first and last name is already on the roster, do nothing and return.
    while (traversePtr) {
        if ((traversePtr->m_firstName == firstName) && (traversePtr->m_lastName == lastName))
            return 0;
        traversePtr = traversePtr->next;
    }
    
    // If the player should go at the front of the list
    if (lastName < head->m_lastName || (lastName == head->m_lastName && firstName < head->m_firstName)) {
        this->insertPlayerToFront(firstName, lastName, value);
        return 1;
    }
    // If the player should go at the end of the list
    if (lastName > tail->m_lastName || (lastName == tail->m_lastName && firstName > tail->m_firstName)) {
        this->insertPlayerToRear(firstName, lastName, value);
        return 1;
    }
    
    // Reset traversePtr
    traversePtr = head;
        
            while (traversePtr) {
                
                // If there is a player with the same lastName already on the roster,
                // sort by first name
                if (traversePtr->m_lastName == lastName) {
                    while (traversePtr && traversePtr->m_lastName ==  lastName && traversePtr->m_firstName < firstName) {
                        // If traversePtr points to the last pointer, save this value as nextToLast, so
                        // that when we add another node at the end of the list, we can access nextToLast's values
                        if (!traversePtr->next)
                            nextToLast = traversePtr;
                        traversePtr = traversePtr->next;
                    }
                    Node* newNode = new Node;
                    newNode->m_firstName = firstName;
                    newNode->m_lastName = lastName;
                    newNode->m_value = value;
                    // If we've reached the end of the list
                    // perform the insertion using nexToLast's values
                    if (!traversePtr) {
                        newNode->next = nullptr;
                        newNode->previous = nextToLast;
                        nextToLast->next = newNode;
                        tail = newNode;
                        return 1; 
                    }
                    if (!traversePtr->next) {
                        newNode->next = traversePtr;
                        newNode->previous = traversePtr->previous;
                        traversePtr->previous = newNode;
                        newNode->previous->next = newNode;
                        return 1; 
                    }
                    newNode->next = traversePtr;  // Update the new nodes's next-pointer with the traversePtr's next-pointer
                    newNode->previous = traversePtr->previous;    // Update the new node's previous-pointer with the traversePtr
                    traversePtr->previous = newNode;        // Update the previous node's next-pointer with the new node
                    newNode->previous->next = newNode;  // Update the next node's previous-pointer with the new node
                        
                    return 1;
                }
                
                // Assuming there is no player with the same lastName already on the roster,
                // sort by last name
                if (traversePtr->m_lastName < lastName &&
                    traversePtr->next->m_lastName > lastName) {
                    Node* newNode = new Node;
                    newNode->m_firstName = firstName;
                    newNode->m_lastName = lastName;
                    newNode->m_value = value;
                    newNode->next = traversePtr->next;
                    newNode->previous = traversePtr;
                    traversePtr->next = newNode;
                    newNode->next->previous = newNode;
                    return 1;
                }
                
                traversePtr = traversePtr->next;
                
            }
    return 0;
}

bool BballRoster::resignPlayer(const std::string& firstName, const std::string&
                  lastName, const SomeType& value) {
    
    Node* traversePtr = head;
    while (traversePtr->next)
    {
        if (traversePtr->m_firstName == firstName && traversePtr->m_lastName == lastName) {
            traversePtr->m_value = value;
            return true;
        }
        traversePtr = traversePtr->next;
    }
    return false;
}


bool BballRoster::signOrResign(const std::string& firstName, const std::string&
                               lastName, const SomeType& value) {
    
    if (resignPlayer(firstName, lastName, value))
        return true;
    if (!resignPlayer(firstName, lastName, value))
        signPlayer(firstName, lastName, value);
    return true;
}


bool BballRoster::renouncePlayer(const std::string& firstName, const
                    std::string& lastName) {
    Node* traversePtr = head;
    
    // Cannot renounce a player who is not on the roster 
    if (!this->playerOnRoster(firstName,lastName))
        return false;
    
    // If the renounced player is first on the roster
    if (head->m_firstName == firstName && head->m_lastName == lastName) {
        traversePtr = head;
        head = traversePtr->next;
        // If there is more than one player on the roster, i.e. if head does not point to the nullptr
        if (head)
            head->previous = nullptr;
        delete traversePtr;
        return true;
    }
    
    // If the renounced player is last on the roster
    if (tail->m_firstName == firstName && tail->m_lastName == lastName) {
        traversePtr = tail;
        tail = tail->previous;
        delete traversePtr;
        // If there is more than one player on the roster, i.e. if tail does not point to the nullptr
        if (tail)
            traversePtr = tail;
            traversePtr->next = nullptr;
        return true; 
    }
    
    
    traversePtr = head;
    
    // If the reounced player is somwhere in the middle of the roster
    while (traversePtr->next) {
        // If we at the node we want to delete
        if (traversePtr->m_firstName == firstName && traversePtr->m_lastName == lastName) {
            traversePtr->next->previous = traversePtr->previous;
            traversePtr->previous->next = traversePtr->next;
            delete traversePtr;
            return true;
        }
        traversePtr = traversePtr->next;
    }
    
    return false; 

}


bool BballRoster::playerOnRoster(const std::string& firstName, const
                    std::string& lastName) const {
    
    // No player can be on an empty roster
    if (this->rosterEmpty())
        return false;
    
    Node* traversPtr = head;
    while (traversPtr)
    {
        if (traversPtr->m_firstName == firstName && traversPtr->m_lastName == lastName)
            return true;
        traversPtr = traversPtr->next;
    }
    return false;
};

bool BballRoster::lookupPlayer(const std::string& firstName, const std::string&
                    lastName, SomeType& value) const {
    
    Node* traversPtr = head;
    while (traversPtr)
    {
        if (traversPtr->m_firstName == firstName && traversPtr->m_lastName == lastName) {
            value = traversPtr->m_value;
            return true;
        }
        traversPtr = traversPtr->next;
    }
    return false;
};

bool BballRoster::choosePlayer(int i, std::string& firstName, std::string&
                  lastName, SomeType& value) const {
    
    // If the Roster is empty
    if (this->rosterEmpty())
        return false;
    // If the index is 'out-of-range'
    if ( i < 0 || i >= this->howManyPlayers())
        return false;
    Node* traversePtr = head;
    // Decrement i until we reach the index of the node we want
    while (i) {
        traversePtr = traversePtr->next;
        i--;
    }
    // Copy the information into the function reference-arguments
    firstName = traversePtr->m_firstName;
    lastName = traversePtr->m_lastName;
    value = traversePtr->m_value;
    return 1; 
};

void BballRoster::swapRoster(BballRoster& other) {
    
    BballRoster tempRoster(other);
    other = *this;
    *this = tempRoster;
};

//=============== Private Auxillary Function Implementations ====================//

void BballRoster::insertPlayerToFront(const std::string& firstName, const std::string&
                   lastName, const SomeType& value) {
    
    Node* newNode = new Node;
    newNode->m_firstName = firstName;
    newNode->m_lastName = lastName;
    newNode->m_value = value;
    newNode->next = head; // will still point to nullptr if the list is empty because head points to mullptr in empty list
    newNode->previous = nullptr; // first node's previous-pointer points to nullptr
    // if the list is non-empty the first node is now the second node, and its previous-pointer
    // must point to the newly-created front node
    if (head != nullptr)
        head->previous = newNode;
    head = newNode;
    if (this->howManyPlayers() == 1)
        tail = head;
}

void BballRoster::insertPlayerToRear(const std::string& firstName, const std::string&
                  lastName, const SomeType& value) {
    
    // If we are adding to an empty LinkedList
    if (this->rosterEmpty())
        this->insertPlayerToFront(firstName, lastName, value);
    // Else if the LinkedList is not empty
    else {
        Node* newNode = new Node;
        newNode->m_firstName = firstName;
        newNode->m_lastName = lastName;
        newNode->m_value = value;
        tail->next = newNode;
        newNode->next = nullptr;   // This is the last node, so it must point to the nullptr
        newNode->previous = tail; 
        tail = newNode;             //  This is the last node, so the tail must point to it
    }
}


//=============== Non-member Function Implementations ====================//



bool joinRosters(const BballRoster & bbOne, const BballRoster & bbTwo, BballRoster & bbJoined) {
    
    // Create a temporary Roster
    BballRoster tempRoseter;
    
    // Checking for various cases of aliasing:
    // If the first two arguments alias the same Roster
    if (&bbOne == &bbTwo) {
        bbJoined = bbOne;
        return true;
    }
    
    // Assign bbJoined an empty roster if both arguments are empty rosters 
    if (bbOne.rosterEmpty() && bbTwo.rosterEmpty()) {
        bbJoined = tempRoseter;
        return true;
    }
    // If either of the arguments is empty, assign bbJoined the other roster 
    if (bbOne.rosterEmpty()) {
        bbJoined = bbOne;
        return true;
    }
    if (bbTwo.rosterEmpty()) {
        bbJoined = bbOne;
        return true;
    }
    
    bool sameNameDiffValFlag = false;
    // Iterate through the first roster, get info for each player;
    // if a player with the same First and Last name is on the second roster
    // with a different value, set sameNameDiffValFlag, otherwise add player
    // to bbJoined. If a player with the same first and last name is NOT on
    // the second roster, add player to bbJoined
    for (int i = 0; i < bbOne.howManyPlayers(); i++) {
        std::string first;
        std::string last;
        SomeType valOne;
        SomeType valTwo;
        bbOne.choosePlayer(i, first, last, valOne);
        if (bbTwo.lookupPlayer(first, last, valTwo)) {
            if (valOne != valTwo)
                sameNameDiffValFlag = true;
            else
                tempRoseter.signPlayer(first, last, valOne);
        } else {
            tempRoseter.signPlayer(first, last, valOne);
        }
    }
    // We have already checked for matching names, so now we need only
    // add the names from the second roster that are not on the first roster
    for (int i = 0; i < bbTwo.howManyPlayers(); i++) {
        std::string first;
        std::string last;
        SomeType valOne;
        bbTwo.choosePlayer(i, first, last, valOne);
        if (!bbOne.playerOnRoster(first, last))
            tempRoseter.signPlayer(first, last, valOne);
    }
    // Assign the newly-create temporary Roster to bbJoined Roster
    bbJoined = tempRoseter;
    // If there exists a full name that appears in both bbOne and bbTwo, but with different
    // corresponding values, then this function returns false; if there is no full name like this,
    // the function returns true, i.e.:
    return !sameNameDiffValFlag;
    
};

void checkRoster (const std::string& fsearch, const std::string& lsearch, const BballRoster& bbOne,
                  BballRoster& bbResult) {
    
    // Creete a temporary Roster
    BballRoster tempRoseter;
    // If you wildcard both the first and last name,
    // assign bbOne to bbResult and exit
    if (fsearch == "*" && lsearch == "*") {
        bbResult = bbOne;
        return;
    }
    
    // for each player on the roster, get his information and
    // compare with fsearch and/or lsearch depending on wildcarding
    for (int i = 0; i < bbOne.howManyPlayers(); i++)
    {
        std::string first;
        std::string last;
        SomeType val;
        bbOne.choosePlayer(i, first, last, val);
        if (fsearch == "*") {
            if (lsearch == last)
                tempRoseter.signPlayer(first, last, val);
        } else if (lsearch == "*") {
            if (fsearch == first)
                tempRoseter.signPlayer(first, last, val);
        } else {
            if (lsearch == last && fsearch == first)
                tempRoseter.signPlayer(first, last, val);
        }
    }
    bbResult = tempRoseter;
};
