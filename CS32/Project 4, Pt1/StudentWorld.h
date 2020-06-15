#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "freeglut.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include "GameController.h"
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include "Actor.h"

class Actor;
class TunnelMan;
class Earth;
class Boulder;
class SonarKit;
class Barrel;
class WaterSquirt;


const int UpperBoundary = 60;
const int LowerBoundary = 0;
const int LeftBoundary  = 0;
const int RightBoundary = 60; 

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
		: GameWorld(assetDir)
	{
	}
    
    
    // Primary Functions
    virtual ~StudentWorld();  // ** NOT DONE

    virtual int init();           // ** NOT DONE

    virtual int move();         // ** NOT DONE

    virtual void cleanUp();         // ** NOT DONE Does what the destructor does 
    
    
    // Auxillary Member Functions
    virtual void digEarth(const int& xCoord, const int& yCoord);
    
    virtual bool checkEarth(const int& xCoord, const int& yCoord);
    
    virtual void getSquirt(WaterSquirt* squirt);
    
    virtual bool blocked(const int& xCoord, const int& yCoord);
    
    virtual void crushedByRock(const int& xCoord, const int& yCoord);
    
    virtual bool hitBySquirt(const int& xCoord, const int& yCoord); 
    
    virtual double getDistance(int x1, int y1, int x2, int y2);
    
    virtual void decBarrelCount() { m_nBarrels--; }
    
    virtual int getPlayerXCoord();
    
    virtual int getPlayerYCoord();
    

private:
    
    TunnelMan* m_TunnelMan;
    
    std::vector<Actor* > m_actors;
    
    std::vector<WaterSquirt* > m_squirts;
    
    Earth* ground[63][60]; // 2D Array of Ground Pointers
    
    bool m_dugUpEarthFlag;
    
    int m_nBarrels; 
    
};

#endif // STUDENTWORLD_H_
