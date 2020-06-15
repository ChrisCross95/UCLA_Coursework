#ifndef ACTOR_H_
#define ACTOR_H_

#include "freeglut.h"
#include "GameWorld.h"
#include "GameController.h"
#include "SpriteManager.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include "StudentWorld.h"
#include <vector>



// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp


// ====================================================================================//
// ||- - - - - - - - - - - - - - - INHERITANCE HIERARCHY - - - - - - - - - - - - - - - ||

/* The following indented list represents the inheritance hierarchy for GraphObject
    the following classes are Abstract Base Classes (ABC): Actor, Inventory, AnimateObject.
    The RegularProtestor class serves as a base class for the HardCoreProtestor class,*/

class GraphObject;
    class Actor;                // <---- Actor is an ABC
        class TerrainObject;
            class Earth;
            class Barrel;
            class Boulder;
            class WaterRefill;
        class Inventory;        // <----- Inventory is an ABC
            class WaterSquirt;
            class GoldenNuggets;
            class SonarKit;
        class AnimateObject;    // <----- AnimateObject is an ABC
            class TunnelMan;
            class RegularProtestor;
                class HardcoreProtestor; // <---- HardCoreProtestor inherits RegularProtestor, which is a regular class

// ====================================================================================//


enum State {dead, alive, stable, waiting, falling, permanent, temporary, exiting, searching, resting };



class Actor: public GraphObject
{
public:
    
    
    Actor(int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0)
        : GraphObject(imageID, startX, startY, dir, size, depth), m_isAnimate(false), m_state(alive)
    {
        setVisible(true); // All actors should initially be visible
        setState(alive);
    }
    
    virtual ~Actor() {}
    
    virtual void doSomething() = 0;
    
    virtual void setAnimate(bool input) {m_isAnimate = input; }
    
    virtual bool isAnimate() { return m_isAnimate; }
    
    virtual State getState() {return m_state; }
    
    virtual void setState(State newState) {m_state = newState; }
    
    virtual void annoy(int damage) {}
    
private:
    
    bool m_isAnimate;
    State m_state;
    
};

// ====================================================================================//
// ||- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -||
// || - - - - - - - - - - - DERIVED CLASSES FOR TERRAIN OBJECTS - - - - - - - - - - - ||
// || - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ||
// ====================================================================================//

class TerrainObject: public Actor
{
public:
    
    TerrainObject(int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0)
        : Actor(imageID, startX, startY, dir, size, depth)
    {}
    
    virtual ~TerrainObject() {}
    
    virtual void activate() {}
    
private:
    
};

class Earth: public TerrainObject
{
public:
    
    Earth(int startX, int startY)
        : TerrainObject(TID_EARTH, startX, startY, right, 0.25, 3)
    {}
    
    virtual ~Earth() {}
    
    virtual void doSomething() {}  // Earth doesn't do anything, so it redefines Actor's pure virtual function as empty
    
private:
    
    
};

class Boulder: public TerrainObject
{
public:
    Boulder(int startX, int startY, StudentWorld* worldPtr)
    : TerrainObject(TID_BOULDER, startX, startY, right, 1.0, 1)
    {
        setState(stable);
        m_studentWorld = worldPtr;
    }
    
    virtual ~Boulder() {}
    
    virtual void doSomething();
    
    virtual void startClock() {m_waitClock = 30; };
    
    virtual void tick() { m_waitClock--; }
    
    
private:
    StudentWorld* m_studentWorld;
    int m_waitClock; 
    
};



class Barrel: public TerrainObject
{
public:
    
    Barrel(int startX, int startY, StudentWorld* worldPtr)
    : TerrainObject(TID_BARREL, startX, startY, right, 1.0, 2)
    {
        setVisible(false);
        m_StudentWorld = worldPtr;
    }
    
    virtual ~Barrel() {};
    
    virtual void doSomething();
    virtual void activate(); 
    
private:
    StudentWorld* m_StudentWorld;
};



class WaterRefill: public TerrainObject
{
public:
    WaterRefill(int startX, int startY, StudentWorld* worldPtr)
    : TerrainObject(TID_WATER_POOL, startX, startY, right, 1.0, 0)
    {
        setState(temporary);
        m_studentWorld = worldPtr;
        m_Clock = fmax(100, 300 - (10 * m_studentWorld->getLevel() ) );
        
    }
    
    
    virtual ~WaterRefill() {}
    virtual void doSomething();
    virtual void activate();
    virtual void tick() { m_Clock--; }
    
    
private:
    
    StudentWorld* m_studentWorld;
    int m_Clock;
    
    
};

// ====================================================================================//
// || - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ||
// || - - - - - - - - - - - DERIVED CLASSES FOR INVENTORY OBJECTS - - - - - - - - - - ||
// || - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ||
// ====================================================================================//




class Inventory: public Actor
{
public:
    Inventory(int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0)
        : Actor(imageID, startX, startY, dir, size, depth)
    {}

    
    virtual ~Inventory() {}
    
    virtual void activate() = 0; 
    
    
private:
    
    bool m_isPickedUp;

};


class WaterSquirt: public Inventory
{
public:
    WaterSquirt(int startX, int startY, Direction dir, StudentWorld* worldPtr)
    : Inventory(TID_WATER_SPURT, startX, startY, dir, 1.0, 1),
        m_travelDistance(4), m_studentWorld(worldPtr)
    {
        // Immediately set state to dead if it is beyond the boundaries of the oilfield
        if (getX() >= 64 || getX() < 0)
            setState(dead);
        if (getY() >= 64 || getY() < 0)
            setState(dead);
    }
    
