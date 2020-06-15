#include "WordTree.h"




WordTree::WordTree(const WordTree& rhs) {
    
    this->root = nullptr;
    
    if (rhs.root == nullptr)
        return;
    else {
        int count = 0;
        IType name = ""; 
        WordNode* cur = rhs.root;
        this->copyAux(count, name, cur, *this);
    }
}


const WordTree& WordTree::operator=(const WordTree& rhs) {
    
    WordNode* thisPtr = this->root;
    
    // Delete whatever is currently in the tree
    destructorAux(thisPtr);
    
    // Set this root to nullptr, so there is no accident referencing
    this->root = nullptr;
    
    // Proceed as if the tree was empty
    int count = 0;
    IType name = "";
    WordNode* cur = rhs.root;
    this->copyAux(count, name, cur, *this);
    
    // Return the entire tree (*this) and not thisPtr
    return *this;
};

WordTree::~WordTree() {
    
    WordNode* cur = root;
    destructorAux(cur);
    root = nullptr;
}

void WordTree::destructorAux(WordNode* cur) {
    
    if (cur == nullptr)
        return;
    
    destructorAux(cur->m_left);
    destructorAux(cur->m_right);
    // Delete the current node AFTER checking for and
    // deleting possible children
    delete cur;
}


// Inserts v into the WordTree
void WordTree::add(IType v, int count) {
    
    // If the tree is empty, point the rootNode to a new pointer
    if (!root) {
        root = new WordNode(v, count);
        return;
    }

    // If the root contains the same value, increase count
    if (root->m_data == v) {
        root->m_count++;
        return;
    }
    
    else {
        WordNode* cur = root;
        
        // Indefinite Loop
        for(;;) {
            
            if (v == cur->m_data) {
                cur->m_count++;
                return;
            }
            
            else if (v < cur->m_data) {
                
                if (cur->m_left == nullptr) {
                    cur->m_left = new WordNode(v, count);
                    return;
                }
                else // There is a left child node
                    cur = cur->m_left;
            }
                
            else if (v > cur->m_data) {
                
                if (cur->m_right == nullptr) {
                    cur->m_right = new WordNode(v, count);
                    return;
                }
                else // There is a left child node
                    cur = cur->m_right;
            }
        }
    }
}





int WordTree::distinctWords() const {
    
    int wordCount = 0;
    WordNode* cur = root;
    distinctWordsAux(cur, wordCount); // Recursive helper function;
    return wordCount;
}

void WordTree::distinctWordsAux(WordNode* cur, int& count) const
{
    if (cur == NULL)        // If empty, return withoud
        return;
    
    distinctWordsAux(cur->m_left, count);     // Process nodes in left sub-tree.
    count += (cur->m_count / cur->m_count);   // Add one for the current
    distinctWordsAux(cur->m_right, count);    // Process nodes in right sub-tree.
}


int WordTree::totalWords() const {
    
    int totalCount = 0;
    WordNode* cur = root;
    totalWordsAux(cur, totalCount);
    return totalCount;
}

void WordTree::totalWordsAux(WordNode* cur, int& totalCount)  const
{
    if (cur == NULL)        // If empty, return withoud
        return;
    
    totalWordsAux(cur->m_left, totalCount);     // Process nodes in left sub-tree.
    totalCount += cur->m_count;                 // Add the count
    totalWordsAux(cur->m_right, totalCount);    // Process nodes in right sub-tree.
}

void WordTree::printInOrder(ostream &out, WordNode* cur) const {
    
    if (cur == nullptr)
        return;
    
    printInOrder(out, cur->m_left);
    
    out << cur->m_data << " " << cur->m_count << endl;
    
    printInOrder(out, cur->m_right);
    
}

void WordTree::copyAux(int &count, string &IType, WordNode* cur, WordTree &src) const {
    
    if (cur == nullptr)
        return;
    
    copyAux(count, IType, cur->m_left, src);
    
    src.add(cur->m_data, cur->m_count);
    
    copyAux(count, IType, cur->m_right, src);
    
    
    
}

// - - - - - - - - - -  Friend Function(s) Implementation - - - - - - - - - - //
ostream& operator<<(ostream &out, const WordTree& rhs) {
    
    WordNode* cur = rhs.root;
    rhs.printInOrder(out, cur);
    return out; 
    
}




