


#include "freeglut.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include "GameController.h"
#include <string>
#include <vector>
#include <algorithm>
#include <random>
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
        std::vector<Actor* >::iterator tempItr = actorItr;
        actorItr++;
        delete *tempItr;            // First, delete the dynamically-allocated object
        m_actors.erase(tempItr);    // Then, erase the pointer itself in the vector
    }
    
    // Delete any Earth objects in the Earth pointer field
    for (int k = 0; k < 60; k++) {
        for (int j = 0; j < 60 ; j++) {
            if (ground[k][j]) {
                delete ground[k][j];
            }
        }
    }
    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - INIT - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

int StudentWorld::init()
{
    
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
    
    
    
    // B Boulders in each level, where:
    int B = fmin(getLevel() / 2 + 2, 9);
    
    // G Gold Nuggets in each level, where:
    int G = fmax(5 - getLevel() / 2, 2);
    
    // L Barrels of oil in each level, where:
    int L = fmin(2 + getLevel(), 21);
    
    // Private count of number of barrels remaining 
    m_nBarrels = L;
    
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(0, 60); // distribution in range [0, 60]
    
    // dis(rng);
    
    /*
     while (B) {
     Boulder* tempBoulderPtr = new Boulder;
     m_actors.push_back(tempBoulderPtr);
     }
     
     while (G) {
     GoldenNugget* tempNuggetPtr = new GoldenNuggets;
     m_actors.push_back(tempNuggetPtr);
     }
     
     while (L) {
     Barrel* tempBarrelPtr = new Barrel;
     m_actors.push_back(tempBarrelPtr);
     }
     */
 
    
    return GWSTATUS_CONTINUE_GAME;
}





    
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - MOVE - - - - - - - - - - - - - - - - - - - - - - - - //
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

int StudentWorld::move()
{
    // To update text on top of screen
    setGameStatText("Test String");
    
    // Let the player make a move
    m_TunnelMan->doSomething();
    // If the player collects all the barrels, they win
    if (m_nBarrels == 0) {
        playSound(SOUND_FINISHED_LEVEL);
        return GWSTATUS_FINISHED_LEVEL;
    }
    
    // Ask each actor to do something
    std::vector<Actor* >::iterator ActorItr;
    for (ActorItr = m_actors.begin(); ActorItr != m_actors.end(); ActorItr++) {
        if ((*ActorItr)->getState() != dead)
            (*ActorItr)->doSomething();
        // If the actor causes the player to die, the player loses
        if (m_TunnelMan->isDead())
            return GWSTATUS_PLAYER_DIED;
    }
    
    for (ActorItr = m_actors.begin(); ActorItr != m_actors.end(); ActorItr++) {
        if ((*ActorItr)->getState() == dead) {
            std::vector<Actor* >::iterator tempItr = ActorItr;
            if (ActorItr == m_actors.begin())
                ActorItr++;
            else
                ActorItr--;
            delete *tempItr;         // First, delete the dynamically-allocated object
            m_actors.erase(tempItr); // Then, delete the pointer itself in the list
        }
    }
    
    
    // The player is still alive but hasn't collected all barrels: continue game
    return GWSTATUS_CONTINUE_GAME;
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
        actorItr++;
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


void StudentWorld::getSquirt(WaterSquirt* squirt) {
    m_squirts.push_back(squirt);
}






bool StudentWorld::blocked(const int& xCoord, const int& yCoord) {
    
    int tempX = xCoord + 2;
    int tempY = yCoord + 2;
    
    std::vector<Actor* >::iterator actorItr;
    
    for(actorItr = m_actors.begin(); actorItr < m_actors.end(); actorItr++) {
        if ((*actorItr)->getID() == TID_BOULDER) {
            // Adding to gets us the center of the object (instead of its lower left corner)
            if (getDistance(tempX, tempY, (*actorItr)->getX() + 2, (*actorItr)->getY() + 2))
                return true;
        }
    }
    return false; 
}


void StudentWorld::crushedByRock(const int& xCoord, const int& yCoord) {
    
    int tempX = xCoord + 2;
    int tempY = yCoord + 2;
    
    std::vector<Actor* >::iterator tempActorItr;
    
    for (tempActorItr = m_actors.begin(); tempActorItr != m_actors.end(); tempActorItr++) {
        if ((*tempActorItr)->isAnimate())
            if (getDistance(tempX, tempY, (*tempActorItr)->getX(), (*tempActorItr)->getY()) <= 3.0)
                (*tempActorItr)->annoy(100); 
        }

}
    
    


double StudentWorld::getDistance(int x1, int y1, int x2, int y2) {
    
    double xDistance = (x1 - x2);
    double yDistance = (y1 - x2);
    double Distance = sqrt( (xDistance * xDistance) + (yDistance * yDistance) );
    return Distance;
}

bool StudentWorld::checkEarth(const int& xCoord, const int& yCoord) {
    
    if (ground[xCoord][yCoord] && ground[xCoord][yCoord]->getState() != dead)
        return true;
    else
        return false;
}


bool StudentWorld::hitBySquirt(const int& xCoord, const int& yCoord) {
    
    int tempX = xCoord + 2;
    int tempY = yCoord + 2;
    
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

