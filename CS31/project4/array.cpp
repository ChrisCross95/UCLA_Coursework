#include <iostream>
#include <string>
#include <cassert>
using namespace std;

int countMatches(const string a[], int n, string target);

int detectMatch(const string a[], int n, string target);

bool detectSequence(const string a[], int n, string target, int& begin, int& end);

int detectMin(const string a[], int n);

int moveToBack(string a[], int n, int pos);

int moveToFront(string a[], int n, int pos);

int detectDifference(const string a1[], int n1, const string a2[], int n2);

int deleteDups(string a[], int n);

bool contains(const string a1[], int n1, const string a2[], int n2);

int meld(const string a1[], int n1, const string a2[], int n2,
         string result[], int max);

int split(string a[], int n, string splitter);


int main()
{
    
  /*  string h[7] = { "romanoff", "thor", "rogers", "banner", "", "danvers", "rogers" };
    assert(countMatches(h, 7, "rogers") == 2);
    assert(countMatches(h, 7, "") == 1);
    assert(countMatches(h, 7, "rhodes") == 0);
    assert(countMatches(h, 0, "rogers") == 0);
    assert(detectMatch(h, 7, "rogers") == 2);
    assert(detectMatch(h, 2, "rogers") == -1);
    int bg;
    int en;
    assert(detectSequence(h, 7, "banner", bg, en) && bg == 3 && en == 3);
    
    string g[4] = { "romanoff", "thor", "banner", "danvers" };
    assert(detectMin(g, 4) == 2);
    assert(detectDifference(h, 4, g, 4) == 2);
    assert(contains(h, 7, g, 4));
    assert(moveToBack(g, 4, 1) == 1 && g[1] == "banner" && g[3] == "thor");
    
    string f[4] = { "danvers", "banner", "thor", "rogers" };
    assert(moveToFront(f, 4, 2) == 2 && f[0] == "thor" && f[2] == "banner");
    
    string e[5] = { "danvers", "danvers", "danvers", "thor", "thor" };
    
    deleteDups(e, 5);
    
    for (int j = 0; j < deleteDups(e, 5); j++)
    {
        cerr << e[j] << ", ";
    }
    
    assert(deleteDups(e, 5) == 2 && e[1] == "thor");
    
    string x[4] = { "rhodes", "rhodes", "tchalla", "thor" };
    string y[4] = { "banner", "danvers", "rhodes", "rogers" };
    string z[10];
    
    meld(x, 4, y, 4, z, 10);
    
    for (int k = 0; k != 8; k ++)
    {
        cerr << z[k] << ", ";
    }
    
    assert(meld(x, 4, y, 4, z, 10) == 8 && z[5] == "rogers");
    
    split(h, 7, "rogers");
    
    for (int k = 0; k != 7; k++)
    {
        cerr << h[k] << ", ";
    }
    
    cerr << endl;
    
    assert(split(h, 7, "rogers") == 3);
    
    cout << "All tests succeeded" << endl; */
    
    string a[] = {"Tom", "Dick", "Harry"};
    string b[] = {};
    
    assert(contains(a, 3, b, 0) == true);
    cerr << "Good!";
}


/* =======================================================================================*/
/* Returns the number of strings in the array that are equal to target. Arguments are string array a and int n, specifying size of array a */

int countMatches(const string a[], int n, string target)
{
    if (n < 0)
        return -1;
    
    int numberOfMatchingStrings = 0;
    
    for (int k = 0; k != n; k++)
    {
        if (a[k] == target)
            numberOfMatchingStrings ++;
    }
    return numberOfMatchingStrings;
}


/* =======================================================================================*/
/* Return sthe position of a string in the array that is equal to target; if there is more than one such string, return the smallest position number of such a matching string. Returns −1 if there is no such string. Arguments: string array a, n, string to be matched. */

int detectMatch(const string a[], int n, string target)
{
    if(n < 0)
        return -1;
    
    int indexOfFirstMatch;
    
    for (int k = 0; k != n; k++)
    {
        if (a[k] == target)
        {
            indexOfFirstMatch = k;
            return indexOfFirstMatch;
        }
    }
    return -1;
}


