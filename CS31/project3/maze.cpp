#include "grid.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cassert>
using namespace std;

int r, c;
int driveSegmentEast(int r, int c);
bool isCourseWellFormed(string course);
bool isValidPosition(int r, int c);
int driveSegment(int r, int c, char dir, int maxSteps);
int driveCourse(int sr, int sc, int er, int ec, string course, int& nsteps);
void updatePosition(int& currentRow, int& currentCol, char dir, int distance);

int stringtoInteger(char a);

#define last_char_is_digit (isdigit(course[k + 1])) && ((k + 2) == course.size())
#define next_char_exists (isalpha(course[k])) && ((k + 1) < course.size())
#define two_char_after_dir_char (isalpha(course[k])) && ((k + 1) < course.size()) && isdigit(course[k + 1]) && ((k + 2) < course.size()))

int main()
{
   /* setSize(25,25);      // make a 3 by 4 grid
    setWall(1,4);      // put a wall at (1,4)
    setWall(2,2);       // put a wall at (2,2)
    setWall(5,6);
    setWall(3,5);
    setWall(4,3);
    setWall(5,5);
    setWall(3,9);
    if (!isWall(3,2))  // if there's no wall at (3,2)  [there isn't]
        setWall(3,2);  //    put a wall at (3,2)
    draw(3,1, 3,4);    // draw the grid, showing an S at (3,1)
    //    start position, and an E at (3,4)
    
    */
    
   /* assert (!isdigit('+') == true);
    assert (!isalpha('+') == true);
    
    assert(isCourseWellFormed("N") == true);
    assert(isCourseWellFormed("N2eE01n0e2e1") == true);
    assert(isCourseWellFormed("NW42") == true);
    assert(isCourseWellFormed("w2+n3") == false);
    assert(isCourseWellFormed("N144") == false);
    assert(isCourseWellFormed("e1x") == false);
    assert(isCourseWellFormed("3sn") == false);
    assert(isCourseWellFormed("") == true);
    assert(isCourseWellFormed(" ") == false);
    assert(isCourseWellFormed("EEEE") == true);
    assert(isCourseWellFormed("43424e") == false);
    assert(isCourseWellFormed("434423") == false);
    assert(isCourseWellFormed("EEE43WWW2NNNN3") == true);
    
    cerr << "All tests succeded!" << endl;
    
    cerr << driveSegment(1, 1, 'S', 7) << endl;
    cerr << driveSegment(4, 10, 'W', 7) << endl;
    cerr << driveSegment(5, 7, 'N', 7) << endl;
    cerr << driveSegment(4, 1, 'E', 7) << endl;
    cerr << driveSegment(1, 1, 'e', 7) << endl;
    assert(isValidPosition(4, 7) == true);
    assert(isValidPosition(4, 3) == false);
    
    assert(driveSegment(1, 1, 'e', 7) == 2);
    assert(driveSegment(1, 1, 'S', 3) == 3);
    assert(driveSegment(1, 1, 'N', 7) == 0);
    assert(driveSegment(1, 1, 'W', 7) == 0);
    assert(driveSegment(3, 1, 'W', 7) == 0);
    assert(driveSegment(4, 10, 'W', 7) == 6);
    assert(driveSegment(3, 8, 'E', 7) == 0);
    assert(driveSegment(5, 7, 'N', 7) == 4);
    assert(driveSegment(1, 10, 'W', 7) == 5);
    assert(driveSegment(4, 1, 'e', 7) == 1);
    assert(driveSegment(2, 5, 's', 7) == 0);
    assert(driveSegment(3, 1, 'W', 7) == 0);
    assert(driveSegment(3, 1, 'd', 7) == -1);
    assert(driveSegment(3, 1, 'd', -7) == -1);
    cerr << "All tests succeded!" << endl;
    
    int nSteps = 0;
    
    assert(driveCourse(1, 15, 3, 7, "E", nSteps) == 1);
    assert(driveCourse(1, 1, 1, 4, "E2S", nSteps) == 2);
    assert(driveCourse(1, 3, 4, 5, "W432", nSteps) == 2);
    assert(driveCourse(4, 4, 4, 4, "", nSteps) == 0);
    assert(driveCourse(1, 3, 1, 5, "", nSteps) == 1);
    
    
    assert(driveSegment(1, 1, 'e', 9) == 2);
    cerr << driveSegment(1, 1, 'e', 9) << endl;
    cerr << driveCourse(1, 1, 1, 10, "w", nSteps) << endl;
    assert(driveCourse(1, 1, 1, 10, "w", nSteps) == 3);
    assert(driveCourse(1, 1, 1, 11, "w", nSteps) == 3);
    cerr << driveCourse(1, 1, 1, 1, "w", nSteps) << endl;
    assert(driveCourse(1, 1, 1, 1, "w", nSteps) == 3);
    assert(driveCourse(1, 1, 1, 1, "s", nSteps) == 1);
    assert(driveCourse(1, 1, 2, 1, "s", nSteps) == 0);
    
    // int nSteps = 0;
    cerr << "====================" << endl;
   cerr << driveCourse(1, 1, 3, 1, "ss", nSteps) << endl;
    nSteps = 0;
    assert(driveCourse(1, 1, 3, 1, "ss", nSteps) == 0);
    nSteps = 0;
    cerr << driveCourse(5, 3, 5, 1, "w2", nSteps) << endl;
    nSteps = 0;
    assert(driveCourse(5, 3, 5, 1, "w2", nSteps) == 0);
    nSteps = 0;
    //cerr << driveCourse(2, 3, 2, 8, "w5", nSteps) << endl;
   // nSteps = 0;
     // assert(driveCourse(5, 3, 5, 2, "w2", nSteps) == 1);
    
    //cerr << driveCourse(2, 1, 1, 3, "nE2", nSteps) << endl;
    nSteps = 0;
    // cerr << "============" << endl;
    // cerr << driveCourse(2, 1, 1, 3, "nE2see6s3", nSteps) << endl;
   // nSteps = 0;
    // cerr << driveCourse(2, 1, 24, 11, "nE2see6s31ne", nSteps) << endl;
    
    cerr << driveCourse(2, 1, 24, 11, "nE2see6s23ne", nSteps) << endl;
    // assert(driveCourse(2, 1, 24, 11, "nE2see6s31ne", nSteps) == 0);
    cerr << nSteps << endl;
    cerr << driveSegment(1, 1, 'e', 7) << endl; */
    
    
    setSize(2,5);      // make a 2 by 5 grid
    setWall(2,2);
    draw(2,5, 1,3);
    
    int nsteps;
    //int nsteps = 0;
    cerr << driveCourse(2,5, 1,3, "w2n1", nsteps) << endl;
    cerr << nsteps << endl;
    cerr << "============" << endl;
    
    //nsteps = 0;
    cerr << driveCourse(2,5, 1,3, "w1n1", nsteps) << endl;
    cerr << nsteps << endl;
    cerr << "============" << endl;
    
    //nsteps = 0;
    cerr << driveCourse(2,5, 1,3, "w4n1", nsteps) << endl;
    cerr << nsteps << endl;
    cerr << "============" << endl;

    
    // cerr << "All tests succeded!" << endl;

    }


