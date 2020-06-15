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
    
    if (m_travelDistance == 0)
        setState(dead);
    
    
    // Check
    switch (getDirection()) {
        case right:
            if (getX() == 60)
                setState(dead);
            else if (m_studentWorld->checkEarth(getX() + 1, getY()))
                setState(dead);
            else if (m_studentWorld->blocked(getX() + 1, getY()))
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
            else if (m_studentWorld->blocked(getX() - 1, getY()))
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
            else if (m_studentWorld->blocked(getX(), getY() + 1))
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
            else if (m_studentWorld->blocked(getX(), getY() - 1))
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
    
    if (getState() == dead)
        return;
    else if (!isVisible() == false && m_studentWorld->getDistance(getX(), getY(), m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()) <= 4.0) {
        
        setVisible(true);
        return;
    }
    else if (!isBribeOffer() && m_studentWorld->getDistance(getX(), getY(), m_studentWorld->getPlayerXCoord(), m_studentWorld->getPlayerYCoord()) <= 3.0) {
        activate(); 
        
    }
    
    // Complete Implementation: Picked up by protestor 
 
}


void GoldenNugget::activate() {
    
    setState(dead);
    m_studentWorld->playSound(SOUND_GOT_GOODIE);
    m_studentWorld->increaseScore(10);
    // d. The Gold Nugget must tell the TunnelMan object that it just received a new
    // Nugget so it can update its inventory.
    
    
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
    // c. The Sonar Kit must tell the TunnelMan object that it just received a new Sonar
    // Kit so it can update its inventory.
    m_studentWorld->increaseScore(75);
    
}

// - - - - - - - - - - - - - - - - - Water Pool - - - - - - - - - - - - - - - - - - - //








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
                    WaterSquirt* tempPtr = new WaterSquirt(getX() + 4, getY(), getDirection());
                    m_nWaterSquirts--;
                    m_StudentWorld->getSquirt(tempPtr);
                }
                else if (getDirection() == left) {
                    WaterSquirt* tempPtr = new WaterSquirt(getX() - 4, getY(), getDirection());
                    m_nWaterSquirts--;
                    m_StudentWorld->getSquirt(tempPtr);
                }
                else if (getDirection() == up) {
                    WaterSquirt* tempPtr = new WaterSquirt(getX(), getY() + 4, getDirection());
                    m_nWaterSquirts--;
                    m_StudentWorld->getSquirt(tempPtr);
                }
                else if (getDirection() == down) {
                    WaterSquirt* tempPtr = new WaterSquirt(getX(), getY() - 4, getDirection());
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
                else if (!m_StudentWorld->blocked(getX(), getY()))
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
                else if (!m_StudentWorld->blocked(getX(), getY()))
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
                else if (!m_StudentWorld->blocked(getX(), getY()))
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
                else if (!m_StudentWorld->blocked(getX(), getY()))
                    moveTo(getX(), getY() - 1);
                break;
            case KEY_PRESS_TAB:
                if (m_nGoldenNuggets > 0)
                    // Create Golden Nugget
                    break;
            case INVALID_KEY:
                break;
            default:
                break; // Do nothing
            
        }
    } else {} // Do Nothing

    return;
    
    // WaterSquirt(int startX, int startY, Direction dir)
    
    // const int KEY_PRESS_LEFT  = 1000;
    // const int KEY_PRESS_RIGHT = 1001;
    //const int KEY_PRESS_UP      = 1002;
    // const int KEY_PRESS_DOWN  = 1003;
    //const int KEY_PRESS_SPACE = ' ';
    //const int KEY_PRESS_ESCAPE = '\x1b';
    // const int KEY_PRESS_TAB      = '\t';
    
    
    
}




















// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