/* =======================================================================================*/
/* Finds the EARLIEST occurrence in a of one or more consecutive strings that are equal to target; sets begin to the position of the first occurrence of target, sets end to the last occurrence of target in that earliest consecutive sequence, and return trues. If n is negative or if no string in a is equal to target, leaves begin and end unchanged and returns false */

bool detectSequence(const string a[], int n, string target, int& begin, int& end)
{
    if (n < 0)
        return false;
    
    for (int k = 0; k != n; k++)
    {
        if (a[k] == target)
        {
            begin = k;
            end   = k;
            
            if (k == (n - 1))
                return true;
            else
                for (int j = k+1; j != n; j++)
                {
                    if (a[j] == target)
                        end = j;
                    
                    if (!(a[j] == target))
                        return true;
                }
            return true;
        }
    }
    return false;
}


/* =======================================================================================*/
/* Returns the position of a string in the array such that that string is <= every string in the array. If there is more than one such string, returns the smallest position number of such a string. Returns −1 if the function should examine no elements of the array.*/

int detectMin(const string a[], int n)
{
    if(n <= 0)
        return -1;
    
    int minIndex = 0101;
    
    string minValue = a[0];
    
    for (int k = 0; k != n; k++)
    {
        if (minValue > a[k])
        {
            minValue = a[k];
            minIndex = k;
        }
        
        if ((minValue == a[k]) && !(k == minIndex))
        {
            if (k < minIndex)
                minIndex = k;
        }
            }
    
    if (minIndex == 0101)
        return -1;
    
    else
        return minIndex;
}


/* =======================================================================================*/
/* Eliminates the item at position pos by copying all elements after it one place to the left. Puts the item that was thus eliminated into the last position of the array. Returns the original position of the item that was moved to the end. */

int moveToBack(string a[], int n, int pos)
{
    if (n < 0)
        return -1;
    
    if (pos > (n - 1))  //makes sure position is in range of index
        return -1;
    
    if (pos < 0)
        return -1;
    
    if (n == 0)
        return 0; // what to do about empty strings?
    
    if (n == 1)
        return 0;
    
    string valueHolder = a[pos];
    
    for (int k = pos; k != (n - 1); k++)
    {
        a[k] = a[k + 1];
    }
    a[n - 1] = valueHolder;
    
    return pos;
}


/* =======================================================================================*/
/* Eliminates the item at position pos by copying all elements before it one place to the right. Puts the item that was thus eliminated into the first position of the array. Returns the original position of the item that was moved to the beginning. */

int moveToFront(string a[], int n, int pos)
{
    if (n < 0)
        return -1;
    
    if (pos > (n - 1))  //makes sure position is in range of index
        return -1;
    
    if (pos < 0)
        return -1;
    
    string valueHolder = a[pos];
    
    for (int k = pos; k > 0; k--)
    {
        a[k] = a[k - 1];
    }
    a[0] = valueHolder;
    
    return pos;
}


/* =======================================================================================*/
/* Returns the position of the first corresponding elements of a1 and a2 that are not equal. n1 is the number of interesting elements in a1, and n2 is the number of interesting elements in a2. If the arrays are equal up to the point where one or both runs out, returns whichever value of n1 and n2 is less than or equal to the other.*/

int detectDifference(const string a1[], int n1, const string a2[], int n2)
{
    
    int indexBound = 0;
    
    if (n1 > n2)
        indexBound = n2;
    
    else
        indexBound = n1;
        
    
    for (int k = 0; k < indexBound; k++)
    {
        if (!(a1[k] == a2[k]))
            return k;
    }
    
    return indexBound;
}


/* =======================================================================================*/
/* For every sequence of consecutive identical items in a, retains only one item of that sequence. Suppose we call the number of retained items r. Then when this functions returns, elements 0 through r-1 of a must contain the retained items (in the same relative order they were in originally), and the remaining elements may have whatever values you want. Returns the number of retained items. */

