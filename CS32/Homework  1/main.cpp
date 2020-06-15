

#include <iostream>
#include "LinkedList.h"
using namespace std;






int main() {

  /*  LinkedList e1;
    e1.insertToFront("bell");
    e1.insertToFront("biv");
    e1.insertToFront("devoe");
    LinkedList e2;
    e2.insertToFront("Andre");
    e2.insertToFront("Big Boi");
    cout << e1.size() << endl;
    cout << e2.size() << endl; 
    e1.append(e2); // adds contents of e2 to the end of e1
    string s;
    cout << e1.size() << endl;
    cout << e2.size();
    e1.printList(); 
    assert(e1.size() == 5);
    cout << s; 
    assert(e1.get(3, s) && s == "Big Boi");
    
    assert(e2.size() == 2 && e2.get(1, s) && s == "Andre");
    
    LinkedList testlist;
    
    assert(!e1.get(5, s));
    string q;
    testlist.insertToFront("WOW");
    testlist.printList();
    assert(testlist.get(0, q) && q == "WOW");
    assert(!testlist.get(3, q));
    
    
    
    LinkedList e3;
    e3.insertToFront("Sam");
    e3.insertToFront("Carla");
    e3.insertToFront("Cliff");
    e3.insertToFront("Norm");
    e3.reverseList(); // reverses the contents of e1
    assert(e3.size() == 4 && e3.get(0, s) && s == "Sam");
    
    LinkedList e1;
    e1.insertToFront("A");
    e1.insertToFront("B");
    e1.insertToFront("C");
    e1.insertToFront("D");
    e1.printList();
    LinkedList e2(e1);
    e2.printList();
   
    e2.insertToFront("X");
    e2.insertToFront("Y");
    e2.insertToFront("Z");

    
    e1.swap(e2); // exchange contents of e1 and e2
    string s;
    assert(e1.size() == 3 && e1.get(0, s) && s == "Z");
    assert(e2.size() == 4 && e2.get(2, s) && s == "B");
    
    LinkedList e1;
    e1.insertToFront("A");
    e1.insertToFront("B");
    e1.insertToFront("C");
    e1.insertToFront("D");
    LinkedList e2;
    e2.insertToFront("X");
    e2.insertToFront("Y");
    e2.insertToFront("Z");
    e1.swap(e2); // exchange contents of e1 and e2
    string s;
    assert(e1.size() == 3 && e1.get(0, s) && s == "Z");
    assert(e2.size() == 4 && e2.get(2, s) && s == "B");
    e1.printList();
    e2.printList();
    
    LinkedList e1;
    e1.insertToFront("Sam");
    e1.insertToFront("Carla");
    e1.insertToFront("Cliff");
    e1.insertToFront("Norm");
    e1.printList(); 
    e1.reverseList(); // reverses the contents of e1
    e1.printList();
    string s;
    assert(e1.size() == 4 && e1.get(0, s) && s == "Sam"); */
    
    LinkedList e1;
    e1.insertToFront("A");
    e1.insertToFront("B");
    e1.insertToFront("C");
    e1.insertToFront("D");
    LinkedList e2;
    e2.insertToFront("X");
    e2.insertToFront("Y");
    e2.insertToFront("Z");
    e1.swap(e2); // exchange contents of e1 and e2
    string s;
    assert(e1.size() == 3 && e1.get(0, s) && s == "Z");
    assert(e2.size() == 4 && e2.get(2, s) && s == "B");
    cerr << "stay strong";

    
}
