#include <iostream>
#include <cstring>
#include <string>
#include <cassert>
using namespace std;

typedef char* CharArrayPtr;
typedef int* IntArrayPtr;


long countWords(const char cstring[], long stringLength);

CharArrayPtr* createWordArray(char cstring[], long stringLength);

IntArrayPtr wordOrder(CharArrayPtr*, char ciphertex[]);

int findMatch(IntArrayPtr cipherWordLengths, IntArrayPtr cribWordLengths, CharArrayPtr* cipherWords,
               CharArrayPtr* cribWords, char ciphertext[], char cribtext[]);

bool decrypt(const char ciphertext[], const char crib[]);

bool containsNewline(CharArrayPtr* cipherWords, int startRow);

bool isSimpleSub(CharArrayPtr cipherJustLetters, CharArrayPtr cribJustLetters);

CharArrayPtr justLetters(int starPos, CharArrayPtr* wordsArray, char tex[]);

CharArrayPtr exchangeLetters(const CharArrayPtr cipherJustLetters, const CharArrayPtr cribJustLetters, char dupCipherText[], const int ciphertextlength, int len);



int main()
{
    
   /* char teststring[] = "Vlgmgak zfefyfektkyy.\nIfk!";
    
    cerr << stringLength(teststring) << endl;
    
    printString(teststring);
    
    cerr << countWords(teststring, stringLength(teststring)) << endl;
    
    cerr << strlen(teststring) << endl;
    
    
    char teststring1[] = "Vlgmgak4321fdfs!*(fd zfefyfektkyy.\nIfk!";
    
    createWordArray(teststring1, strlen(teststring1)); */
    
    char testcipher[] = "Vlghjl\n.hj oiup mgak zfefyfekt kyy.\nIfk!\n cddceb cd ncpq";
    char testcrib[] = "Attack at dawn!";
    char testcribx[] = "";
    char testcriby[] = "tank!";
    
    
    char testcipher2[] = "kyy.\nIfk!";
    
   // CharArrayPtr* cipher2Words = createWordArray(testcipher2, strlen(testcipher2));
    
  /*  CharArrayPtr* cipherWords = createWordArray(testcipher, strlen(testcipher));
    
    CharArrayPtr* cribWords = createWordArray(testcrib, strlen(testcrib));
    
    IntArrayPtr cipherWordLengths = wordOrder(cipherWords, testcipher);
    
    IntArrayPtr cribWordLengths = wordOrder(cribWords, testcrib);
    
   
    findMatch(cipherWordLengths, cribWordLengths, cipherWords, cribWords, testcipher, testcrib); */
    
    decrypt(testcipher, testcrib);
    
    char testciphercornercase[] = "nIfk!\n cddce";
    
    //decrypt(testciphercornercase, testcriby);
    
}

/* ==================================================================*/
// This function takes a duplicate ciphertext C-string, and iterates through it searching for alphabetic characters. If an alphabetic character is found and it matches any character in the sequence that matches the crib (cipherJustLetters), the function places the character in the duplicate ciphertext C-string with the corresponding uppercases letter in the crib (cribJustLetters). If the alphabetic character matches no characters in the sequence that matches the crib, the function returns the same letter, but lowercase. All non-alphabetic characters remain unchanged.
CharArrayPtr exchangeLetters(const CharArrayPtr cipherJustLetters, const CharArrayPtr cribJustLetters, char dupCipherText[], const int ciphertextlength, const int len)
{
    
    for (int k = 0; k < ciphertextlength; k++)
    {
        if (isalpha(dupCipherText[k]))
        {
            for (int j = 0; j < len; j++)
            {
                    if (toupper(dupCipherText[k]) == toupper(cipherJustLetters[j]))
                    {
                        dupCipherText[k] = cribJustLetters[j];
                        toupper(dupCipherText[k]);
                        break;
                    }
            dupCipherText[k] = tolower(dupCipherText[k]);
            }
        }
    }
    
    return dupCipherText;
    
}


