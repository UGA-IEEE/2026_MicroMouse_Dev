#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

// Greedy algorithm for path following in a maze
// Inputs: floodFill array and walls array from FloodFill.c
// Body: determines next move + calls for bot movements
// Outputs: new position and orientation of the robot

const int MAZE_SIZE = 16;

// Direction enumeration: 0=East, 1=South, 2=West, 3=North
typedef enum {
    EAST = 0,
    SOUTH = 1,
    WEST = 2,
    NORTH = 3
} Direction;

// Robot state
typedef struct {
    int x;
    int y;
    Direction facing;
} RobotState;

// Move result tuple
typedef struct {
    int x;
    int y;
    Direction facing;
} MoveResult;

// Wall flags (matching FloodFill.c)
typedef enum {
    NO_WALLS = 0,
    WALL_NORTH = 1,
    WALL_EAST = 2,
    WALL_SOUTH = 4,
    WALL_WEST = 8
} Wall;

// External floodFill array from FloodFill.c
extern int floodFill[16][16];

// External functions to be implemented
// Returns updated floodFill array and updates the global floodFill
extern int** findWalls();

// Returns the walls present in the current cell
extern Wall cellWalls();

// Moves the robot to the specified position with given orientation
extern void moveTo(int x, int y, Direction orientation);

// Helper function to get wall flag based on direction
Wall getWallForDirection(Direction dir) {
    switch (dir) {
        case NORTH:
            return WALL_NORTH;
        case EAST:
            return WALL_EAST;
        case SOUTH:
            return WALL_SOUTH;
        case WEST:
            return WALL_WEST;
        default:
            return NO_WALLS;
    }
}

// Helper function to get next x position based on direction
int getNextX(int x, Direction dir) {
    switch (dir) {
        case EAST:
            return x + 1;
        case WEST:
            return x - 1;
        default:
            return x;
    }
}

// Helper function to get next y position based on direction
int getNextY(int y, Direction dir) {
    switch (dir) {
        case SOUTH:
            return y + 1;
        case NORTH:
            return y - 1;
        default:
            return y;
    }
}

// Helper function to check if position is within maze bounds
bool isWithinBounds(int x, int y) {
    return x >= 0 && x < MAZE_SIZE && y >= 0 && y < MAZE_SIZE;
}

// Greedy algorithm that determines the next move
// Returns a tuple with the next cell position and robot orientation
MoveResult greedyPathFollow(RobotState *robot) {
    MoveResult result;
    result.x = robot->x;
    result.y = robot->y;
    result.facing = robot->facing;
    
    // Update walls in current cell and get updated floodFill array
    int** updatedFloodFill = findWalls();
    
    // Get walls in current cell
    Wall currentWalls = cellWalls();
    
    // Check all four directions
    int minDistance = INT_MAX;
    Direction bestDir = robot->facing;
    
    Direction directions[4] = {NORTH, EAST, SOUTH, WEST};
    
    for (int i = 0; i < 4; i++) {
        Direction dir = directions[i];
        
        // Check if there's a wall in this direction
        if (currentWalls & getWallForDirection(dir)) {
            continue;  // Skip this direction due to wall
        }
        
        int nextX = getNextX(robot->x, dir);
        int nextY = getNextY(robot->y, dir);
        
        // Check bounds
        if (!isWithinBounds(nextX, nextY)) {
            continue;
        }
        
        // Get distance to goal in this direction
        int distance = floodFill[nextX][nextY];
        
        // Update if this is the minimum distance
        if (distance < minDistance) {
            minDistance = distance;
            bestDir = dir;
        }
    }
    
    // Set the result with the best direction found
    result.x = getNextX(robot->x, bestDir);
    result.y = getNextY(robot->y, bestDir);
    result.facing = bestDir;
    
    // Print the result
    printf("Current position: (%d, %d), Next move: (%d, %d) facing %d\n",
           robot->x, robot->y, result.x, result.y, result.facing);
    
    return result;
}

// Main path following loop
void runPathFollowingLoop() {
    RobotState robot;
    robot.x = 0;
    robot.y = 0;
    robot.facing = EAST;
    
    printf("Starting path following from (%d, %d) facing EAST\n", robot.x, robot.y);
    
    int steps = 0;
    const int MAX_STEPS = 1000;  // Prevent infinite loops
    const int GOAL_X_MIN = 7, GOAL_X_MAX = 8, GOAL_Y_MIN = 7, GOAL_Y_MAX = 8;
    
    while (steps < MAX_STEPS) {
        // Get next move using greedy algorithm
        MoveResult nextMove = greedyPathFollow(&robot);
        
        // Check if we've reached the goal
        if ((nextMove.x >= GOAL_X_MIN && nextMove.x <= GOAL_X_MAX) &&
            (nextMove.y >= GOAL_Y_MIN && nextMove.y <= GOAL_Y_MAX)) {
            printf("Goal reached at (%d, %d)!\n", nextMove.x, nextMove.y);
            moveTo(nextMove.x, nextMove.y, nextMove.facing);
            break;
        }
        
        // Move to the next cell
        moveTo(nextMove.x, nextMove.y, nextMove.facing);
        
        // Update robot state
        robot.x = nextMove.x;
        robot.y = nextMove.y;
        robot.facing = nextMove.facing;
        
        steps++;
    }
    
    if (steps >= MAX_STEPS) {
        printf("Maximum steps reached without finding goal\n");
    } else {
        printf("Path following completed in %d steps\n", steps);
    }
}
