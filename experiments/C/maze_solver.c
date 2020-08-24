#define _GNU_SOURCE

#include "./common/common.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   800
#define FPS_COUNT       25

/*
    Struct representing a node on a map
    x = x coordinate
    y = y coordinate
*/
typedef struct Node
{
    unsigned int x;
    unsigned int y;
} Node;

/*
    Drawing, input and rendering function
    Moved to a function so that it can be called from within multiple parts of the maze solving algorithm
*/
BOOL DrawCells(Map * map, SDL_Renderer * pRenderer)
{
    // Clear screen and set renderer colour to white
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(pRenderer);

    // Check events to resolve exit and to keep app responsive
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0)
    {
        if(e.type == SDL_QUIT)
        {
            return FALSE;
        }
    }
    
    // Draw the map cells
    int rectSize = __min(WINDOW_WIDTH / map->width, WINDOW_HEIGHT / map->height);
    for(int i = 0, x = 0; i < map->width; i++, x += rectSize)
    {
        for(int ii = 0, y = 0; ii < map->height; ii++, y += rectSize)
        {
            switch(Map_namespace.GetCell(map, i, ii))
            {
            case WALL:
                SDL_SetRenderDrawColor(pRenderer, 54, 34, 199, SDL_ALPHA_OPAQUE);
                break;
            case PATH:
                SDL_SetRenderDrawColor(pRenderer, 54, 199, 34, SDL_ALPHA_OPAQUE);
                break;
            case FREE:
                SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                break;
            default: // Route candidate
                SDL_SetRenderDrawColor(pRenderer, 255, 165, 0, SDL_ALPHA_OPAQUE);
                break;
            }
            SDL_Rect rect;
            rect.w = rectSize - 1;
            rect.h = rectSize - 1;
            rect.x = x;
            rect.y = y;
            SDL_RenderFillRect(pRenderer, &rect);
        }
    }
    // Update screen and delay
    SDL_RenderPresent(pRenderer);
    SDL_Delay((1.0 / FPS_COUNT) * 1000);
    return TRUE;
}

/*
    Visual implementation of the Solve function below
    Encapsulates all solving and rendering logic
*/
void SolveRealtime(Map * map, SDL_Renderer * pRenderer)
{
    // Setup
    int size = 1;  // size of our queue
    int index = 0; // Index of the start of the queue 
    int distanceCount = 3; // Distance walked (start at 3 to avoid confusion between 0 (free space) and 1 (wall))
    int x = 0;
    int y = 0;
    BOOL newDistance = TRUE;
    BOOL running = TRUE;
    Node * distances = (Node *)malloc(sizeof(Node)); // Create a new array of size 1 and add the starting point
    distances[0].x = map->start;
    distances[0].y = 0;

    // Walk away from the start location, remembering the distance to it
    while(newDistance && running)
    {
        newDistance = FALSE;
        distanceCount++;
        int distanceMax = size - index; // Offset by the starting index (see below for more info)
        for(int i = 0; i < distanceMax && running; i++)
        {
            x = distances[index].x;
            y = distances[index].y;
            Map_namespace.SetCell(map, x, y, distanceCount);
            // North
            if((y > 0) && Map_namespace.GetCell(map, x, y - 1) == FREE)
            {
                if(!ResizeArray((void **)&distances, ++size, sizeof(Node)))
                {
                    running = FALSE;
                    break;
                }
                distances[size - 1].x = x;
                distances[size - 1].y = y - 1;
                newDistance = TRUE;
            }
             // South
            if(((y + 1) < map->height) && Map_namespace.GetCell(map, x, y + 1) == FREE)
            {
                if(!ResizeArray((void **)&distances, ++size, sizeof(Node)))
                {
                    running = FALSE;
                    break;
                }
                distances[size - 1].x = x;
                distances[size - 1].y = y + 1;
                newDistance = TRUE;
            }
            // East
            if(((x + 1) < map->width) && Map_namespace.GetCell(map, x + 1, y) == FREE)
            {
                if(!ResizeArray((void **)&distances, ++size, sizeof(Node)))
                {
                    running = FALSE;
                    break;
                }
                distances[size - 1].x = x + 1;
                distances[size - 1].y = y;
                newDistance = TRUE;
            }
            // West
            if((x > 0) && Map_namespace.GetCell(map, x - 1, y) == FREE)
            {
                if(!ResizeArray((void **)&distances, ++size, sizeof(Node)))
                {
                    running = FALSE;
                    break;
                }
                distances[size - 1].x = x - 1;
                distances[size - 1].y = y;
                newDistance = TRUE;
            }
            // Destination. Break out of the loop
            if(y == map->height - 1 && x == map->end)
            {
                newDistance = FALSE;
                break;
            }
            // Best approach would be to have a linked list and simply pop the first element from the queue
            // I am too lazy and using a growing array instead
            distances[index].x = -1;
            distances[index++].y = -1;
            running = DrawCells(map, pRenderer);
        }
    }

    // Walk back towards the exit, marking nodes as solution on the way
    x = map->end;
    y = map->height - 1;
    distanceCount = Map_namespace.GetCell(map, x, y);
    while(running && distanceCount != 3)
    {
        Map_namespace.SetCell(map, x, y, PATH);
        distanceCount--;
        // North
        if((y > 0) && (distanceCount == Map_namespace.GetCell(map, x, y - 1)))
        {
            y--;
        }
        // South
        else if(((y + 1) < map->height) && (distanceCount == Map_namespace.GetCell(map, x, y + 1)))
        {
            y++;
        }
        // East
        else if(((x + 1) < map->width) && (distanceCount == Map_namespace.GetCell(map, x + 1, y)))
        {
            x++;
        }
        // West
        else if((x > 0) && (distanceCount == Map_namespace.GetCell(map, x - 1, y)))
        {
            x--;
        }
        running = DrawCells(map, pRenderer);
    }    

    // Cleanup.
    // Scan each cell and if not a wall or a path, set to free cell
    for(int i = 0; i < map->width; i++)
    {
        for(int ii = 0; ii < map->height; ii++)
        {
            if(Map_namespace.GetCell(map, i, ii) != WALL && Map_namespace.GetCell(map, i, ii) != PATH)
            {
                Map_namespace.SetCell(map, i, ii, FREE);
            }
        }
    }

    while(running)
    {
        running = DrawCells(map, pRenderer);
    }
}

