#ifndef WORDTREE_H
#define WORDTREE_H

#include <iostream>
#include <string>
using namespace std;

typedef string IType;

struct WordNode {
    
    WordNode(IType data)
    : m_data(data), m_left(nullptr), m_right(nullptr), m_count(1) {}
    
    WordNode(IType data, int count)
    : m_data(data), m_left(nullptr), m_right(nullptr), m_count(count) {}
    
    ~WordNode() { cerr << "Poof" << endl; }
    
    IType m_data;
    int m_count; 
    WordNode *m_left;
    WordNode *m_right;
};

class WordTree
{
public:
    // default constructor
    WordTree() : root(nullptr) { };
    
    // copy constructor
    WordTree(const WordTree& rhs);
    
    // Destroys all the dynamically allocated memory in the tree
    ~WordTree();
    
    // assignment operator
    const WordTree& operator=(const WordTree& rhs);
    
    // Inserts v into the WordTree
    void add(IType v, int count = 1);
    
    // Returns the number of distinct words / nodes
    int distinctWords() const;
    
    // Returns the total number of words inserted, including
    // duplicate values
    int totalWords() const;
    
    // Prints the LinkedList
    friend ostream& operator<<(ostream &out, const WordTree& rhs);
    
private:
    WordNode* root;
    
    // Private Auxiilary Member Functions
    void distinctWordsAux(WordNode* cur, int& count) const;
    void totalWordsAux(WordNode* cur, int& totalCount) const;
    void destructorAux(WordNode* cur); 
    void printInOrder(ostream &out, WordNode* cur) const;
    void copyAux(int &count, string &IType, WordNode* cur, WordTree &src) const; 
};

#endif
