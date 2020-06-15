#include "Actor.h"
#include "StudentWorld.h"
#include "freeglut.h"
#include "GameWorld.h"
#include "GameController.h"
#include "SpriteManager.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include <vector>

using namespace std;

// ====================================================================================//
// ||- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -||
// || - - - - - - - - - - - IMPLEMENTATION FOR TERRAIN OBJECTS - - - - - - - - - - - ||
// || - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ||
// ====================================================================================//


void Boulder::doSomething() {
    
    if (getState() == dead)
        return;
    
    if (getState() == stable) {
        bool w = m_studentWorld->checkEarth(getX(), getY() - 1);
        bool x = m_studentWorld->checkEarth(getX() + 1, getY() - 1);
        bool y = m_studentWorld->checkEarth(getX() + 2, getY() - 1);
        bool z = m_studentWorld->checkEarth(getX() + 3, getY() - 1);
        
        if (w || x || y || z)
            return;
        else {
            setState(waiting);
            startClock();
        }
    }
    
    if (getState() == waiting) {
        if (m_waitClock == 0) {
            setState(falling);
            m_studentWorld->playSound(SOUND_FALLING_ROCK);
        }
        else
            tick();
    }
    
    if (getState() == falling) {
        
        // If the boulder hits the bottom of the oil field
        if (getY() == 0) {
            setState(dead);
            return;
        }
        // If the boulder runs into an Earth object
        else if(m_studentWorld->checkEarth(this->getX(), this->getY())) {
            setState(dead);
            return;
        }
        // If the boulder runs into another Rock object
        else if (m_studentWorld->blocked(this->getX(), this->getY())) {
            setState(dead);
            return;
        }
        else {
            // Kill any living object on the way down
            m_studentWorld->crushedByRock(this->getX(), this->getY());
            // Move down by one
            moveTo(getX(), getY() - 1);
        }
    }
    
    return;
}
        

// ====================================================================================//
// || - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ||
// || - - - - - - - - - - - DERIVED CLASSES FOR INVENTORY OBJECTS - - - - - - - - - - ||
// || - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ||
// ====================================================================================//



// - - - - - - - - - - - - - - - WATERSQUIRT - - - - - - - - - - - - - - - - - - //


void WaterSquirt::doSomething() {
    
    if (m_studentWorld->hitBySquirt(this->getX(), this->getX()))
        setState(dead);
        
    if (m_travelDistance == 0) {
        setState(dead);
        return;
    }
    
    // Check
    switch (getDirection()) {
        case right:
            if (getX() == 60)
                setState(dead);
            else if (m_studentWorld->checkEarth(getX() + 1, getY()))
                setState(dead);
            else if (m_studentWorld->blocked(getX(), getY()))
                setState(dead);
            else {
                moveTo(getX() + 1, getY());
                activate();
            }
            break;
        case left:
            if (getX() == 0)
                setState(dead);
            else if (m_studentWorld->checkEarth(getX() - 1, getY()))
                setState(dead);
            else if (m_studentWorld->blocked(getX(), getY()))
                setState(dead);
            else {
                moveTo(getX() - 1, getY());
                activate();
            }
            break;
        case up:
            if (getY() == 60)
                setState(dead);
            else if (m_studentWorld->checkEarth(getX(), getY() + 1))
                setState(dead);
            else if (m_studentWorld->blocked(getX(), getY()))
                setState(dead);
            else {
                moveTo(getX(), getY() + 1);
                activate();
            }
            break;
        case down:
            if (getY() == 0)
                setState(dead);
            else if (m_studentWorld->checkEarth(getX(), getY() - 1))
                setState(dead);
            else if (m_studentWorld->blocked(getX(), getY()))
                setState(dead);
            else {
                moveTo(getX(), getY() - 1);
                activate();
            }
            break;
        default:
            break;
    }
    return;
}


// - - - - - - - - - - - - - - - BARREL - - - - - - - - - - - - - - - - - - //

void Barrel::doSomething() {
    
    if (getState() == dead)
        return;
    
    // If the Barrel is not currently visible AND
    // the TunnelMan is within a radius of 4.0 of it,
    // set visbile to true and immediately return
    if (isVisible() == false)
        if (m_StudentWorld->getDistance(this->getX(), this->getY(),
                                        m_StudentWorld->getPlayerXCoord(), m_StudentWorld->getPlayerYCoord()) <= 4.0) {
            setVisible(true);
            return;
        }
    
    if (m_StudentWorld->getDistance(this->getX(), this->getY(),
                                    m_StudentWorld->getPlayerXCoord(), m_StudentWorld->getPlayerYCoord()) <= 3.0) {
        activate();
    }
}

