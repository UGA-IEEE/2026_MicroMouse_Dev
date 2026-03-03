#include <stdio.h>
#include <stdbool.h>

const int MAZE_SIZE  = 16;

//were using bitwise operations for the walls so we can store multiple walls in one array
typedef enum {
    NO_WALLS = 0,
    WALL_NORTH = 1,
    WALL_EAST = 2,
    WALL_SOUTH = 4,
    WALL_WEST = 8,
    WALL_NORTH_EAST = WALL_NORTH | WALL_EAST,
    WALL_NORTH_SOUTH = WALL_NORTH | WALL_SOUTH,
    WALL_NORTH_WEST = WALL_NORTH | WALL_WEST,
    WALL_EAST_SOUTH = WALL_EAST | WALL_SOUTH,
    WALL_EAST_WEST = WALL_EAST | WALL_WEST,
    WALL_SOUTH_WEST = WALL_SOUTH | WALL_WEST,
    WALL_NORTH_EAST_SOUTH = WALL_NORTH | WALL_EAST | WALL_SOUTH,
    WALL_NORTH_EAST_WEST = WALL_NORTH | WALL_EAST | WALL_WEST,
    WALL_NORTH_SOUTH_WEST = WALL_NORTH | WALL_SOUTH | WALL_WEST,
    WALL_EAST_SOUTH_WEST = WALL_EAST | WALL_SOUTH | WALL_WEST
} Wall;

int floodFill[16][16] = {
    {14, 13, 12, 11, 10,  9,  8,  7,  7,  8,  9, 10, 11, 12, 13, 14},
    {13, 12, 11, 10,  9,  8,  7,  6,  6,  7,  8,  9, 10, 11, 12, 13},
    {12, 11, 10,  9,  8,  7,  6,  5,  5,  6,  7,  8,  9, 10, 11, 12},
    {11, 10,  9,  8,  7,  6,  5,  4,  4,  5,  6,  7,  8,  9, 10, 11},
    {10,  9,  8,  7,  6,  5,  4,  3,  3,  4,  5,  6,  7,  8,  9, 10},
    { 9,  8,  7,  6,  5,  4,  3,  2,  2,  3,  4,  5,  6,  7,  8,  9},
    { 8,  7,  6,  5,  4,  3,  2,  1,  1,  2,  3,  4,  5,  6,  7,  8},
    { 7,  6,  5,  4,  3,  2,  1,  0,  0,  1,  2,  3,  4,  5,  6,  7},
    { 7,  6,  5,  4,  3,  2,  1,  0,  0,  1,  2,  3,  4,  5,  6,  7},
    { 8,  7,  6,  5,  4,  3,  2,  1,  1,  2,  3,  4,  5,  6,  7,  8},
    { 9,  8,  7,  6,  5,  4,  3,  2,  2,  3,  4,  5,  6,  7,  8,  9},
    {10,  9,  8,  7,  6,  5,  4,  3,  3,  4,  5,  6,  7,  8,  9, 10},
    {11, 10,  9,  8,  7,  6,  5,  4,  4,  5,  6,  7,  8,  9, 10, 11},
    {12, 11, 10,  9,  8,  7,  6,  5,  5,  6,  7,  8,  9, 10, 11, 12},
    {13, 12, 11, 10,  9,  8,  7,  6,  6,  7,  8,  9, 10, 11, 12, 13},
    {14, 13, 12, 11, 10,  9,  8,  7,  7,  8,  9, 10, 11, 12, 13, 14}
};

Wall walls[MAZE_SIZE][MAZE_SIZE];

void initializeWalls() {
    for (int i = 0; i < MAZE_SIZE; i++) {
        for (int j = 0; j < MAZE_SIZE; j++) {
            walls[i][j] = NO_WALLS; 
        }
    }
}

void floodFillAlgorithm();
void updateWalls(int x, int y, Wall newWalls);

// Assumed functions to simulate wall detection and movement
bool checkWallLeft();
bool checkWallRight();
bool checkWallFront();
void followPath();

int main() {
    // Initialize the walls array
    initializeWalls();

    return 0;
}

void floodFillAlgorithm() {
    bool updated;
    do {
        updated = false;
        for (int x = 0; x < MAZE_SIZE; x++) {
            for (int y = 0; y < MAZE_SIZE; y++) {
                int currentDistance = floodFill[x][y];

                if ((x == 7 || x == 8) && (y == 7 || y == 8)) {
                    continue;
                }
                if (!(walls[x][y] & WALL_NORTH) && y > 0 && floodFill[x][y - 1] > currentDistance + 1) {
                    floodFill[x][y - 1] = currentDistance + 1;
                    updated = true;
                }
                if (!(walls[x][y] & WALL_EAST) && x < MAZE_SIZE - 1 && floodFill[x + 1][y] > currentDistance + 1) {
                    floodFill[x + 1][y] = currentDistance + 1;
                    updated = true;
                }
                if (!(walls[x][y] & WALL_SOUTH) && y < MAZE_SIZE - 1 && floodFill[x][y + 1] > currentDistance + 1) {
                    floodFill[x][y + 1] = currentDistance + 1;
                    updated = true;
                }
                if (!(walls[x][y] & WALL_WEST) && x > 0 && floodFill[x - 1][y] > currentDistance + 1) {
                    floodFill[x - 1][y] = currentDistance + 1;
                    updated = true;
                }
            }
        }
    } while (updated);
}

void updateWalls(int x, int y, Wall newWalls) {

    // using bitwise operations again to add walls instead of replacing them

    walls[x][y] |= newWalls;

    
    if (newWalls & WALL_NORTH && y > 0) {
        walls[x][y - 1] |= WALL_SOUTH;
    }
    if (newWalls & WALL_EAST && x < MAZE_SIZE - 1) {
        walls[x + 1][y] |= WALL_WEST;
    }
    if (newWalls & WALL_SOUTH && y < MAZE_SIZE - 1) {
        walls[x][y + 1] |= WALL_NORTH;
    }
    if (newWalls & WALL_WEST && x > 0) {
        walls[x - 1][y] |= WALL_EAST;
    }
}