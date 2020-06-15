
#include <iostream>

#include "Before.h"
#include "Arena.h"
#include "Game.h"
#include "globals.h"

using namespace std;

Before::Before(int nRows, int nCols) {
    
    this->m_rows = nRows;
    this->m_cols = nCols;
    
    for (int k = 1; k <= nRows; k++)
        for (int j = 1; j <= nCols; j++)
            m_historyGrid[k][j] = '.';
}

bool Before::keepTrack(int r, int c) {
    
    if (r < 1 || c < 1 || r > m_rows || c > m_cols)
        return false;
    
    if (m_historyGrid[r][c] == '.') {
        m_historyGrid[r][c] = 65;
        return true;
    }
    
    if (m_historyGrid[r][c] == 'Z')
        return true;
    
    m_historyGrid[r][c]++;
    return true;
}

void Before::printWhatWasBefore() const {
    
    clearScreen();
    
    for (int k = 1; k <= m_rows; k++) {
        cout << endl;
        for (int j = 1; j <= m_cols; j++) {
            cout << m_historyGrid[k][j];
        }
    }
    cout << endl << endl; 
}