void Barrel::activate() {
    // Set state to dead so it can be removed
    setState(dead);
    // Play sound indicating player found gold
    m_StudentWorld->playSound(SOUND_FOUND_OIL);
    // Increase player score by 1000
    m_StudentWorld->increaseScore(1000);
    // Decrease the number barrels the StudentWorld thinks it has
    m_StudentWorld->decBarrelCount();
}

// - - - - - - - - - - - - - - - GOLDEN NUGGET - - - - - - - - - - - - - - - - - - //


void GoldenNugget::doSomething() {
    
    if (getState() == temporary) {
        
    }
    if (getState() == dead)
        return;
    else if (!isBribeOffer() && m_studentWorld->getDistance(getX(), getY(), m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()) <= 3.0) {
        activate();
        
    }
    else if (!isBribeOffer() && m_studentWorld->getDistance(getX(), getY(), m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()) <= 4.0) {
        
        setVisible(true);
        return;
    }
    else if (isBribeOffer()) {
        m_waitClock--;
        if (!m_waitClock)
            setState(dead); 
    }
    
    // Complete Implementation: Picked up by protestor 
 
}


void GoldenNugget::activate() {
    
    setState(dead);
    m_studentWorld->playSound(SOUND_GOT_GOODIE);
    m_studentWorld->increaseScore(10);
    m_studentWorld->incNuggets();
    
}


// - - - - - - - - - - - - - - - - - - Sonar Kit - - - - - - - - - - - - - - - - - - - //


void SonarKit::doSomething() {
    
    if(m_Clock == 0)
        setState(dead);
    
    if (getState() == dead)
        return;
    else if (m_studentWorld->getDistance(getX(), getY(),
                                         m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()) <= 3.0) {
        activate();
    }
    else if (getState() == temporary)
        tick();
}



void SonarKit::activate() {
    
    setState(dead);
    m_studentWorld->playSound(SOUND_GOT_GOODIE);
    m_studentWorld->increaseScore(75);
    m_studentWorld->incKits(); 
    
}

// - - - - - - - - - - - - - - - - - Water Pool - - - - - - - - - - - - - - - - - - - //


void WaterRefill::doSomething() {
    
    if (m_Clock == 0)
        setState(dead);
    
    if (getState() == dead)
        return;
    if (getState() == dead)
        return;
    else if (m_studentWorld->getDistance(getX(), getY(),
                                         m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()) <= 3.0) {
        activate();
    }
    else if (getState() == temporary)
        tick();
}

void WaterRefill::activate() {
    
    m_studentWorld->playSound(SOUND_GOT_GOODIE);
    m_studentWorld->incSquirts();
    m_studentWorld->increaseScore(100);
    setState(dead);
    
}


// ====================================================================================//
// || - - - - - - - - - - - -IMPLEMENTATION FOR ANIMATE OBJECTS - - - - - - - - - - - ||
// ====================================================================================//