/*
    Solve the map using Dijkstra
*/
BOOL Solve(Map * map)
{
    int size = 1;  // size of our queue
    int index = 0; // Index of the start of the queue 
    int distanceCount = 3; // Distance walked (start at 3 to avoid confusion between 0 (free space) and 1 (wall))
    int x = 0;
    int y = 0;
    BOOL newDistance = TRUE;
    Node * distances = (Node *)malloc(sizeof(Node)); // Create a new array of size 1 and add the starting point
    distances[0].x = map->start;
    distances[0].y = 0;

    // Walk away from the start location, remembering the distance to it
    while(newDistance)
    {
        newDistance = FALSE;
        distanceCount++;
        int distanceMax = size - index; // Offset by the starting index (see below for more info)

        for(int i = 0; i < distanceMax; i++)
        {
            x = distances[index].x;
            y = distances[index].y;
            Map_namespace.SetCell(map, x, y, distanceCount);
            // North
            if((y > 0) && Map_namespace.GetCell(map, x, y - 1) == FREE)
            {
                if(!ResizeArray((void **)&distances, ++size, sizeof(Node)))
                {
                    return FALSE;
                }
                distances[size - 1].x = x;
                distances[size - 1].y = y - 1;
                newDistance = TRUE;
            }
             // South
            if(((y + 1) < map->height) && Map_namespace.GetCell(map, x, y + 1) == FREE)
            {
                if(!ResizeArray((void **)&distances, ++size, sizeof(Node)))
                {
                    return FALSE;
                }
                distances[size - 1].x = x;
                distances[size - 1].y = y + 1;
                newDistance = TRUE;
            }
            // East
            if(((x + 1) < map->width) && Map_namespace.GetCell(map, x + 1, y) == FREE)
            {
                if(!ResizeArray((void **)&distances, ++size, sizeof(Node)))
                {
                    return FALSE;
                }
                distances[size - 1].x = x + 1;
                distances[size - 1].y = y;
                newDistance = TRUE;
            }
            // West
            if((x > 0) && Map_namespace.GetCell(map, x - 1, y) == FREE)
            {
                if(!ResizeArray((void **)&distances, ++size, sizeof(Node)))
                {
                    return FALSE;
                }
                distances[size - 1].x = x - 1;
                distances[size - 1].y = y;
                newDistance = TRUE;
            }
            // Destination. Break out of the loop
            if(y == map->height - 1 && x == map->end)
            {
                newDistance = FALSE;
                break;
            }

            // Best approach would be to have a linked list and simply pop the first element from the queue
            // I am too lazy and using a growing array instead
            distances[index].x = -1;
            distances[index++].y = -1;
        }
    }

    // Walk back towards the exit, marking nodes as solution on the way
    x = map->end;
    y = map->height - 1;
    distanceCount = Map_namespace.GetCell(map, x, y);
    while(distanceCount != 3)
    {
        Map_namespace.SetCell(map, x, y, PATH);
        distanceCount--;
        // North
        if((y > 0) && (distanceCount == Map_namespace.GetCell(map, x, y - 1)))
        {
            y--;
        }
        // South
        else if(((y + 1) < map->height) && (distanceCount == Map_namespace.GetCell(map, x, y + 1)))
        {
            y++;
        }
        // East
        else if(((x + 1) < map->width) && (distanceCount == Map_namespace.GetCell(map, x + 1, y)))
        {
            x++;
        }
        // West
        else if((x > 0) && (distanceCount == Map_namespace.GetCell(map, x - 1, y)))
        {
            x--;
        }
    }

    // Cleanup.
    // Scan each cell and if not a wall or a path, set to free cell
    for(int i = 0; i < map->width; i++)
    {
        for(int ii = 0; ii < map->height; ii++)
        {
            if(Map_namespace.GetCell(map, i, ii) != WALL && Map_namespace.GetCell(map, i, ii) != PATH)
            {
                Map_namespace.SetCell(map, i, ii, FREE);
            }
        }
    }
    return TRUE;
}

//The parameters in the main function cannot be omitted, or an error will be reported
int main(int arg, char *argv[])
{
    SDL_Window   * pWindow   = NULL;
    SDL_Surface  * pSurface  = NULL;
    SDL_Renderer * pRenderer = NULL;

    // SDL Setup
    if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN, &pWindow, &pRenderer) != 0)
    {
        return -1;
    }
    pSurface = SDL_GetWindowSurface(pWindow);
    if (!pSurface)
    {
        return -2;
    }

    // Seed the random generator
    time_t t;
    srand((unsigned) time(&t));

    // Load map
    Map map;
    if(Map_namespace.LoadMap(&map, "map.txt"))
    {
        SolveRealtime(&map, pRenderer); // Main loop inside
    }

    // Cleanup
    Map_namespace.Destroy(&map);

    // SDL Cleanup
    if(pSurface)
    {
        SDL_FreeSurface(pSurface);
    }
    if(pRenderer)
    {
        SDL_DestroyRenderer(pRenderer);
    }
    if(pWindow)
    {
        SDL_DestroyWindow(pWindow);
    }
    SDL_Quit();
    return 0;
}