int deleteDups(string a[], int n)
{
    if (n < 0)
        return -1;
    
    int begin;
    int end;
    
    int retainedItems = n;
    
    for (int k = 0; k != n; k++)
    {
        detectSequence(a, n, a[k], begin, end);
        
        if (detectSequence(a, n, a[k], begin, end) == true)
        {
            int indexGap = (end - begin);
            
            if (a[k] == " *deleted* ")
                break;
            
            for (int j = 1; j != (indexGap + 1); j++)
            {
                a[begin + j] = " *deleted* ";
            }
        }
    }
    
    for (int j = 0; j != n; j++)
    {
        if (a[j] == " *deleted* ")
        {
            moveToBack(a, n, j);
            retainedItems --;
        }
    }
    
    return retainedItems;
}


/* =======================================================================================*/
/* If all n2 elements of a2 appear in a1, in the same order (though not necessarily consecutively), then returns true. Returns false if a1 does not so contain a2. (Of course, every sequence, even a sequence of 0 elements, contains a sequence of 0 elements.) Returns false (instead of −1) if passed any bad arguments.*/

bool contains(const string a1[], int n1, const string a2[], int n2)
{
    if ((n1 < 0) || (n2 < 0))
        return false;
    
    if (n1 < n2)
        return false;
    
    if (n2 == 0)
        return true;
    
    int indexPlace = 0;
    int matches = 0;
    
    for (int k = 0; k != n2; k++)
    {
        
        for (int j = indexPlace; j != n1; j++)
        {
            if (a2[k] == a1[j])
            {
                matches ++;
                indexPlace = j + 1;
                break;
            }
                
        }
    }
    
    if (matches == n2)
        return true;
    else
        return false;
}


/* =======================================================================================*/
/* If a1 has n1 elements in nondecreasing order, and a2 has n2 elements in nondecreasing order, places in result all the elements of a1 and a2, arranged in nondecreasing order, and returns the number of elements so placed. Returns −1 if the result would have more than max elements or if a1 and/or a2 are not in nondecreasing order. (Note: nondecreasing order means that no item is > the one that follows it.) */

int meld(const string a1[], int n1, const string a2[], int n2,
         string result[], int max)
{
    int meldedArraySize = n1 + n2;
    int placeHolder = 0;
    
    if ((n1 < 0) || (n2 < 0) || (max < 0))
        return -1;
    
    if (meldedArraySize > max)
        return -1;
    
    
    for (int k = 0; k != (n1 - 1); k++)
    {
        if (a1[k] > a1[k + 1])
            return -1;
    }
    
    for (int j = 0; j != (n2 - 1); j++)
    {
        if (a2[j] > a2[j + 1])
            return -1;
    }
    
    for (int w = 0; w != n1; w++)
    {
        result[w] = a1[w];
        placeHolder ++;
    }
    
    for (int w = 0; w != n2; w++)
    {
        result[w + placeHolder] = a2[w];
    }
    
    for (int w = 0; w != meldedArraySize; w++)
    {
        for (int j = w; j != meldedArraySize; j++)
        {
            string placeHolder;
            
            if (result[j] < result[w])
            {
                placeHolder = result[w];
                result[w] = result[j];
                result[j] = placeHolder;
            }
                
        }
    }
    
    return meldedArraySize;
}


/* =======================================================================================*/
/* Rearranges the elements of the array so that all the elements whose value is < splitter come before all the other elements, and all the elements whose value is > splitter come after all the other elements. Returns the position of the first element that, after the rearrangement, is not < splitter, or n if there are no such elements.*/

int split(string a[], int n, string splitter)
{
    if (n < 0)
        return -1;
    
    int placeIndicator = 0;
    string tempString;
    
    for (int k = 0; k != n; k++)
    {
        
        if (a[k] < splitter)
        {
            cerr << a[k] << " is < than " << splitter << endl;
            
            moveToFront(a, n, k);
            placeIndicator++;
        }
    }
    
return placeIndicator;
}