int driveCourse(int sr, int sc, int er, int ec, string course, int& nsteps)
{
    
    int currentRow = sr;
    int currentCol = sc;
    
    
    if ((!isValidPosition(sr, sc)) || (!isValidPosition(er, ec)))
        return 2;
    
    if (!isCourseWellFormed(course))
        return 2;
    
    if (course == "")
    {
        if ((sr == er) && (sc == ec))
            return 0;
    
        else
            return 1;
    }
    
    nsteps = 0;
    
    for (int k = 0; k < course.size(); k++)
    {
        
            if ((isalpha(course[k])) && (k == (course.size() - 1)))  //If the direction char is the last or only character
                {
                    nsteps += driveSegment(currentRow, currentCol, course[k], 1);
                    
                    cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                    cerr << nsteps << endl;
                    cerr << 'x' << endl;
                    
                    if (driveSegment(currentRow, currentCol, course[k], 1) < 1)
                        return 3;
                    
                    updatePosition(currentRow, currentCol, course[k],
                                   driveSegment(currentRow, currentCol, course[k], 1));
                    
                    cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                }
            
            if ((isalpha(course[k])) && ((k + 1) < course.size()))   // If there is a next character
                    {
                        
                        if(isalpha(course[k + 1]))  // If the next character is a direction character
                        {
                            nsteps += driveSegment(currentRow, currentCol, course[k], 1);
                            
                            cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                            cerr << nsteps << endl;
                            cerr << 'y' << endl;
                        
                            if ((driveSegment(currentRow, currentCol, course[k], 1) < 1))
                                return 3;
                            
                            updatePosition(currentRow, currentCol, course[k],
                                           driveSegment(currentRow, currentCol, course[k], 1));
                        }
                        
                        // If the next character is a digit and is also the last character in the string
                        if ((isdigit(course[k + 1])) && ((k + 2) == course.size()))
                        {
                            
                            const int valueofIndex = stringtoInteger(course[k + 1]);
                            
                            nsteps += driveSegment(currentRow, currentCol, course[k], valueofIndex);
                            
                            // assert(driveSegment(currentRow, currentCol, course[k], valueofIndex) == 5);
        
                            cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                            cerr << "The number of steps is: " << valueofIndex << endl;
                            cerr << "The number of steps proposed by the course is: " << valueofIndex << endl;
                            cerr << 'z' << endl;
                            
                            if (driveSegment(currentRow, currentCol, course[k], valueofIndex) < valueofIndex)
                                return 3;
                            
                            updatePosition(currentRow, currentCol, course[k],
                                               driveSegment(currentRow, currentCol, course[k], valueofIndex));
                            
                            cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                        }
                    }
        
            // If there is another character (two more after the dir char)
            if ((isalpha(course[k])) && ((k + 1) < course.size()) && isdigit(course[k + 1]) && ((k + 2) < course.size()))
                        {
                            if (isalpha(course[k + 2])) //If the second next character is alphabetic
                            {
                                const int valueofIndex = stringtoInteger(course[k + 1]);
                                
                                nsteps += driveSegment(currentRow, currentCol, course[k], valueofIndex);
                                
                                cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                                cerr << "The number of steps is: " << valueofIndex << endl;
                                cerr << "The number of steps proposed by the course is: " << valueofIndex << endl;
                                cerr << 'w' << endl;
                                
                                if ((driveSegment(currentRow, currentCol, course[k], valueofIndex) < valueofIndex))
                                    return 3;
                                
                                updatePosition(currentRow, currentCol, course[k],
                                               driveSegment(currentRow, currentCol, course[k], valueofIndex));
                                
                                cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                                
                            }
                            
                            if (isdigit(course[k + 2])) // If the second next character is a digit
                            {
                                const int valueofIndex = (10*stringtoInteger(course[k + 1])) + stringtoInteger(course[k + 2]);
                                
                                nsteps += driveSegment(currentRow, currentCol, course[k], valueofIndex);
                                
                                cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                                cerr << "The number of steps is: " << valueofIndex << endl;
                                cerr << "The number of steps proposed by the course is: " << valueofIndex << endl;
                                cerr << 't' << endl;
                                
                                if ((driveSegment(currentRow, currentCol, course[k], valueofIndex) < valueofIndex))
                                    return 3;
                                
                                updatePosition(currentRow, currentCol, course[k],
                                               driveSegment(currentRow, currentCol, course[k], valueofIndex));
                                
                                cerr << "(" << currentRow << ", " << currentCol << ")" << endl;
                            }
                        }
                    }
    
    if ((currentRow == er) && (currentCol == ec)) {
        return 0;
    } else {
        return 1;
    }
}

