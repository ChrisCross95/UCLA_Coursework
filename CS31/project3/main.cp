#include <iostream>
#include <fstream>
#include <istream>
#include <cstring>
using namespace std;

#include "anagrams.h"

const int MAXRESULTS   = 20;    // Max matches that can be found
const int MAXDICTWORDS = 30000; // Max words that can be read in


 int main()
{
    string results[MAXRESULTS];
    string dict[MAXDICTWORDS];
    ifstream dictfile;         // file containing the list of words
    int nwords;                // number of words read from dictionary
    string word;
    
    
    dictfile.open("/Users/majestikmind/Documents/Project 3/words.txt");
    if (!dictfile) {
        cout << "File not found!" << endl;
        return (1);
    }
    
    nwords = loadWords(dictfile, dict);
    cerr << nwords << endl;
    dictfile.close();
    
    cout << "Please enter a string for an anagram: ";
    cin >> word;
    
    int numMatches = recBlends(word, dict, nwords, results);
    if (!numMatches)
        cout << "No matches found" << endl;
    else
        showResults(results, numMatches);
}
