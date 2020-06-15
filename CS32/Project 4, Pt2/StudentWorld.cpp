


#include "freeglut.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include "GameController.h"
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "Actor.h"
#include "StudentWorld.h"
using namespace std;


class Actor;
class TunnelMan;
class Earth;
class Boulder;
class SonarKit;
class Barrel;
class WaterSquirt;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}


StudentWorld::~StudentWorld()
{
    // Delete TunnelMan
    delete m_TunnelMan;
    
    // Delete the actors using an iterator
    std::vector<Actor* >::iterator actorItr;
    for (actorItr = m_actors.begin(); actorItr != m_actors.end(); actorItr++) {
        if (*actorItr) {
            std::vector<Actor* >::iterator tempItr = actorItr;
            actorItr--;
            delete *tempItr;            // First, delete the dynamically-allocated object
            m_actors.erase(tempItr);    // Then, erase the pointer itself in the vector
        }
    }
    
    // Delete any Earth objects in the Earth pointer field
    for (int k = 0; k < 60; k++) {
        for (int j = 0; j < 60 ; j++) {
            if (ground[k][j]) {
                delete ground[k][j];
                ground[k][j] = nullptr;
            }
        }
    }
    
    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - INIT - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

int StudentWorld::init()
{
    
    if (getLives() <= 0)
        return GWSTATUS_PLAYER_DIED; 
    
    // Create new TunnelMan
    m_TunnelMan = new TunnelMan(this);
    
    // Fill the entire oil field with Earth objects
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 60; j++) {
            Earth* tempEarthPtr = new Earth(i, j);
            ground[i][j] = tempEarthPtr;
            if (i == 59) {
                Earth* tempEarthPtr0 = new Earth(i, j);
                tempEarthPtr0->moveTo(i + 1, j);
                ground[i + 1][j] = tempEarthPtr0;
                Earth* tempEarthPtr1 = new Earth(i, j);
                tempEarthPtr1->moveTo(i + 2, j);
                ground[i + 2][j] = tempEarthPtr1;
                Earth* tempEarthPtr2 = new Earth(i, j);
                tempEarthPtr2->moveTo(i + 3, j);
                ground[i + 3][j] = tempEarthPtr2;
            }
        }
    
    // Create central tunnel
    for (int i = 30; i < 34; i++) {
        for (int j = 4; j < 60; j++) {
            delete ground[i][j];
            ground[i][j] = nullptr;
        }
    }
    /*
     No distributed game object may be within a radius (Euclidian distance) of 6 squares of
     any other distributed game object. For example, if a Boulder were distributed to x=1,y=2,
     then a Nugget could not be distributed to x=6,y=4 because the two would be 5.39 squares
     away (less than or equal to 6 squares away). However the same Nugget could be
     distributed to x=6,y=6 because this would be 6.4 squares away (more than 6.0 squares
     away).
     
     Nuggets and Oil Barrels must be distributed between x=0,y=0 and x=60,y=56
     inclusive, meaning that the lower-left corner of any such object must fall within this
     rectangle.
     
     Boulders must be distributed between x=0,y=20 and x=60,y=56, inclusive (so
     they have room to fall).
     
     */
    
    
    // B Boulders in each level, where:
    int B = fmin(getLevel() / 2 + 2, 9);
    
    // G Gold Nuggets in each level, where:
    int G = fmax((5 - getLevel()) / 2, 2);
    
    // L Barrels of oil in each level, where:
    int L = fmin(2 + getLevel(), 21);
    
    // Private count of number of barrels remaining 
    m_barrelsLeft = L;
    
    int NtotalActors = B + G + L;
    // Use current time as seed for random generator
    srand(time(0));
    
    
    for (int i = 0; i < NtotalActors; i++) {
        int xCoord = 0;
        int yCoord = 0;
        bool validPos = true; // Set this to true so we can enter the while loop
        while (validPos) {
            // Start by assuming none of the actors are too close
            validPos = false;
            // Generate random values
            if (i < (NtotalActors - B)) {
                xCoord = rand() % 60;
                yCoord = (rand() % 36) + 20;
            }
            else {
                xCoord = rand() % 60;
                yCoord = rand() % 56;
            }
            // Make sure the values are not too close to other actors
            std::vector<Actor* >::iterator actorItr;
             for (actorItr = m_actors.begin(); actorItr != m_actors.end(); actorItr++)
                 if (getDistance(xCoord, yCoord, (*actorItr)->getX(), (*actorItr)->getY()) <= 6.0)
                     validPos = true;
            
            // Make sure the values are not in the central tunnel
            if (xCoord >= 26 && xCoord < 34)
                validPos = true;
     }
        // Create new boulder
        if (i < (NtotalActors - (G + L))) {
            m_actors.push_back(new Boulder(xCoord, yCoord, this));
            // Delete any Earth objects
            digEarth(xCoord, yCoord);
        }
        else if (i >= (NtotalActors - (G + L)) && i < (NtotalActors - G))
            m_actors.push_back(new Barrel(xCoord, yCoord, this));
        else if (i >= (NtotalActors - G) && i < NtotalActors)
            m_actors.push_back(new GoldenNugget(xCoord, yCoord, this, "World"));
    }
    
    
    // create new Regular Protestor
    m_actors.push_back(new RegularProtestor(this));
    
    // Start the level 
    return GWSTATUS_CONTINUE_GAME;
}





    
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - MOVE - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