/* ==================================================================*/
// This function takes a two-dimesional array of alphabetic and non-alphabetic characters and returns
// a one-dimensional array containing only alphabetic characters, preserving the order in which they appeared in the
// two-dimensional array.
CharArrayPtr justLetters(int starPos, CharArrayPtr* wordsArray, char text[])
{
    int nCols = 81;
    int nRows = countWords(text, strlen(text));  // Same as the output for the countWords function! Helpful to think of it as indicating rows in this case, though.
    int len = nCols * nRows;
    
    // Let's create a char array to store the letters.
    CharArrayPtr justLettersArrayPtr;
    justLettersArrayPtr = new char[len];

    //First, let's first fill the array with zero-characters.
    for (int k = 0; k < len; k++)
        justLettersArrayPtr[k] = '\0';
    
    
    //Next, let's fill the array we created with just the alphabetic characters of the putative matching sequence of ciphertext
    
    int positionIndicator = 0;
    
    for (int k = starPos; k < (starPos + nRows); k++)
    {
        for (int j = 0; j < nCols; j++)
        {
            if (wordsArray[k][j] == '\0')
                break;
            if (isalpha(wordsArray[k][j]))
            {
                justLettersArrayPtr[positionIndicator] = wordsArray[k][j];
                positionIndicator++;
            }
        }
    }
    
    for (int i = 0; i < positionIndicator; i++)
        cerr << justLettersArrayPtr[i] << ", ";
    cerr << endl << "============" << endl;
    
    // Return the one-dimensional array of just the alpabhetic characters
    return justLettersArrayPtr;
}


/* ==================================================================*/
// Now we need to check the two one-dimension arrays we just create against each other. We are checking to make sure the substitiution is simple.
bool isSimpleSub(CharArrayPtr justCipherLettersArrayPtr, CharArrayPtr justCribLettersArrayPtr)
{
    int n = strlen(justCipherLettersArrayPtr);
    
    
    
    for (int k = 0; k < n; k++)
    {
        for (int j = k; j < n; j++)
        {
            if (k != j)
            {
                // If the same cipher letter is replaced by two different crib letters,
                //the substitution scheme is not simple; return false
                if (toupper(justCipherLettersArrayPtr[k]) == toupper(justCipherLettersArrayPtr[j]))
                    if (toupper(justCribLettersArrayPtr[k]) != toupper(justCribLettersArrayPtr[j]))
                        return false;
            
                // If the same crib letter replaces two different crib letters,
                // the substitution scheme is not simple; return false
                if (toupper(justCribLettersArrayPtr[k]) == toupper(justCribLettersArrayPtr[j]))
                    if (toupper(justCipherLettersArrayPtr[k]) != toupper(justCipherLettersArrayPtr[j]))
                        return false;
            }
        }
    }
    

    
    //If the substitution scheme is simple, return true;
    return true;
}

/* ==================================================================*/
//This function check to see if there is a newLine character ('*') in the matched sequence. A newline character at the very start or the very end of the sequence is okay. A newline character anywhere else will cause the function to return false for the test.
bool containsNewline(CharArrayPtr* cipherWords, int startRow,
                  int cipherbWordLengthsArrayLength, int cribWordLengthsArrayLength)
{
    
    int nRows = cribWordLengthsArrayLength;
    int nCols = 81;
    
    for (int k = startRow; k < startRow + nRows; k++)
    {
        for (int j = 0; j < nCols; j++)
        {
                //The '*' character is used to indicate a newline
                if (cipherWords[k][j] == '*')
                {
                    // If the indexed element is at the very start of the sequence being tested, do nothing
                    if ((k == startRow) && (j == 0));
                    // If the indexed element is at the very end of the sequence being tested, do nothing
                    else if ((k == (nRows -1)) && (j == (nCols - 1)));
                    // If the indexed element is somehwere in the middle, return true
                    else
                        return true;
                }
        }
    }
    
    return false;
}