// Try writing a function that determines how far you can go in a straight line to the east through the grid starting from a given position, without running off the edge of the grid or hitting a wall.
//Hint: Your implementation of this function will surely call getRows, getCols and isWall.

int driveSegment(int r, int c, char dir, int maxSteps)
{
    const string validDirections = "NnSsEeWw";
    int maxDistance = 0;
    int rowLength = getCols();
    int colLength = getRows();
    
    //The following two if-statements ensure the current position is a valid empty grid position
    if (!isValidPosition(r,c))
        return -1;
    
    //This if-statement ensures that the number for the maximun number of steps is positive
    if (maxSteps < 0)
        return -1;
    
    //The following for-loop ensures the direction is a valid, cardinal direction
    for (int j = 0; j != validDirections.size(); j++)
    {
        if (dir == validDirections[j])
            break;
        if ((j == validDirections.size() - 1) && !(dir == validDirections[j]))
            return -1;
    }
    
    // For the parameters maxSteps and dir, if the car starting at (r,c) could indeed travel that number of steps in that direction without moving to a cell containing a wall or running off the edge of the grid, then the function returns that number of steps; otherwise, the function returns the maximum number of valid steps in that direction the car could travel. Each of the four code blocks calculates this value for each of the four possible directions, i.e. N,S,E,W.
    
    if (toupper(dir) == 'N')
    {
        for (int i = r; i > 0; i--)
        {
            if(isValidPosition(i - 1, c))
            {
                maxDistance ++;
                
                if (maxDistance == maxSteps)
                {
                    return maxSteps;
                }
            }
            if (!isValidPosition(i - 1, c))
            {
                return maxDistance;
            }
        }
        return maxDistance;
    }
    
    if (toupper(dir) == 'S')
    {
        for (int i = r; i != colLength; i++)
        {
            if(isValidPosition(i + 1, c))
            {
                maxDistance ++;
                
                if (maxDistance == maxSteps)
                {
                    return maxSteps;
                }
            }
            if (!isValidPosition(i + 1, c))
            {
                return maxDistance;
            }
        }
        return maxDistance;
    }
    
    if (toupper(dir) == 'E')
    {
        for (int i = c; i != rowLength; i++)
        {
            if(isValidPosition(r, i + 1))
            {
                maxDistance ++;
                
                if (maxDistance == maxSteps)
                {
                    return maxSteps;
                }
            }
            if (!isValidPosition(r,i + 1))
            {
                return maxDistance;
            }
        }
        return maxDistance;
    }
    
    if (toupper(dir) == 'W')
    {
        for (int i = c; i > 0; i--)
        {
            if(isValidPosition(r, i - 1))
            {
                maxDistance ++;
                
                if (maxDistance == maxSteps)
                {
                    return maxSteps;
                }
            }
            if (!isValidPosition(r,i - 1))
            {
                return maxDistance;
            }
        }
        return maxDistance;
    }
    
    cerr << endl;
    return 0;
}

