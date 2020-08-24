#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define FPS           30
#define STAR_COUNT    350

// Map a value from one range to another
long MapValue(const float val, const int inStart, const int inEnd, const int outStart, const int outEnd)
{
    return (val - inStart) * (outEnd - outStart) / (inEnd - inStart) + outStart;
}

/*
Star structure
x is position along the horizontal axis
y is position along the vertical axis
z is position along the third axis (used to simulare distance to camera)
initZ is the initial z coordinate (used to draw the 'lightspeed' lines)
*/
typedef struct Star
{
    float x;
    float y;
    float z;
    float initZ;
} Star;

/*
Initialise a star, set three coordinates at random
*/
void StarSetup(struct Star * pStar)
{
    // Offset by half of the screen size so that the stars will disperse outwards
    pStar->x = (rand() % (WINDOW_WIDTH  - -WINDOW_WIDTH  + 1)) + -WINDOW_WIDTH;
    pStar->y = (rand() % (WINDOW_HEIGHT - -WINDOW_HEIGHT + 1)) + -WINDOW_HEIGHT;
    pStar->z = rand()  % WINDOW_WIDTH;
    pStar->initZ = pStar->z;
}

/*
Update star position, moving it closer towards the camera
*/
void StarUpdate(struct Star * pStar, const int speed)
{
    pStar->z -= speed; // Moving closer towards the 'camera'
    if(pStar->z < 1) // Reset the star coordinates. Set Z to start so it's far away
    {
        pStar->x = (rand() % (WINDOW_WIDTH  - -WINDOW_WIDTH  + 1)) + -WINDOW_WIDTH;
        pStar->y = (rand() % (WINDOW_HEIGHT - -WINDOW_HEIGHT + 1)) + -WINDOW_HEIGHT;
        pStar->z = WINDOW_WIDTH;
        pStar->initZ = pStar->z;
    }
}

/*
Draw the star on the screen
*/
void StarDraw(const struct Star star, SDL_Renderer * pRenderer)
{
    // Map the coordinates from 0-1 to 0-width/height and add half of the window height due to the offsetting
    SDL_Rect rect; 
    rect.x = MapValue(star.x / star.z, 0, 1, 0, WINDOW_WIDTH)  + (WINDOW_WIDTH  / 2);
    rect.y = MapValue(star.y / star.z, 0, 1, 0, WINDOW_HEIGHT) + (WINDOW_HEIGHT / 2);
    long r = MapValue(star.z, 0, WINDOW_WIDTH, 10, 1); // Star size
    rect.h = r;
    rect.w = r;
    SDL_RenderFillRect(pRenderer, &rect);

    // Draw the 'lightspeed' line between star's initial z position and current z position
    // Same as above, map to value and add offset
    float prevX = MapValue(star.x / star.initZ, 0, 1, 0, WINDOW_WIDTH)  + (WINDOW_WIDTH  / 2);
    float prevY = MapValue(star.y / star.initZ, 0, 1, 0, WINDOW_HEIGHT) + (WINDOW_HEIGHT  / 2);
    SDL_RenderDrawLine(pRenderer, prevX, prevY, rect.x + (rect.w / 2), rect.y + (rect.h / 2));
}

int main(int arg, char *argv[])
{
    SDL_Window   * pWindow   = NULL;
    SDL_Surface  * pSurface  = NULL;
    SDL_Renderer * pRenderer = NULL;

    // SDL setup
    if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN, &pWindow, &pRenderer) != 0)
    {
        return -1;
    }
    pSurface = SDL_GetWindowSurface(pWindow);
    if (!pSurface)
    {
        return -2;
    }

    // Random generator seed
    time_t t;
    srand((unsigned) time(&t));

    // Declare a star array and fill with random values
    struct Star stars[STAR_COUNT];
    for(int i = 0; i < STAR_COUNT; i++)
    {
        StarSetup(&stars[i]);
    }

    int speed = 0;
    int quit  = 0;
    while (!quit)
    {
        // Clear screen and set renderer colour to white
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(pRenderer);
        SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

        // Event capture and process
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                if(e.wheel.y < 0)
                {
                    speed = __max(speed - 2, 0);
                }
                else
                {
                    speed = __min(speed + 2, 24);
                }
            }
        }

        // Update and draw
        for(int i = 0; i < STAR_COUNT; i++)
        {
            StarUpdate(&stars[i], speed);
            StarDraw(stars[i], pRenderer);
        }

        // Render
        SDL_RenderPresent(pRenderer);
        SDL_Delay((1.0 / FPS) * 1000);
    }

    // SDL cleanup
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