/* ==================================================================*/
// the findMatch function, that searches for a possible matching sequence in the int array. The implementation of the findMatch function calls two test functions -- containsNewLine and isSimpleSub -- which are detailed below. If a match is found, and it passes the test functions, findMatch returns an int that represents the row of cipherWords where the matching sequence begins. The findMatch function returns a value of -1 if no potential matches are found; decrypt returns false if findMatch returns -1.
int findMatch(IntArrayPtr cipherWordLengths, IntArrayPtr cribWordLengths, CharArrayPtr* cipherWords,
               CharArrayPtr* cribWords, char dupCipherText[], char dupCribText[])
{
    
    // Yes, the length OF the arrays containing the word lengths is the same as the numeber of words, but, again, it is helpful to think of the value as representing something else in this case.
    long cipherWordLengthsArrayLength = countWords(dupCipherText, strlen(dupCipherText));
    long cribWordLengthsArrayLength = countWords(dupCribText, strlen(dupCribText));
    
    
    //This array, containing just the alphabetic characters in the crib, is used to check that the substitution scheme is simple. The second array depends upon the position in the two-dimensional word array, so is declared in the loop.
    CharArrayPtr justCribLetters = justLetters(0, cribWords, dupCribText);
    
    
    //same value as cribWordLengthsArrayLength, but will be used for something else
    long matchCounter = countWords(dupCribText, strlen(dupCribText));
    
    int cipherPosIndex = 0;
    int cribPosIndex = 0;
    
    // This function is iterating through the 2 one-dimensional int arrays representing word length and order. The matchCounter is initialzied to the length of the crib text, and is decreaed by one each time  a CONSECUTIVE match is found. Once matchCounter reaches 0, the loop checks to make sure the subtitution scheme is simple and there are no newline characters in the possible matching sequence of ciphertext.
    for (int i = cribPosIndex; i < cribWordLengthsArrayLength; i++)
    {
        for(int j = cipherPosIndex; j < cipherWordLengthsArrayLength; j++)
        {
            cipherPosIndex++;
            
            cerr << "Ciphervalue is " << cipherWordLengths[j] << "; CribValue is "  << cribWordLengths[i] << endl;
            if (cipherWordLengths[j] == cribWordLengths[i])
            {
                matchCounter--;
                //cipherPosIndex++;
                
                // This is where the two test functions are called.
                if (matchCounter == 0)
                {
                    
                    //To make sure the substitution scheme is simple, we need an array containing just the letters of the possible matching sequence of the ciphertext
                    
                    int startPos = cipherPosIndex - cribWordLengthsArrayLength;
                                                          
                    CharArrayPtr justCipherLetters = justLetters(startPos, cipherWords, dupCribText);
                    
                    //Perform both the tests newLine Characters and simple substitution. If either test fails: set i to 0 (restart the loop through the cribWordLengths), set j to one greater the first element of the failed candidate sequence.
                    if(containsNewline(cipherWords, startPos,
                                       cipherWordLengthsArrayLength, cribWordLengthsArrayLength))
                    {
                        i = -1;  // -1 Instead of 0 so that the next increment will be 0,
                        cipherPosIndex -= (cribWordLengthsArrayLength - 1);    //This ensures that can check all but the first putative match element. This is for cases where
                        matchCounter = countWords(dupCribText, strlen(dupCribText));
                        
                        cerr << "Contains newLine" << endl;
                        break;
                    }
                    
                    if (!isSimpleSub(justCipherLetters, justCribLetters))
                    {
                        i = -1;  // -1 Instead of 0 so that the next increment will be 0,
                        cipherPosIndex -= (cribWordLengthsArrayLength - 1);    //This ensures that can check all but the first putative match element. This is for cases where
                        matchCounter = countWords(dupCribText, strlen(dupCribText));
                        
                        cerr << "Not simple substitution" << endl;
                        break;
                    }
                    
                    return startPos;
                }
                
                break;
            }
            
            if (cipherWordLengths[i] != cribWordLengths[j])
            {
                matchCounter = countWords(dupCribText, strlen(dupCribText));
            }
        
            //Do the testing somewhere around here
            
        }
    }
    
    if (matchCounter == 0)
        cerr << "We have possible match" << endl;
    else
        cerr<< "No possible match" << endl;
    
    
    
    // might want to use positionIndicator = positionIndicator - (cribWordLengthsArrayLength - 1) to make sure you are not missing possible matches
    
    return -1; // A return value of -1 means that no potential matches were found. decrypt function should return false if findMatch returns -1. HOWEVER what I want to return in the case of a match is a int that represents the row of cipherWords where the matching sequence begins. *This corresponds to the ith index of the cipherWordLengths array!
    
    
    
}


