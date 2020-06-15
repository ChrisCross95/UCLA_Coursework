
#ifndef GAME_H
#define GAME_H

#include <stdio.h>

#include "Arena.h"


class Game
{
public:
    // Constructor/destructor
    Game(int rows, int cols, int nRobots);
    ~Game();
    
    // Mutators
    void play();
    
private:
    Arena* m_arena;
};

#endif /* Game_hpp */