int StudentWorld::move()
{
    
    // If the actor causes the player to die, the player loses
    if (m_TunnelMan->getState() == dead) {
        playSound(SOUND_PLAYER_GIVE_UP);
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }
    
    // Let the player make a move
    m_TunnelMan->doSomething();
    // If the player collects all the barrels, they win
    if (m_barrelsLeft == 0) {
        playSound(SOUND_FINISHED_LEVEL);
        return GWSTATUS_FINISHED_LEVEL;
    }
    
    // Ask each actor to do something
    std::vector<Actor* >::iterator ActorItr;
    
    for (ActorItr = m_actors.begin(); ActorItr != m_actors.end(); ActorItr++) {
        if ((*ActorItr)->getState() != dead)
            (*ActorItr)->doSomething();
    }
    
    // Get rid of any dead actors
    for (ActorItr = m_actors.begin(); ActorItr != m_actors.end(); ActorItr++) {
        
        if ((*ActorItr)->getState() == dead) {
            std::vector<Actor* >::iterator tempItr = ActorItr;
            ActorItr--;
            delete *tempItr;        // First, delete the dynamically-allocated object
            m_actors.erase(tempItr); // Then, delete the pointer itself in the list
        }
    }
    
    // Add new goodies
    addGoodies();
    
    
    // To update text on top of screen
    setDisplayText(); 
    
    
    // The player is still alive but hasn't collected all barrels: continue game
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::addGoodies() {
    srand(time(0));
    // Add new goodies
    int goodieProbA = rand() % (getLevel() * 10 + 300);
    // Add Sonarkit
    if (goodieProbA == 1) {
        int goodieProbB = rand() % 4;
        if (goodieProbB == 1)
            m_actors.push_back(new SonarKit(this));
        // Calculate position and add water refill
        else {
            int xCoord = 0;
            int yCoord = 0;
            bool hasEarth = true;
            int addWindow = 5;
            while(hasEarth) {
                // addwWindow is necessary to prevent addGoodies() from timing out
                addWindow--;
                if (!addWindow)
                    return;
                hasEarth = false;
                xCoord = rand() % 59;
                yCoord = rand() % 56;
                for (int i = xCoord; i < xCoord + 5; i++)
                    for (int k = yCoord; k < yCoord + 5; k++)
                        if (checkEarth(i, k))
                            hasEarth = true;
            }
            m_actors.push_back(new WaterRefill(xCoord, yCoord, this));
        }
    }
    
    return;
}









// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - CLEANUP - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void StudentWorld::cleanUp()
{
    // Delete TunnelMan
    delete m_TunnelMan;
    
    // Delete the actors using an iterator
    std::vector<Actor* >::iterator actorItr;
    for (actorItr = m_actors.begin(); actorItr != m_actors.end(); actorItr++) {
        std::vector<Actor* >::iterator tempItr = actorItr;
        actorItr--;
        delete *tempItr;            // First, delete the dynamically-allocated object
        m_actors.erase(tempItr);    // Then, erase the pointer itself in the vector
    }
    
    // Delete any Earth objects in the Earth pointer field
    for (int k = 0; k < 63; k++) {
        for (int j = 0; j < 60 ; j++) {
            if (ground[k][j]) {
                delete ground[k][j];
            }
        }
    }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - AUXILLARY FUNCTIONS - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //


void StudentWorld::unveil() {
    
    std::vector<Actor* >::iterator actorItr;
    for (actorItr = m_actors.begin(); actorItr != m_actors.end(); actorItr++)
        if (getDistance(getPlayerXCoord(), getPlayerYCoord(), (*actorItr)->getX(), (*actorItr)->getY()) <= 12.0)
            // Make them vsible
            (*actorItr)->setVisible(true);
}








void StudentWorld::setDisplayText()
{
    int level = getLevel();
    int lives = getLives();
    int health = m_TunnelMan->getHitPoints();
    int squirts = m_TunnelMan->getNSquirt();
    int gold = m_TunnelMan->getNnugget();
    int barrelsLeft = m_barrelsLeft; 
    int sonar = m_TunnelMan->getNKit();
    int score = getScore(); 
    // Next, create a string from your statistics, of the form:
    // Lvl: 52 Lives: 3 Hlth: 80% Wtr: 20 Gld: 3 Oil Left: 2 Sonar: 1 Scr: 321000
    string formatedText = formatDisplay(level, lives, health, squirts, gold, barrelsLeft, sonar, score);
    // Finally, update the display text at the top of the screen with your
    // newly created stats
    setGameStatText(formatedText); // calls our provided GameWorld::setGameStatText
}


string StudentWorld::formatDisplay(int level, int lives, int health, int squirts,
                                  int gold, int barrelsLeft, int sonar, int score)
{
    
    int healthPct = 100;
    if (health < 10)
        healthPct = (health * 10) % 100;
    
    
    
    ostringstream oss;
    
    oss << setw(2) << "Lvl: " << level;
    oss << setw(8) << "Lives: " << lives;
    oss << setw(7) << "Hlth: " << healthPct << "%";
    oss << setw(6) << "Wtr: " << squirts;
    oss << setw(6) << "Gld: " << gold;
    oss << setw(11) << "Oil Left: " << barrelsLeft;
    oss << setw(8) << "Sonar: " << sonar;
    oss << setw(6) << "Scr: ";
    
    oss.fill('0');
    
    oss << setw(8) << score;
    
    string formatedText = oss.str();
    return formatedText;
    
}



void StudentWorld::digEarth(const int& xCoord, const int& yCoord)
{
    // xCoord and xCoord are passed-by-reference
    // in TunnelMan's doSomething() method
    
    int waitCount = 0;
    
    // If the tunnelman is at the top of the oil field, he cannot dig, so return
    if (yCoord == 60)
        return;
    
    // If the tunnelman is at the upper right-hand corner of the oil field, he cannot dig, so return
    if (xCoord == 60 && yCoord == 60)
        return; 
    
    // Only check the section of the Earth-pointer field that
    // overlaps the area of the TunnelMan 
    for (int k = xCoord; (k < 63) && (k < xCoord + 4); k++) {
        for (int j = yCoord; (j < 60) && (j < yCoord + 4); j++) {
            if (ground[k][j]) {

                delete ground[k][j];
                ground[k][j] = nullptr;
                waitCount--;
            }
        }
    }
    if (waitCount < 0)
        playSound(SOUND_DIG);
}


bool StudentWorld::blocked(const int& xCoord, const int& yCoord) {

    std::vector<Actor* >::iterator actorItr;
    
    for(actorItr = m_actors.begin(); actorItr < m_actors.end(); actorItr++) {
        if ((*actorItr)->getID() == TID_BOULDER) {
            // Make sure the rock does consider itself
            if (xCoord == (*actorItr)->getX() && yCoord == (*actorItr)->getY());
            // Adding 2 gets us the center of the object (instead of its lower left corner)
            else if (getDistance(xCoord + 2, yCoord + 2, (*actorItr)->getX() + 2, (*actorItr)->getY() + 2) <= 2.0)
                return true;
        }
    }
    return false; 
}


void StudentWorld::crushedByRock(const int& xCoord, const int& yCoord) {
    
    // Check to see if the rock crushed the player
    if (getDistance(m_TunnelMan->getX() + 2, m_TunnelMan->getY() + 2, xCoord + 2, yCoord + 2) <= 3.0)
        m_TunnelMan->setState(dead); 
    
    std::vector<Actor* >::iterator tempActorItr;
    
    for (tempActorItr = m_actors.begin(); tempActorItr != m_actors.end(); tempActorItr++) {
        if ((*tempActorItr)->isAnimate()) {
            if ((*tempActorItr)->getX() == xCoord && (*tempActorItr)->getY() == yCoord); // Do nothing
            else if (getDistance(xCoord + 2, yCoord + 2, (*tempActorItr)->getX() + 2, (*tempActorItr)->getY() + 2) <= 3.0)
                (*tempActorItr)->annoy(100);
        }
    }
}
    
    


double StudentWorld::getDistance(int x1, int y1, int x2, int y2) {
    
    double xDistance = (x1 - x2);
    double yDistance = (y1 - y2);
    double Distance = sqrt( (xDistance * xDistance) + (yDistance * yDistance) );
    return Distance;
}

bool StudentWorld::checkEarth(const int& xCoord, const int& yCoord) {
    
    if (ground[xCoord][yCoord])
        return true;
    else
        return false;
}


bool StudentWorld::hitBySquirt(const int& xCoord, const int& yCoord) {
    
    int tempX = xCoord;
    int tempY = yCoord;
    
    bool contactFlag = false;
    
    std::vector<Actor* >::iterator tempActorItr;
    
    for (tempActorItr = m_actors.begin(); tempActorItr != m_actors.end(); tempActorItr++) {
        if ((*tempActorItr)->isAnimate() && (*tempActorItr)->getID() != TID_PLAYER)
            if (getDistance(tempX, tempY, (*tempActorItr)->getX(), (*tempActorItr)->getY()) <= 3.0) {
                (*tempActorItr)->annoy(2);
                contactFlag = true;
            }
    }
    return contactFlag;
}


int StudentWorld::getPlayerXCoord() { return m_TunnelMan->getX(); }

int StudentWorld::getPlayerYCoord() { return m_TunnelMan->getY(); }

// Handles transfer of waterspuirt to studentworld
void StudentWorld::getSquirt(WaterSquirt* squirt) {
    m_actors.push_back(squirt);
}

void StudentWorld::getGold(GoldenNugget* nugget) {
    m_actors.push_back(nugget);
}


void StudentWorld::annoyPlayer() {
    m_TunnelMan->annoy(2);
    
    if (m_TunnelMan->getHitPoints() <= 0)
        m_TunnelMan->setState(dead);
}


void StudentWorld::incSquirts() { m_TunnelMan->getNSquirt() += 5; }
void StudentWorld::incKits() { m_TunnelMan->getNKit()++; }
void StudentWorld::incNuggets() { m_TunnelMan->getNnugget()++; }
void StudentWorld::decSquirts() { m_TunnelMan->getNSquirt()--; }
void StudentWorld::dectKits() { m_TunnelMan->getNKit()--; }
void StudentWorld::dectNugeets() { m_TunnelMan->getNnugget()--; }

bool StudentWorld::clearPath(int beginx, int beginy, int endx, int endy) {
    
    bool clear = true;
    
    // Protestor and Player are in same column
    if (beginx == endx) {
        if (beginy < endy) {
            for (int j = beginy; j < endy; j++) {
                for (int i = beginx; i < (beginx + 3) && i < 63; i++) {
                    if (checkEarth(i, j) || blocked(i, j))
                        return false;
                }
            }
        }
        else if (beginy > endy)
            for (int j = beginy; j > endy; j--) {
                for (int i = beginx; i < (beginx + 3) && i < 63; i++) {
                    if (checkEarth(i, j) || blocked(i, j))
                        return false;
                }
            }
    }
    
    // Protestor and Player are in same row
    else if (beginy == endy) {
        // Protestor is to the left of player
        if (beginx < endx) {
            for (int i = beginx; i < endx; i++) {
                for (int j = beginy; j < (beginy + 3) && j < 60; j++) {
                    if (checkEarth(i, j) || blocked(i, j))
                        return false;
                }
            }
        }
        // Protestor is the to right of player
        else if (beginx > endx) {
            for (int i = beginx; i > endx; i--) {
                for (int j = beginy; j < (beginy + 3) && j < 60; j++) {
                    if (checkEarth(i, j) || blocked(i, j))
                        return false;
                }
            }
        }
    }
                
    
    return clear;

}




void StudentWorld::create_path(int xCoord, int yCoord, std::string& path) {
    
    
    bool found = false;
    // Get an udpate layout of the map
    update_protestorMaze();
    // Call find path
    find_path(m_protestorMaze, xCoord, yCoord, path, found);
}




void StudentWorld::find_path(char maze[63][60], int xCoord ,int yCoord, std::string& path, bool &found, int bx, int by) {
    
    int direction[4][2] = { {-1,0}, {0,-1}, {1,0}, {0,1} };
    string dir_str[4] = {"N","W","S","E"};
    
    {
        if (found) return;
        for(int i=0;i<4;i++) {
            int next_x = xCoord + direction[i][0];
            int next_y = yCoord + direction[i][1];
            if( next_x >= 0 && next_y >= 0 && next_x < bx && next_y < by) {
                if (maze[ next_x ][ next_y ] == '.' ) {
                    maze[ next_x ][ next_y ] = 'V';  // V for visted, equivalent to dropping a crumb
                    find_path(maze,next_x, next_y,
                              path += dir_str[i] , found );
                    if ( ! found)
                        maze[ next_x ][ next_y ] = '.';
                }
                else if (maze[ next_x ][ next_y ] ==  'Q') {
                    path += dir_str[i] + " Found Exit!\n";
                    found = true;
                    for(int j=0;j<path.size();j++)
                        cout << path[j];
                }
            }
        }
    } 
}

// Updates the maze data structure used to move protestors to exit point
void StudentWorld::update_protestorMaze() {
    
    for (int i = 0; i < 63; i++) {
        for (int j = 0; j < 60; j++) {
            if (checkEarth(i, j) || blocked(i, j))
                m_protestorMaze[i][j] = 'X';
            else
                // A '.' indicates that the path is clear
                m_protestorMaze[i][j] = '.';
        }
    }
    // Set exit point
    m_protestorMaze[60][59] = 'Q';
}