//This function returns true if its parameter is a syntactically valid course, and false otherwise. A syntactically valid course is a sequence of zero or more course segments (not separated or surrounded by spaces, commas, or anything else). This fuction returns true is the string is empty, false if: the first character is not alphabetic, the last character.

bool isCourseWellFormed(string course)
{
    
    if (course == "")
        return true;
    
    if (!(isalpha(course[0])))
        return false;
    
    if ((course.size() == 1) && (!isalpha(course[0])))    // E is okay. But 5 is not okay.
        return false;
    
    
    for (int k = 0; k != course.size(); k++) //Check to ensure only alphabetic or numeric characters
    {
        if (!isalpha(course[k]) && !isdigit(course[k]))
            return false;
        
        if ((isalpha(course[k])))  //Checks for valid directions
            {
                if (!(toupper(course[k]) == 'N') && !(toupper(course[k]) == 'S')
                    && !(toupper(course[k]) == 'E')  && !(toupper(course[k]) == 'W'))
                    return false;
                
            }
        
        if (isdigit(course[k])) //Checks if there are more than two consecutive digits 
            {
                if (!((k + 1) > (course.size() - 1)) && (isdigit(course[k + 1])))
                    {
                        if (!((k + 2) > (course.size() - 1)) && (isdigit(course[k + 2])))
                            return false;
                    }
            }
    }
    
    return true;
}

//This function checks to see if the given position(r,c) is valid. Returns false if the position is outside the boundaries of the grid or if the position is a wall. Returns true otherwise.

bool isValidPosition(int r, int c)
{
    int rowLength = getCols();
    int colLength = getRows();
    
    if (r < 1 ||  r > colLength  ||  c < 1  ||  c > rowLength)
        return false;
    
    if (isWall(r,c))
        return false;
    
    else
        return true;
}


//Simple function for updating position based on direction input

void updatePosition(int& currentRow, int& currentCol, char dir, int distance)
    {
        if (toupper(dir) == 'E')
            currentCol += distance;
        
        if (toupper(dir) == 'W')
            currentCol -= distance;
        
        if(toupper(dir) == 'S')
            currentRow += distance;
        
        if(toupper(dir) == 'N')
            currentRow -= distance;
    }

//Quick function for converting digital characters into their literal numerical values
int stringtoInteger(char a)
{
    int w = a - 48;
    return w;
}