/* ==================================================================*/
// This function creates a one-dimensional int arrays that capture -- IN ORDER -- the word lengths in the two dimensional array, e.g. {2, 4 , 2, ... , 21}.
IntArrayPtr wordOrder(CharArrayPtr* cipherWords, char ciphertext[])  // CharArrayPtr* cribWords, char crib[])
{
    long nCols = 81;
    long nRows = countWords(ciphertext, strlen(ciphertext));
   // int cipherWordLengths[nRows];
    
    
    IntArrayPtr wordLengthsArrayPtr;
    
    wordLengthsArrayPtr = new int[nRows];

    
    
    for (int k = 0; k < nRows; k++)
    {
        int letterCount = 0;
        
        for (int j = 0; j < nCols; j++)
        {
            if (cipherWords[k][j] == '\0')
            {
                wordLengthsArrayPtr[k] = letterCount;
                break;
            }
            if (isalpha(cipherWords[k][j]))
                letterCount++;
        }
    }
    
    for (int i = 0; i < nRows; i++)
        cerr << wordLengthsArrayPtr[i] << ", ";
    
    cerr << endl << "============" << endl;
    
    return wordLengthsArrayPtr;
    
}


/*==============================================================================*/
// This function takes a one-dimensional c-string and creates a two-dimensional char array, with each row representing a possible word and the columns of each row representing the letters in each possible word.
CharArrayPtr* createWordArray(char cstring[], long stringLength)
{
    //no message within the ciphertext will be longer than 80 characters (not counting a newline at the end of the message). In other words, there will never be more than 80 characters between two newlines in the ciphertext or before the first newline or after the last newline. This means no word will be longer than 80 characters
    long nCols = 81;
    
    long nRows = countWords(cstring, strlen(cstring));
    
    
    // Let's create a 2D array with the rows for the number of possible words and the colomns for the maximum word length. Lets also fill the array with null-characters.
    
    CharArrayPtr *cypherTextWordArray;
    
    cypherTextWordArray = new CharArrayPtr[nRows];
    
    
    for (int i = 0; i < nRows; i++)
        cypherTextWordArray[i] = new char[nCols];
    // char cypherTextWordArray[nRows][nCols];
    
    
    for (int k = 0; k < nRows; k++)
    {
        for (int j = 0; j < nCols; j++)
        {
            cypherTextWordArray[k][j] = '\0';
        }
    }
    
    //We need to take a long C-string full of alphabetic and non-alphabetic characters and format it into the array, with each row representing one possible word. What this codeblock does it fill the 2-D char array with the alphabetic characters of the 1-D C-string. This codeblock also represents a newline character ('\n) as an *, so that there is a record of were messages begin and end. In certain cases, it is necessary to place an '.' so as not to disrupt the functionality of other functions that use the 2-D word array; most of the inner body of the loop is devoted to solving corner cases where an '.' is necessary; the '.' characters serve no purpose other than to facilitate array processing and can be largely ignored.
    int placeIndicator = 0;
    
        for (int k = 0; k < nRows; k++)
        {
            for (int j = 0; j < nCols; j++)
            {
                // If the indexed char is non-alphabetic and also not a newline
                if ((cstring[placeIndicator] != '\n') && !isalpha(cstring[placeIndicator]))
                    {
                        // Replaces the null character will a throwaway non-alphabetic character
                        if ((j == 0) || (cypherTextWordArray[k][j - 1] == '*'))
                            cypherTextWordArray[k][j] = '.';
                        
                        else if (!isalpha(cstring[placeIndicator - 1])); //Do nothing
                        
                        // If the thing before the char is not alphabetic and the thing after the char is alphabetic
                        else if (!isalpha(cstring[placeIndicator - 1]) && ((placeIndicator + 1) < stringLength) && isalpha(cstring[placeIndicator + 1]))
                            cypherTextWordArray[k][j] = '.';
                        
                        // If the ting before the char is alphabetic
                        else if (isalpha(cstring[placeIndicator - 1]))
                        {
                            cypherTextWordArray[k][j] = '\0';
                            placeIndicator++;
                            break;
                        }
                    }
                
                // If the indexed char is alphabetic
                if (isalpha(cstring[placeIndicator]))
                    cypherTextWordArray[k][j] = cstring[placeIndicator];
                
                // If the indexed char is a newline character
                if (cstring[placeIndicator] == '\n')
                {
                    if (cstring[placeIndicator - 1]  == '\n'); // Do nothing, two consecutive newline chars is unnecessary
                        
                    if (isalpha(cstring[placeIndicator - 1]))
                    {
                        cypherTextWordArray[k][j] = '*';
                        placeIndicator++;
                        break;
                    }
                    if (!isalpha(cstring[placeIndicator - 1]))
                        cypherTextWordArray[k][j] = '*';
                }
                placeIndicator++;
                
            }
        }
    
    
    for (int k = 0; k < nRows; k++)
    {
        cerr << endl;
        for (int j = 0; j < nCols; j++)
        {
            cerr << cypherTextWordArray[k][j];
        }
    }
            
    cerr << endl;
    
    return cypherTextWordArray;
}