void TunnelMan::doSomething() {
    
    if (isDead())   // Checks to see if TunnelMan is dead
        return;
    
    // Remove/destroy the Earth objects from the 4x4 area occupied by
    // calling a public method in StudentWorld Class
    m_StudentWorld->digEarth(this->getX(), this->getY());
    
    
    int ch;
    if (m_StudentWorld->getKey(ch) == true)
    {
        // user hit a key this tick!
        switch (ch)
        {
            // If the user presses the Escape key, this
            // allows the user to abort the current level.
            case KEY_PRESS_ESCAPE:
                this->setDead();
                break;
            case KEY_PRESS_SPACE:
                if (m_nWaterSquirts == 0)
                    return;
                else if (getDirection() == right) {
                    WaterSquirt* tempPtr = new WaterSquirt(getX() + 4, getY(), getDirection(), m_StudentWorld);
                    m_nWaterSquirts--;
                    m_StudentWorld->getSquirt(tempPtr);
                }
                else if (getDirection() == left) {
                    WaterSquirt* tempPtr = new WaterSquirt(getX() - 4, getY(), getDirection(), m_StudentWorld);
                    m_nWaterSquirts--;
                    m_StudentWorld->getSquirt(tempPtr);
                }
                else if (getDirection() == up) {
                    WaterSquirt* tempPtr = new WaterSquirt(getX(), getY() + 4, getDirection(), m_StudentWorld);
                    m_nWaterSquirts--;
                    m_StudentWorld->getSquirt(tempPtr);
                }
                else if (getDirection() == down) {
                    WaterSquirt* tempPtr = new WaterSquirt(getX(), getY() - 4, getDirection(), m_StudentWorld);
                    m_nWaterSquirts--;
                    m_StudentWorld->getSquirt(tempPtr);
                }
                m_StudentWorld->playSound(SOUND_PLAYER_SQUIRT);
                break;
            case KEY_PRESS_LEFT:
                // Check if player isfacing left
                if (getDirection() != left)
                    setDirection(left);
                // Check if player is at left boundary
                else if (getX() == 0)
                    return;
                // Check if player is blocked by boulder
                else if (m_StudentWorld->blocked(getX() - 1, getY()))
                    return;
                else
                    moveTo(getX() - 1, getY());
                break;
            case KEY_PRESS_RIGHT:
                // Check if player is facing right
                if (getDirection() != right)
                    setDirection(right);
                // Check if player is at right boundary
                else if (getX() == 60)
                    return;
                // Check if player is blocked by boulder
                else if (m_StudentWorld->blocked(getX() + 1, getY()))
                    return;
                else
                    moveTo(getX() + 1, getY());
                break;
            case KEY_PRESS_UP:
                // Check if player is facing up
                if (getDirection() != up)
                    setDirection(up);
                // Check if player is at upper boundary
                else if (getY() == 60)
                    return;
                // Check if player is blocked by boulder
                else if (m_StudentWorld->blocked(getX(), getY() + 1))
                    return;
                else
                    moveTo(getX(), getY() + 1);
                break;
            case KEY_PRESS_DOWN:
                // Check if player is facing down
                if (getDirection() != down)
                    setDirection(down);
                // Check if player is at lower boundary
                else if (getY() == 0)
                    return;
                // Check if player is blocked by boulder
                else if (m_StudentWorld->blocked(getX(), getY() - 1))
                    return;
                else
                    moveTo(getX(), getY() - 1);
                break;
            case KEY_PRESS_TAB:
                if (m_nGoldenNuggets > 0) {
                    m_nGoldenNuggets--;
                    GoldenNugget* tempNugget = new GoldenNugget(getX(), getY(), m_StudentWorld, "Player");
                    m_StudentWorld->getGold(tempNugget);
                }
                    break;
            case 'Z':
            case 'z':
                if (m_nSonarKits > 0) {
                    m_nSonarKits--;
                    m_StudentWorld->unveil();
                    m_StudentWorld->playSound(SOUND_SONAR); 
                }
                break;
            case INVALID_KEY:
                break;
            default:
                break; // Do nothing
            
        }
    } else {} // Do Nothing

    return;
    
}


// - - - - - - - - - - - - - - - - - Regular Protestor - - - - - - - - - - - - - - - - - - - //

void RegularProtestor::tick() {
    if (m_shoutClock)
        m_shoutClock--;
}


void RegularProtestor::shout() {
    
    m_studentWorld->playSound(SOUND_PROTESTER_YELL);
    m_studentWorld->annoyPlayer();
    resetClock();
}

void RegularProtestor::annoy(int damage) {
    // A Regular Protester can’t be further annoyed once it is in the exiiting state
    if (getState() == exiting)
        return;
    
    AnimateObject::annoy(damage);
    if (getState() == dead) {
        // The protestor is 'dead' but still needs to exit,
        setState(exiting);
        // Ask world to create exit path
        m_studentWorld->create_path(getX(), getY(), m_exitPath);
    }
}

void RegularProtestor::followExitPath(char dir) {
    
    switch(dir) {
        case 'W':
            // Check if player isfacing left
            if (getDirection() != left)
                setDirection(left);
            else
                moveTo(getX() - 1, getY());
            break;
        case 'E':
            // Check if player is facing right
            if (getDirection() != right)
                setDirection(right);
            else
                moveTo(getX() + 1, getY());
            break;
        case 'N':
            // Check if player is facing up
            if (getDirection() != up)
                setDirection(up);
            else
                moveTo(getX(), getY() + 1);
            break;
        case 'S':
            // Check if player is facing down
            if (getDirection() != down)
                setDirection(down);
            else
                moveTo(getX(), getY() - 1);
            break;
        default:
            return;
    }
    return;
}

void RegularProtestor::attemptShout() {
    if (m_shoutClock)
        return;
    else {
        if ( (getDirection() == right) && (getX() < m_studentWorld->getPlayerXCoord()) ) {
            shout();
            m_shoutClock += 15;
            return;
        }
        else if ( (getDirection() == left) && (getX() > m_studentWorld->getPlayerXCoord()) ) {
            shout();
            m_shoutClock += 15;
            return;
        }
        else if ( (getDirection() == up) && (getY() < m_studentWorld->getPlayerYCoord()) ) {
            shout();
            m_shoutClock += 15;
            return;
        }
        else if ( (getDirection() == down) && (getY() > m_studentWorld->getPlayerYCoord()) ) {
            shout();
            m_shoutClock += 15; 
            return;
        }
    }
}