    virtual ~WaterSquirt() {}
    virtual void doSomething();
    virtual void activate() {m_travelDistance--; }
    
private:
    bool m_fired;
    bool m_hitSomething;
    int m_travelDistance;
    StudentWorld* m_studentWorld; 
    
};


class GoldenNugget: public Inventory
{
public:
    GoldenNugget(int startX, int startY, StudentWorld* worldPtr, std::string DroppedBy = "Player", Direction dir = right, double size = 1.0,
                 unsigned int depth = 2)
    : Inventory(TID_GOLD, startX, startY, dir, size, depth)
    {
        m_studentWorld = worldPtr;
        // If it is dropped by the player, it should be in a temporary, visible state
        if (DroppedBy == "Player") {
            setVisible(true);
            setState(temporary);
            setBribeState();
            m_waitClock = 100;
        }
        //If it is created by the world, it shuold be in a permanent, invisible state
        if (DroppedBy == "World") {
            setVisible(false); // should be false
            setState(permanent);
            m_isBribeOffer = false;
        }
    }
    
    virtual ~GoldenNugget() {}
    
    
    virtual void doSomething();
    virtual void activate(); 
    virtual bool isBribeOffer() { return m_isBribeOffer; }
    virtual void setBribeState() {m_isBribeOffer = true; }
    
private:
    
    StudentWorld* m_studentWorld;
    bool m_isBribeOffer;
    int m_waitClock;
    
};


class SonarKit: public Inventory
{
public:
    SonarKit(StudentWorld* worldPtr, std::string DroppedBy = "Player")
    : Inventory(TID_SONAR, 0, 60, right, 1.0, 2)
    {
        m_studentWorld = worldPtr; 
        setState(temporary);
        m_Clock = fmax(100, (300 - (10 * m_studentWorld->getLevel() ) ) ); 
    }
    
    virtual ~SonarKit() {}
    virtual void doSomething();
    virtual void activate();
    virtual void tick() { m_Clock--; }
    
private:
    StudentWorld* m_studentWorld;
    int m_Clock;
    
};



// ====================================================================================//
// || - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ||
// || - - - - - - - - - - - DERIVED CLASSES FOR ANIMATE OBJECTS - - - - - - - - - - - ||
// || - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ||
// ====================================================================================//


class AnimateObject: public Actor
{
public:
    
    AnimateObject(int imageID, int startX, int startY, int hitPoints, Direction dir = right)
            : Actor(imageID, startX, startY, dir, 1.0, 0),
                m_Dead(false), m_hitPoints(hitPoints), m_hasHealth(true) { }
    
    virtual ~AnimateObject() {};
    
    virtual void annoy(int damage) {
        m_hitPoints -= damage;
        if (m_hitPoints <= 0) setDead();
    }
    virtual void setDead() {m_Dead = true; }
    virtual bool isDead() {return m_Dead; }
    virtual int getHitPoints() { return m_hitPoints; }
    
private:
    
    int m_hitPoints;
    bool m_Dead;
    bool m_hasHealth;
};



class TunnelMan : public AnimateObject
{
public:
    TunnelMan(StudentWorld* StudentWorld)
        : AnimateObject(TID_PLAYER, 30, 60, 10, right),
        m_StudentWorld(StudentWorld)
    { 
        m_nWaterSquirts = 500;
        m_nSonarKits = 1;
        m_nGoldenNuggets = 0;
        setAnimate(true); 
    }
    
    virtual ~TunnelMan() {};
    virtual void doSomething();
    
    // Accessor and Mutator Functions
    int& getNSquirt() { return m_nWaterSquirts; }
    int& getNKit() { return m_nSonarKits; }
    int& getNnugget() { return m_nGoldenNuggets; }

    
private:
    
    StudentWorld* m_StudentWorld;

    int m_nWaterSquirts;
    int m_nSonarKits;
    int m_nGoldenNuggets;
    
};


class RegularProtestor: public AnimateObject
{
public:
    RegularProtestor(StudentWorld* worldPtr)
    : AnimateObject(TID_PROTESTER, 60, 60, 5, left),
        m_studentWorld(worldPtr)
    {
        setState(searching);
        numSquaresToMoveInCurrentDirection = (rand() % 52) + 8;
        ticksToWaitBetweenMoves = fmax(0, 3 - (m_studentWorld->getLevel() / 4) ); 
    }
    
    virtual ~RegularProtestor() {};
    
    virtual void annoy(int damage);
    
    virtual void doSomething();
    
    virtual void shout(); 
    
    virtual void rest() {ticksToWaitBetweenMoves--; }
    
    virtual void tick();
    
    virtual void resetClock() { m_shoutClock = 15; }
    
    virtual void followExitPath(char dir);
    
    virtual void attemptShout();
    
    virtual void moveTowardPlayer();
    
    virtual bool sawPlayer();
    
    virtual void pickNewDirection();
    
    virtual bool moveOnce();
    
    virtual void checkForTurn(); 
    
protected:
    std::string m_exitPath = "";
    
private:
    
    StudentWorld* m_studentWorld;
    int numSquaresToMoveInCurrentDirection;
    int ticksToWaitBetweenMoves;
    int m_shoutClock;
    
};


class HardcoreProtestor: public RegularProtestor
{
public:
    
    
    
private:
    
    
    
    
};







#endif // ACTOR_H_