/*==============================================================================*/
//This function takes as inputs a C-string and a long representing the lenngth of that C-string and returns the number of possible words in that string. It is used to determine the number of rows of the two-dimensional word array.
long countWords( const char cstring[], long stringLength)
{
    long wordCount = 0;
    
    for (int k = 0; k < stringLength; k++)
    {
        
        if (isalpha(cstring[k]) && !isalpha(cstring[k + 1]))
            wordCount++;
    }
    return wordCount;
}


/* ==================================================================*/
// This is the primary decrypt function, but most of the action takes place in functions called by this function. Some preliminary testing is done within the body of the decrypt function, and there is a commented section that explains what the decrypt function is doing with the functions it calls. After the commeneted section, the decrypt function executes the core routines of the cyptanalysis.
bool decrypt(const char ciphertext[], const char crib[])
{
    
    // You must not assume any particular limit to the possible length of the crib string argument that is passed to the function.
    long cribLength = strlen(crib);
    long cipherLength = strlen(ciphertext);
    
    //Coping ciphertext and the crib C-strings into an array that can be manipulated. All the array operations that follow will manipulate these duplicate C-string arrays, leaving the arrays passed 'by reference' to the function unchanged.
    char dupCipherText[cipherLength + 1];
    char dupCribText[cribLength + 1];
    
    strcpy(dupCipherText, ciphertext);
    strcpy(dupCribText, crib);
    
    //Let's find out how many words are in the ciphertext and the crib
    long wordsInCipher = countWords(dupCipherText, cipherLength);
    long wordsInCrib = countWords(dupCribText, cribLength);
    
    //Now we have duplicate crib and ciphertext C-string arrays that we can manipulate. We know how many characters are in each and we know how mnay 'words' are in each. We can now do some preliminary testing:
    
    // If the crib is the empty C-string. Condition from specifications.
    if (strcmp(crib, "") == false)                                        // strcmp -- what does it do?
        return false;
    
    // If the crib is longer than the cyphertext. The crib cannot be longer than the ciphertext!
    if(cribLength > cipherLength)
        return false;
    
    
    // If the crib is longer than 80 characters, it cannot fill inside one message, and so cannot be encoded
    if (cribLength > 80)
        return false;

    //If there are more words in the crib than in the ciphertext
    if(wordsInCrib > wordsInCipher)
        return false;
    
    // If none of the characters in the cyphertext are alphabetic
    for (int k = 0; k < cipherLength; k++)
    {
        int count = 0;
        if (isalpha(ciphertext[count]))
            break;
        count++;
        
        if (count == strlen(ciphertext))
            return false;
    }

    
    /* ========================================================================================================
     Extended Documentation:
     
     The next sequence of operations wwill consist largely of the decrypt funtion calling functions that create and compare arrays with various propertiess. The sequence is as follows:
    
        1.) Create 2 two-dimensional char arrays: one for the ciphertext, one for the cribtext. Each array represents the words in ciphertext or cribtext
     
        2.) Create 2 one-dimensional int arrays that capture -- IN ORDER -- the word lengths in the two dimensional array
     
        3.) Call the findMatch function, that searches for a possible matching sequence in the int array. The implementation of the findMatch function calls two test functions -- containsNewLine and isSimpleSub -- which are detailed below. If a match is found, and it passes the test functions, findMatch returns an int that represents the row of cipherWords where the matching sequence begins. The findMatch function returns a value of -1 if no potential matches are found; decrypt returns false if findMatch returns -1.
     
                a) A crib cannot match a sequence of the cipher that contains a newline character; the containsNewLine function returns true if there is such newline (represented by '*'). If this function returns true, the sequence cannot be matched to the crib.
     
                b) The substitution scheme must be one-to-one, e.g. if every E is replaced by K, no other letter will also be replaced by K; the isSimpleSub returns true is the substitution scheme is one-to-one. If this function returns true, the sequence may be matched to the crib, provided it does not contain a newline character.
     
        4.) Assuming there is a good match, take dupCipherText and exhange the letters appropriately
        5.) Print out the partially-decrypted dupCipherText.
     
     Will Provide in-line documentation step by step!
     ======================================================================================================== */
    
    
    int nMessageRows = 60;
    int nCols = 81;

    // Create the 2 two-dimensional word arrays
    CharArrayPtr* cipherWords = createWordArray(dupCipherText, cipherLength);
    CharArrayPtr* cribWords = createWordArray(dupCribText, cribLength);
    
    // Create the 2 one-dimensional word-length arrays
    IntArrayPtr cipherWordLengths = wordOrder(cipherWords, dupCipherText);
    IntArrayPtr cribWordLengths = wordOrder(cribWords, dupCribText);
    
    // Search for and test possible matches
    int startPosition = findMatch(cipherWordLengths, cribWordLengths, cipherWords, cribWords, dupCipherText, dupCribText);
    
    cerr << "The starting row of the matching sequence is: " << startPosition << endl;
    
    // If findMatch did not return any matches
    if (startPosition == -1)
        return false;
    
    // Now we have the alphabetic sequence from the cipher that matches the crib. The letters are in order, so the exhange will be simple.
    CharArrayPtr justCribLettersKey = justLetters(0, cribWords, dupCribText);
    CharArrayPtr justCipherLettersKey = justLetters(startPosition, cipherWords, dupCribText);
    
    int keyLength = 0;
    int k = 0;
    
    while (justCribLettersKey[k] != '\0')
    {
        keyLength++;
        k++;
    }
    
    
    // Make all the crib letters uppercase
    for (int k =0; k < keyLength; k++)
        justCribLettersKey[k] = toupper(justCribLettersKey[k]);
    
    for (int k =0; k < keyLength; k++)
        cerr << justCribLettersKey[k];
    
    // This functinon will do the exchange.
    CharArrayPtr decyptedCipher = exchangeLetters(justCipherLettersKey, justCribLettersKey, dupCipherText, cipherLength, keyLength);
    
    cerr << "so far so good" << endl;
    
    
    // Now just print out partially decryped Cipher!
    
    for (int k = 0; k < cipherLength; k++)
        cerr << decyptedCipher[k];
    
    cerr << endl;
    
    return true;
}
