//https://weblog.jamisbuck.org/2010/12/27/maze-generation-recursive-backtracking

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <SDL2/SDL.h>

typedef char BOOL;
#define TRUE 1
#define FALSE 0

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   800
#define FPS_COUNT       60

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

typedef enum Directions
{
    cNone = -1,
    cUp    = 0,
    cRight = 1,
    cDown  = 2,
    cLeft  = 3
} Directions;

typedef struct Vector
{
    int x;
    int y;
} Vector;

const int TILE_SIZE = 8;

typedef struct Map
{
    int cols;
    int rows;
    int numTiles;
    int wPix;
    int hPix;
    char * tiles;
    Vector * paths;
} Map;

Directions GetEnumFromDir(Vector v)
{
    if(v.x == -1) return cLeft;
    if(v.x == 1)  return cRight;
    if(v.y == -1) return cUp;
    if(v.y == 1)  return cDown;
    
    return cNone;
};

void SetDirFromEnum(Directions dir, Vector * v)
{
    if(dir == cUp)      { v->x = 0;  v->y = -1; }
    if(dir == cRight)   { v->x = 1;  v->y = 0; }
    if(dir == cDown)    { v->x = 0;  v->y = 1; }
    if(dir == cLeft)    { v->x = -1; v->y = 0; }
}

int ToIndex(Map * map, int x, int y)
{
    if(x >= -2 && x < map->cols + 2 && y >= 0 && y < map->rows)
    {
        return (x + 2) + y * (map->cols + 4);
    }
}

Vector GetStartPoint(Map * map, int tx, int ty, Directions dir, BOOL edges[], int pad)
{
    Vector v = {0, 0};
    SetDirFromEnum(dir, &v);
    if(!edges[ToIndex(map, tx + v.y, ty - v.x)])
    {
        pad = IsFloorTile(map, tx + v.y, ty - v.x) ? 5 : 0;
    }
    int px = -TILE_SIZE / 2 + pad;
    int py = TILE_SIZE / 2;
    float a = dir * M_PI / 2;
    float c = cos(a);
    float s = sin(a);
    v.x = (px * c - py * s) + (tx + 0.5f) * TILE_SIZE;
    v.y = (px * s - py * c) + (ty + 0.5f) * TILE_SIZE;
    return v;
}

void MakePath(Map * map, BOOL edges[], BOOL visited[], int tx, int ty)
{
    Vector v = {0, 0};
    Directions dir = cNone;
    if(edges[ToIndex(map, tx + 1, ty)])
    {
        dir = cRight;
    }
    else if(edges[ToIndex(map, tx, ty + 1)])
    {
        dir = cDown;
    }
    else
    {
        printf("error");
        return;
    }
    SetDirFromEnum(dir, &v);

    tx += v.x;
    ty += v.y;

    int tmptx = tx;
    int tmpty = ty;
    int tmpdir = dir;

    Vector path[1];
    int pad; // ?
    Vector point; // ?
    Vector lastPoint; //?

    BOOL turn = FALSE;
    BOOL turnAround = FALSE;

    while(TRUE)
    {
        visited[ToIndex(map, tx, ty)] = TRUE;
        point = GetStartPoint(map, tx, ty, dir, edges, pad);

        if(turn)
        {
            lastPoint = path[ARRAY_SIZE(path) - 1];
            if(v.x == 0)
            {
                point.cx = point.x;
                point.cy = lastPoint.y;
            }
            else
            {
                point.cx = lastPoint.x;
                point.cy = point.y;
            }
        }

        turn = FALSE;
        turnAround = FALSE;

        if(edges[ToIndex(map, tx + v.y, ty - v.x)])
        {
            dir = (dir + 3) % 4;
            turn = TRUE;
        }
        else if(edges[ToIndex(map, tx + v.x, ty + v.y)])
        {
            // No op
        }
        else if(edges[ToIndex(map, tx - v.y, ty + v.x)])
        {
            dir = (dir + 1) % 4;
            turn = TRUE;
        }
        else
        {
            dir = (dir + 2) % 4;
            turnAround = TRUE;
        }
        SetDirFromEnum(dir, &v);
        // path.push(point);

        if(turnAround)
        {
            //path.push(getStartPoint(tx-dir.x, ty-dir.y, (dirEnum+2)%4));
            //path.push(getStartPoint(tx, ty, dirEnum));
        }

        tx += v.x;
        ty += v.y;

        if(tx == tmptx && ty == tmpty && dir == tmpdir)
        {
            //that.paths.push(path);
            break;
        }
    }
}

