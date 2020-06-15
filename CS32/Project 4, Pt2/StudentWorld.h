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
class GoldenNugget;


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
    virtual void setDisplayText();
    
    virtual std::string formatDisplay(int level, int lives, int health, int squirts,
                               int gold, int barrelsLeft, int sonar, int score); 
    
    virtual void digEarth(const int& xCoord, const int& yCoord);
    
    virtual bool checkEarth(const int& xCoord, const int& yCoord);
    
    virtual bool blocked(const int& xCoord, const int& yCoord);
    
    virtual void crushedByRock(const int& xCoord, const int& yCoord);
    
    virtual bool hitBySquirt(const int& xCoord, const int& yCoord); 
    
    virtual double getDistance(int x1, int y1, int x2, int y2);
    
    virtual void decBarrelCount() { m_barrelsLeft--; }
    
    virtual void addGoodies();
    
    virtual void unveil(); 
    
    
    
    // The following three functions are used to generate an exit path for a protestor that has given up
    virtual void create_path(int xCoord, int yCoord, std::string& path);
    
    // Taken from  Kung-Hua Chang's 2015 CS 32 Final Study Guide
    virtual void find_path(char maze[63][60], int xCoord, int yCoord, std::string& path, bool &found, int bx = 60, int by = 60);
    
    virtual void update_protestorMaze();
    
    // Accessor and Mutator Functions
    virtual void getSquirt(WaterSquirt* squirt);
    virtual void getGold(GoldenNugget* nugget);
    void incSquirts();
    void incKits();
    void incNuggets();
    void decSquirts();
    void dectKits();
    void dectNugeets(); 
    
    int getPlayerXCoord();
    int getPlayerYCoord();
    
    virtual bool clearPath(int beginx, int beginy, int endx, int endy);

    
    void annoyPlayer(); 
    
private:

    
    TunnelMan* m_TunnelMan;
    
    std::vector<Actor* > m_actors;
    
    Earth* ground[63][60]; // 2D Array of Ground Pointers
    
    char m_protestorMaze[63][60];
    
    bool m_dugUpEarthFlag;
    
    int m_barrelsLeft;
};

#endif // STUDENTWORLD_H_
