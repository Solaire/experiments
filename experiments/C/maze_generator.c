//https://weblog.jamisbuck.org/2010/12/27/maze-generation-recursive-backtracking

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "./common/common.h"

#include <SDL2/SDL.h>

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   800
#define FPS_COUNT       60

enum Directions
{
    cNorth = 1,
    cSouth = 2,
    cEast  = 4,
    cWest  = 8
};

int ShuffleArray(int * array, int size)
{
    for(int i = 0; i < size - 1; i++)
    {
        int r = i + (rand() % (size - i));
        array[i] += array[r];
        array[r] = array[i] - array[r];
        array[i] -= array[r];
    }
}

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
    Recursive backtracking algorithm
    Carve a path at random and recursively make branches
*/
BOOL CarvePassageFrom(Map * map, SDL_Renderer * pRenderer, int x, int y, int depth)
{
    // 4 directions that we can go in, shuffled
    int directions[4] = {cNorth, cEast, cSouth, cWest};
    ShuffleArray(directions, 4);
    for(int i = 0; i < 4; i++)
    {
        if(directions[i] == cNorth)
        {
            if((y > 2) && Map_namespace.GetCell(map, x, y - 2) == WALL)
            {
                for(int ii = 0; ii < 3; ii++)
                {
                    Map_namespace.SetCell(map, x, y - 2 + ii, FREE);
                    if(!DrawCells(map, pRenderer))
                    {
                        return FALSE;
                    }
                }
                if(!CarvePassageFrom(map, pRenderer, x, y - 2, depth + 1))
                {
                    return FALSE;
                }
            }
        }
        else if(directions[i] == cSouth)
        {
            if(((y + 2) < map->height - 1) && Map_namespace.GetCell(map, x, y + 2) == WALL)
            {
                for(int ii = 0; ii < 3; ii++)
                {
                    Map_namespace.SetCell(map, x, y + ii, FREE);
                    if(!DrawCells(map, pRenderer))
                    {
                        return FALSE;
                    }
                }
                if(!CarvePassageFrom(map, pRenderer, x, y + 2, depth + 1))
                {
                    return FALSE;
                }
            }
        }
        else if(directions[i] == cEast)
        {
            if(((x + 2) < map->width - 1) && Map_namespace.GetCell(map, x + 2, y) == WALL)
            {
                for(int ii = 0; ii < 3; ii++)
                {
                    Map_namespace.SetCell(map, x + ii, y, FREE);
                    if(!DrawCells(map, pRenderer))
                    {
                        return FALSE;
                    }
                }
                if(!CarvePassageFrom(map, pRenderer, x + 2, y, depth + 1))
                {
                    return FALSE;
                }
            }
        }
        else if(directions[i] == cWest)
        {
            if((x > 2) && Map_namespace.GetCell(map, x - 2, y) == WALL)
            {
                for(int ii = 0; ii < 3; ii++)
                {
                    Map_namespace.SetCell(map, x - 2 + ii, y, FREE);
                    if(!DrawCells(map, pRenderer))
                    {
                        return FALSE;
                    }
                }
                if(!CarvePassageFrom(map, pRenderer, x - 2, y, depth + 1))
                {
                    return FALSE;
                }
            }
        }
    }

    if(depth == 1)
    {
        // Generate an endpoint
        {
            int min     = 1;
            int max     = map->width - 2;
            map->end     = (unsigned int)(rand() % (max + 1 - min) + min);    
            Map_namespace.SetCell(map, map->end, map->width - 1, 0);
        }
        BOOL running = Map_namespace.SaveMap(map, "newmap.txt");
        while(running)
        {
            running = DrawCells(map, pRenderer);
        }
        if(!running)
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*
    Generating a maze using recursive backtracking method.
    It's not perfect and sometimes the maze has a lot of blank spaces but it's good enough
*/
int main(int argc, char * argv[])
{
    SDL_Window   * pWindow   = NULL;
    SDL_Surface  * pSurface  = NULL;
    SDL_Renderer * pRenderer = NULL;

    // SDL2 Setup
    if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN, &pWindow, &pRenderer) != 0)
    {
        return -1;
    }
    pSurface = SDL_GetWindowSurface(pWindow);
    if (!pSurface)
    {
        return -2;
    }

    Map map;
    Map_namespace.Init(&map, 32, 32, WALL);
    
    // generate start end manually, since all cells are of WALL value
    // Keep it in separate scope
    {
        int min     = 1;
        int max     = map.width - 2;
        map.start   = (unsigned int)(rand() % (max + 1 - min) + min);
        Map_namespace.SetCell(&map, map.start, 0, 0);
    }
    CarvePassageFrom(&map, pRenderer, map.start, 1, 1); // Main loop inside

    // Cleanup
    Map_namespace.Destroy(&map);

    // SDL2 Cleanup
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