void ParseWalls(Map * map)
{
    BOOL visited[map->numTiles];
    memset(&visited, FALSE, map->numTiles);

    BOOL edges[map->numTiles];
    memset(&edges, FALSE, map->numTiles);

    int i = 0;
    for(int y = 0; y < map->rows, y++)
    {
        for(int x = 0; x < map->cols; x++)
        {
            if(edges[i] && !visited[i])
            {
                visited[i] = TRUE;
                MakePath(map, edges, visited, x, y);
            }
        }
    }
}

int PosToIndex(Map * map, int x, int y)
{
    if(x >= 0 && x < map->rows && y >= 0 && y < map->cols)
    {
        return x + y * map->cols;
    }
    return -1;
}

char GetTile(Map * map, int x, int y)
{
    int index = PosToIndex(map, x, y);
    if(index > -1)
    {
        return map->tiles[index];
    }

    if((x == -1 && GetTile(map, x + 1, y) == '|' && (IsFloorTile(map, x + 1, y + 1) || IsFloorTile(map, x + 1, y - 1))) ||
    (x == map->cols && GetTile(map, x - 1, y) == '|' && (IsFloorTile(map, x - 1, y - 1) || IsFloorTile(map, x - 1, y - 1))))
    {
        return '|';
    }
    if((x == -1 && IsFloorTile(map, x + 1, y)) ||
    (x == map->cols && IsFloorTile(x - 1, y)))
    {
        return ' ';
    }
    return 'X';
}

BOOL IsCharFloorTile(char tile)
{
    return tile == ' ' || tile == '.' || tile == 'o';
}

BOOL IsFloorTile(Map * map, int x, int y)
{
    return IsCharFloorTile(GetTile(map, x, y));
}
/*

char GetCell(Map * map, int x, int y)
{
    // height is the offset for accessing 2D array
    return *((map->tiles + x * map->cols) + y);
}


void SetCell(Map * map, int x, int y, char value)
{
    // height is the offset for accessing 2D array
    *((map->tiles + x * map->cols) + y) = value;
}
*/

void Init(Map * map, int rows, int cols, char value)
{
    map->rows  = rows;
    map->cols = cols;
    map->tiles = (char *)malloc(rows * cols * sizeof(char));
    for(int i = 0; i < rows; i++)
    {
        for(int ii = 0; ii < cols; ii++)
        {
            SetCell(map, i, ii, value);
        }
    }
}

/*
    "Destroy" map, setting all values to 0 and freeing the array
*/
void Destroy(Map * map)
{
    map->rows  = 0;
    map->cols = 0;
    map->hPix = 0;
    map->wPix = 0;
    map->numTiles = 0;
    free(map->tiles);
}


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
    Init(&map, 23, 23, '_');

    BOOL running = TRUE;
    while(running)
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
        int rectSize = TILE_SIZE;
        for(int i = 0, x = 0; i < map.rows; i++, x += rectSize)
        {
            for(int ii = 0, y = 0; ii < map.cols; ii++, y += rectSize)
            {            
                char c = GetCell(&map, i, ii);
                if(c == '_')
                {
                    SDL_SetRenderDrawColor(pRenderer, 54, 34, 199, SDL_ALPHA_OPAQUE);
                }
                else
                {
                    SDL_SetRenderDrawColor(pRenderer, 199, 34, 54, SDL_ALPHA_OPAQUE);
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
    }

    // Cleanup
    Destroy(&map);

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