bool RegularProtestor::sawPlayer() {
    
    if (m_studentWorld->clearPath(getX(), getY(), m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()))
        return true;
    else
        return false;
}


void RegularProtestor::moveTowardPlayer() {
        if (getX() < m_studentWorld->getPlayerXCoord()) {
            setDirection(right);
            moveTo(getX() + 1, getY());
        }
        else if (getX() > m_studentWorld->getPlayerXCoord()) {
            setDirection(left);
            moveTo(getX() - 1, getY());
        }
        else if (getY() < m_studentWorld->getPlayerYCoord()) {
            setDirection(up);
            moveTo(getX(), getY() + 1);
        }
        else if (getY() > m_studentWorld->getPlayerYCoord()) {
            setDirection(down);
            moveTo(getX(), getY() - 1);
        }
    }

void RegularProtestor::pickNewDirection() {
    
        bool badDir = true;
        
        while (badDir) {
            badDir = false;
            int randDir = rand() % 3;
            switch (randDir) {
                case 0:
                    setDirection(right);
                    if (!m_studentWorld->clearPath(getX(), getY(), getX() + 1, getY()))
                        badDir = true;
                    break;
                case 1:
                    setDirection(left);
                    if (!m_studentWorld->clearPath(getX(), getY(), getX() - 1, getY()))
                        badDir = true;
                    break;
                case 2:
                    setDirection(up);
                    if (!m_studentWorld->clearPath(getX(), getY(), getX(), getY() + 1))
                        badDir = true;
                    break;
                case 3:
                    setDirection(down);
                    if (!m_studentWorld->clearPath(getX(), getY(), getX(), getY() - 1))
                        badDir = true;
                    break;
                default:
                    break;
            }
        }
}


bool RegularProtestor::moveOnce() {
    
    Direction dir = getDirection();
    switch(dir) {
        case left:
            if (getX() == 0)
                return false;
            else {
                moveTo(getX() - 1, getY());
                return true;
            }
            break;
        case right:
            if (getX() == 60)
                return false;
            else {
                moveTo(getX() + 1, getY());
                return true;
                }
            break;
        case up:
            if (getY() == 60)
                return  false;
            else {
                moveTo(getX(), getY() + 1);
                return true;
            }
            break;
        case down:
            if (getY() == 0)
                return false;
            else {
                moveTo(getX(), getY() - 1);
                return true;
            }
            break;
        default:
            return false;
    }
}

void RegularProtestor::checkForTurn() {
    
        if (getDirection() == right || getDirection() == left) {
            if (m_studentWorld->clearPath(getX(), getY(), getX(), getY() + 1)) {
                setDirection(up);
                moveOnce();
                return;
            }
            if (m_studentWorld->clearPath(getX(), getY(), getX(), getY() - 1)) {
                setDirection(down);
                moveOnce();
                return;
            }
        }
        // Direction is up or down
        else {
            if (m_studentWorld->clearPath(getX(), getY(), getX() + 1, getY())) {
                setDirection(right);
                moveOnce();
                return;
            }
            if (m_studentWorld->clearPath(getX(), getY(), getX() - 1, getY())) {
                setDirection(left);
                moveOnce();
                return;
            }
        }
}



void RegularProtestor::doSomething() {
    
    tick(); 
    
    if (ticksToWaitBetweenMoves <= 0)
        setState(searching);
    
    if (getState() == dead)
        return;
    if (getState() == resting) {
        rest();
        return;
    }
    // If the Regular Protester is in a leave-the-oil-field state
    else if (getState() == exiting) {
        if (getX() == 60 && getY() == 60)
            setState(dead);
        // The Hardcore Protester moves one square closer to its exit
        else {
            tick();
            // Get the first (next) character in the path string
            char dir = m_exitPath.front();
            // Delate that character; if-statement is a check
            if (dir)
                m_exitPath.erase(0);
            // Take step in appropriate direction
            followExitPath(dir);
            return;
        }
    }
    
    // Protestor is NOT exiting, reesting, or dead //
    
    else if (m_studentWorld->getDistance(getX(), getY(), m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()) <= 4.0) {
        attemptShout();
    }
    else if (m_studentWorld->getDistance(getX(), getY(), m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()) > 4.0) {
        if (sawPlayer()) {
            moveTowardPlayer();
            return;
        }
        else {
            numSquaresToMoveInCurrentDirection--;
            if (numSquaresToMoveInCurrentDirection <= 0) {
                pickNewDirection();
                numSquaresToMoveInCurrentDirection = (rand() % 52) + 8;;
                moveOnce();
                return;
            }
            else
                checkForTurn();
        }
    }
    if (moveOnce())
        return;
    else {
        pickNewDirection();
        return;
    }
}













// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

