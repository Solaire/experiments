#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "./common/common.h"

#include <SDL2/SDL.h>

#define mapWidth 24
#define mapHeight 24
#define screenWidth 1920
#define screenHeight 1080

//The parameters in the main function cannot be omitted, or an error will be reported
int main(int arg, char *argv[])
{
    SDL_Window   * pWindow   = NULL;
    SDL_Surface  * pSurface  = NULL;
    SDL_Renderer * pRenderer = NULL;

    // SDL Setup
    if(SDL_CreateWindowAndRenderer(screenWidth, screenHeight, SDL_WINDOW_SHOWN, &pWindow, &pRenderer) != 0)
    {
        return -1;
    }
    pSurface = SDL_GetWindowSurface(pWindow);
    if (!pSurface)
    {
        return -2;
    }

    Map map;
    int quit = !Map_namespace.LoadMap(&map, "map.txt");
    int showMaze = 0;

    // Raycaster setup
    double posX = map.start, posY = 0;
    double dirX = -1, dirY = 0;
    double planeX = 0, planeY = 0.66;

    double time = 0;
    double oldTime = 0;

    int w = screenWidth;
    int h = screenHeight;
    while(!quit)
    {
        // Clear screen and set renderer colour to white
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(pRenderer);

        for(int x = 0; x < w; x++)
        {
            double cameraX = 2 * x / (double)w - 1;
            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;

            int mapX = (int)posX;
            int mapY = (int)posY;

            double sideDistX;
            double sideDistY;

            double deltaDistX = fabs(1.0 / rayDirX);
            double deltaDistY = fabs(1.0 / rayDirY);
            double perpWallDist;

            int stepX;
            int stepY;

            int hit = 0;
            int side;

            if(rayDirX < 0)
            {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            }
            else
            {
                stepX = 1;
                sideDistX = (mapX + 1.0 - posX) * deltaDistX;
            }

            if(rayDirY < 0)
            {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            }
            else
            {
                stepY = 1;
                sideDistY = (mapY + 1.0 - posY) * deltaDistY;
            }

            while(!hit)
            {
                if(sideDistX < sideDistY)
                {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                }
                else
                {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }

                hit = Map_namespace.GetCell(&map, mapX, mapY);
            }

            perpWallDist = (side == 0) 
                ? (mapX - posX + (1 - stepX) / 2) / rayDirX 
                : (mapY - posY + (1 - stepY) / 2) / rayDirY;

            int lineHeight = (int)(h / perpWallDist);

            int drawStart = -lineHeight / 2 + h / 2;
            if(drawStart < 0)
            {
                drawStart = 0;
            }
            int drawEnd = lineHeight / 2 + h / 2;
            if(drawEnd >= h)
            {
                drawEnd = h - 1;
            }

            SDL_SetRenderDrawColor(pRenderer, (side == 1) ? 128 : 255, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawLineF(pRenderer, x, drawStart, x, drawEnd);
        }

        // Draw maze
        if(showMaze)
        {
            int rectSize = 10;
            int x = (w / 2) - ((map.width / 2) * 10);
            for(int i = 0; i < map.width; i++, x += rectSize)
            {
                int y = (h / 2) - ((map.height / 2) * 10);
                for(int ii = 0; ii < map.height; ii++, y += rectSize)
                {
                    SDL_Rect rect;
                    rect.w = rectSize - 1;
                    rect.h = rectSize - 1;
                    rect.x = x;
                    rect.y = y;

                    if((int)posX == i && (int)posY == ii)
                    {
                        SDL_SetRenderDrawColor(pRenderer, 32, 180, 32, SDL_ALPHA_OPAQUE);
                    }
                    //else if( ARRAY_ACCESS(map.grid, i, map.h, ii) )
                    else if(Map_namespace.GetCell(&map, i, ii))
                    {
                        SDL_SetRenderDrawColor(pRenderer, 54, 34, 199, SDL_ALPHA_OPAQUE);
                    }
                    else
                    {
                        SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                    }

                    SDL_RenderFillRect(pRenderer, &rect);
                }
            }
        }

        oldTime = time;
        time = SDL_GetTicks();

        double frameTime = (time - oldTime) / 1000.0;
        // print fps counter

        double moveSpeed = frameTime * 2.5;
        double rotSpeed = frameTime * 3.0;

        // Poll events

        const Uint8 * keys = SDL_GetKeyboardState(NULL);
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_QUIT || keys[SDL_SCANCODE_ESCAPE])
			{
				quit = 1;
                break;
			}
            if(keys[SDL_SCANCODE_TAB])
            {
                showMaze = (showMaze + 1) % 2;
            }
		}

        // Movement forward backward
        if(keys[SDL_SCANCODE_W])
        {
            if(!Map_namespace.GetCell(&map, (int)(posX + dirX * moveSpeed), (int)posY))
            {
                posX += dirX * moveSpeed;
            }
            if(!Map_namespace.GetCell(&map, (int)posX, (int)(posY + dirY * moveSpeed)))
            {
                posY += dirY * moveSpeed;
            }
        }
        else if(keys[SDL_SCANCODE_S])
        {
            if(!Map_namespace.GetCell(&map, (int)(posX - dirX * moveSpeed), (int)posY))
            {
                posX -= dirX * moveSpeed;
            }
            if(!Map_namespace.GetCell(&map,(int)posX, (int)(posY - dirY * moveSpeed)))
            {
                posY -= dirY * moveSpeed;
            }
        }

        // Movement sideways
        if(keys[SDL_SCANCODE_A])
        {
            if(!Map_namespace.GetCell(&map, (int)(posX + (dirY / 2) * moveSpeed), (int)posY))
            {
                posX += (dirY / 2) * moveSpeed;
            }
            if(!Map_namespace.GetCell(&map, (int)posX, (int)(posY + (dirX / 2) * moveSpeed)))
            {
                posY += (dirX / 2) * moveSpeed;
            }
        }
        else if(keys[SDL_SCANCODE_D])
        {
            if(!Map_namespace.GetCell(&map, (int)(posX - (dirY / 2) * moveSpeed), (int)posY))
            {
                posX -= (dirY / 2) * moveSpeed;
            }
            if(!Map_namespace.GetCell(&map,(int)posX, (int)(posY - (dirX / 2) * moveSpeed)))
            {
                posY -= (dirX / 2) * moveSpeed;
            }
        }

        // Rotation
        if(keys[SDL_SCANCODE_LEFT])
        {
            double oldDirX = dirX;
            dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
            dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
            planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        }
        else if(keys[SDL_SCANCODE_RIGHT])
        {
            double oldDirX = dirX;
            dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
            dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
            planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        }
        
        SDL_RenderPresent(pRenderer);
        SDL_Delay((1.0 / 30) * 1000);
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