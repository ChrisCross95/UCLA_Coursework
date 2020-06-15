
#include <string>
using namespace std;

// Returns the product of two non-negative integers, m and n,
// using only repeated addition.
int prod(unsigned int m, unsigned int n)
{
    if (m == 0 || n == 0)
        return 0;
    else
        return m + prod(m, n - 1);
}

// Returns the number of occurrences of digit in the decimal
// representation of num. digit is an int between 0 and 9
// inclusive.
//
// Pseudocode Example:
// numberOfDigits(18838, 8) => 3
// numberOfDigits(55555, 3) => 0
// numberOfDigits(0, 0) => 0 or 1 (either if fine)
//
int numberOfDigits(int num, int digit)
{
    if (num == 0)
        return 0;
    else {
        int remainder = (num - digit) % 10;
        if (remainder == 0)
            return 1 + numberOfDigits(num/10, digit);
        else
            return 0 + numberOfDigits(num/10, digit);
    }
}

// Returns a string where the same characters next each other in
// string n are separated by "22"
//
// Pseudocode Example:
// doubleDouble("goodbye") => "go22odbye"
// doubleDouble("yyuu") => "y22yu22u"
// doubleDouble("aaaa") => "a22a22a22a"
//
string doubleDouble(string n) {
    
    string returned;
    if (n == "\0")
        return "";
    else {
        char savedChar = n[n.length() - 1];
        n.pop_back();
        string next = doubleDouble(n);
        if (savedChar == next[next.length() - 1]) {
            returned += next;
            returned += "22";
            returned += savedChar;
        }
        else {
            returned += next;
            returned += savedChar;
        }
        return returned;
    }
}
// str contains a single pair of curly brackets, return a new
// string made of only the curly brackets and whatever those
// curly brackets contain
//
// Pseudocode Example:
// curlyFries("abc{ghj}789") => "{ghj}"
// curlyFries("{x}7") => "{x}"
// curlyFries("4agh{y}") => "{y}"
// curlyFries("4agh{}") => "{}"
//
string curlyFries(string str)
{
    string returned;
    if (str == "\0")
        return "";
    else {
        char savedChar = str[str.length() - 1];
        str.pop_back();
        returned = curlyFries(str);
        if (savedChar == '{') {
            returned += "{*";
        }
        else if (savedChar == '}') {
            returned.pop_back();
            returned += '}';
        }
        else if (returned[returned.length() - 1] == '*') {
            returned.pop_back();
            returned += savedChar;
            returned += '*';
        }
        else;
    }
    return returned;
}


// Return true if the total of any combination of elements in
// the array a equals the value of the target.
//
// Pseudocode Example:
// addEmUp([2, 4, 8], 3, 10) => true
// addEmUp([2, 4, 8], 3, 6) => true
// addEmUp([2, 4, 8], 3, 11) => false
// addEmUp([2, 4, 8], 3, 0) => true
// addEmUp([], 0, 0) => true
//
bool addEmUp(const int a[], int size, int target)
{

    int current = *a;
    if (target < 0)
        return false;
    if ((target - current == 0) || target == 0)
        return true;
    if (size == 0)
        return false;
    if (current == target)
        return true;
    // Checks each element against the target & leftwise cumulative sum
    else if (addEmUp(a + 1, size - 1, target - current))
        return true;
    // Do not consider the current value within the leftwise cumulative sum
    else if (addEmUp(a + 1, size - 1, target))
        return true;
    return false;
}

// Return true if there is a path from (sr,sc) to (er,ec)
// through the maze; return false otherwise
bool canWeFinish(string maze[], int nRows, int nCols,
                 int sr, int sc, int er, int ec) {
    
    maze[sr][sc] = '#'; // drop crumb
    if (sr == er && sc == ec)
        return true;
    if (maze[sr - 1][sc] == '.') {
        if (canWeFinish(maze, nRows, nCols, sr - 1, sc, er, ec))
            return true;
    }
    if (maze[sr + 1][sc] == '.') {
        if (canWeFinish(maze, nRows, nCols, sr + 1, sc, er, ec))
            return true;
    }
    if (maze[sr][sc + 1] == '.') {
        if (canWeFinish(maze, nRows, nCols, sr, sc + 1, er, ec))
            return true;
    }
    if (maze[sr][sc - 1] == '.') {
        if (canWeFinish(maze, nRows, nCols, sr, sc - 1, er, ec))
            return true;
    }
    return false;
}
