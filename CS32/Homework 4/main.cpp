
#include <iostream>
#include "WordTree.h"
using namespace std;

int main() {
    
    // Combination of assertion tests and visual ouput tests
    WordTree k;
    k.add("Kim");
    k.add("Kanye");
    k.add("Kanye");
    k.add("Kanye");
    assert(k.distinctWords() == 2);
    assert(k.totalWords() == 4);
    k.add("McCown");
    k.add("Zyphr");
    k.add("Zyphr");
    cout << k;
    assert(k.totalWords() == 7);
    cerr << k.totalWords() << endl;
    WordTree m;
    m = k;
    cout << m.totalWords() << endl; 
    assert(m.totalWords() == 7); 
    cout << m;
    // Explicit call to destructor is only for testing purposes
    k.~WordTree();
    assert(k.totalWords() == 0);
    cerr << k.totalWords() << endl;
    m = k;
    assert(m.totalWords() == 0);
    m.add("Tim");
    m.add("Tim");
    assert(m.totalWords() == 2 && m.distinctWords() == 1);
    WordTree p = m;
    assert(p.totalWords() == 2 && p.distinctWords() == 1);
    // Explicit call to destructor is only for testing purposes
    p.~WordTree();
    WordTree n = p;
    assert(p.totalWords() == 0 && p.distinctWords() == 0);
    cerr << "Tests Passed!" << endl;
}
