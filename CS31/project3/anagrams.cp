#include <iostream>
#include <fstream>
#include <istream>
#include <cstring>
using namespace std;


const int MAXRESULTS   = 20;    // Max matches that can be found
const int MAXDICTWORDS = 30000; // Max words that can be read in

// Forward Funtion Declarations

void loadLoop(istream &dictfile, string dict[], int &dictSize);

int loadWords(istream &dictfile, string dict[]);

bool checkAgainstDict(const string candidateMatch, const string dict[], const int dictSize, int counter); 

int checkAgainstResults(const string candidateMatch, string results[], int& matches, int counter, const string dict[], const int dictSize);

void permute(string word, int startingIndex, int endingIndex, string *results, int &matches, const string *dict, const int &dictSize);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
// Function Implementations

void swap(char* x, char* y) {
    
    char temp = *x;
    *x = *y;
    *y = temp;
};

void swapLoop(int index, int startingIndex, int endingIndex, int max, string word, string *results, int &matches, const string *dict, const int &dictSize ) {
    if (index > max)
        return;
    // Swap the ith character with the character at startingIndex
    swap(word[startingIndex], word[index]);
    // Recursively generate strings with this new ordering
    permute(word, startingIndex + 1, endingIndex, results, matches, dict, dictSize);
    // Undo the previous sway so we can backtrap, i.e. move 'up' the recursive tree
    swap(word[startingIndex], word[index]);
    // Inrrement the index and recursively call swapLoop so we can do it all over again with the next character
    swapLoop(index + 1, startingIndex, endingIndex, max, word, results, matches, dict, dictSize);
}

void permute(string word, int startingIndex, int endingIndex, string *results, int &matches, const string *dict, const int &dictSize)
{
    // Base case: we've reached the end of the word and generated a unique permuutation
    if (startingIndex == endingIndex) {
        cerr << word << endl;
        int counter = 0;
        checkAgainstResults(word, results, matches, counter, dict, dictSize);
    }
    // Generate more permutations
    else
        swapLoop(0, startingIndex, endingIndex, endingIndex, word, results, matches, dict, dictSize);
}

// Places all the combinations of word, which are found in dict into results.
// Returns the number of matched words found. This number should not be larger than
// MAXRESULTS since that is the size of the array. The size is the number of words
// inside the dict array.
int recBlends(string word, const string dict[], int size, string
              results[]) {
    
    int matches = 0;    // This will be incremeted every time a string is added to results
    
    int staringIndex = 0;
    int endingIndex = word.size() - 1;
    
    
    permute(word, staringIndex, endingIndex, results, matches, dict, size); // <---- Main action takes place inside this function 
    
    cerr << matches << endl;
    return matches;
    
};



// ============================== LOADING WORDS =================================== //
// Places each string in dictfile into the array dict. Returns the number of
// words read into dict. This number should not be larger than MAXDICTWORDS since
// that is the size of the array.
void loadLoop(istream &dictfile, string dict[], int &dictSize) {
    
    if (dictSize > MAXDICTWORDS)   // Are we past the maximum dictionary size?
        return;
    else if (dictfile.eof())        // Has the input stream run out of data? 
        return;
    else {                          // If no to both questions, read the next string into the array using buffer loadedWord
        string loadedWord;
        dictfile >> loadedWord;
        *dict = loadedWord;
        dict++;
        dictSize++;
        loadLoop(dictfile, dict, dictSize);
    }
    return;
}


int loadWords(istream &dictfile, string dict[]) {
    
    int dictSize = 0;
    
    loadLoop(dictfile, dict, dictSize);     // <---- Main action takes place inside this function
    
    return dictSize;
};

                 
bool checkAgainstDict(const string candidateMatch, const string dict[], const int dictSize, int counter) {
    
    // We've reached the end of the meaningful section of the dictionary without a match
    if (counter == dictSize) {
        return false;
    }
    // If we've found a match in the dictionary
    else if (candidateMatch == *dict)
        return true;
    else if (checkAgainstDict(candidateMatch, dict + 1, dictSize, counter + 1))
        return true;
    else
        return false;
}

int checkAgainstResults(const string candidateMatch, string results[], int& matches, int counter, const string dict[], const int dictSize) {
    
    // If results array is already full, we cannot
    if (matches == MAXRESULTS)
        return 0;
    // If we are past the last element in results array and we do not have a match, then
    // check candidateMatch against the dictionary array. If a match is found, add
    // candidate match to the end of the results array. In order to add a string to results,
    // it SHOULD NOT already be in results and SHOULD be in dictionary. 
    else if (counter == matches) {
        if (checkAgainstDict(candidateMatch, dict, dictSize, counter = 0)) {
            // Add candidateMatch to the end of the array
            *results = candidateMatch;
            matches++; 
            return 1;
        }
        else
            return 0;   // The candidate was in neither results or dictionary, so return false; 
    }
    // If the candidateMatch is already in the resuls array
    else if (candidateMatch == *results)
        return 0;
    else {
        // Check candidate against the next element in results, increment counter
        if (checkAgainstResults(candidateMatch, results + 1, matches, counter + 1, dict, dictSize))
            return 1;
        else
            return 0;
    }
    // Control should not reach this statement, return value of 2 indicates an error
    return 2;
}

// Displays size number of strings from results in reverse order.
void showResults(const string results[], int size) {
    
    // This base call is noly applicable if the no results were found
    if (size == 0)
        cout << "No matches found!" << endl;
    else {
        cout << "Matching word: " << results[size - 1] << endl;
        // This check ensures that we do not go past the beginning of
        // the array and also that we do not indicate no matches found
        if (size - 1 == 0)
            return;
        else
            showResults(results, size - 1);
    }
    return;
}

