#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800
#define FPS           30
#define SHADE_COUNT   16

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/*
RGB colour structure
*/
typedef struct Colour
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Colour; 

/*
Node structure containing the value and a flag indicating if the node was already matched in the cycle
*/
typedef struct Node
{
	int value;
	int matched;
} Node;

/*
Structure containing two int values
*/
typedef struct Tuple
{
	int a;
	int b;
} Tuple;

// Return colour based on the int value
Colour SetColour(const int i)
{
	Colour out;
	switch(i)
	{
		case 0:
			out.r = 255;
			out.g = 255;
			out.b = 255;
			break;
		
		case 1:
			out.r = 255;
			out.g = 0;
			out.b = 0;
			break;

		case 2:
			out.r = 255;
			out.g = 127;
			out.b = 0;
			break;

		case 3:
			out.r = 255;
			out.g = 255;
			out.b = 0;
			break;

		case 4:
			out.r = 0;
			out.g = 255;
			out.b = 0;
			break;

		case 5:
			out.r = 0;
			out.g = 255;
			out.b = 255;
			break;

		case 6:
			out.r = 0;
			out.g = 0;
			out.b = 255;
			break;

		case 7:
			out.r = 75;
			out.g = 0;
			out.b = 130;
			break;

		default:
			out.r = 148;
			out.g = 0;
			out.b = 221;
			break;
	}
	return out;
}

Node nodes[4][4];
const unsigned int randValues[2] = {1, 2};

/*
Move nodes up, starting from the bottom nodes, working upwards
Returns a flag indicating of any nodes were moved
*/
int MoveUp(void)
{
	int moved = 0;
	for(int i = 0; i < 4; i++)
	{
		for(int ii = 3, done = 0; ii > 0 & !done; ii--)
		{
			if(!nodes[ii][i].value) // Empty. Ignore
			{
				continue;
			}
			if(!nodes[ii - 1][i].value) // The node above is empty, we can move
			{
				nodes[ii - 1][i] = nodes[ii][i];
				nodes[ii][i].value = 0;
				moved = 1;
				ii = 4;
				continue;
			}
			if(nodes[ii - 1][i].value == nodes[ii][i].value 
			&& (nodes[ii - 1][i].matched + nodes[ii][i].matched) == 0) // Current node and node above the same and were not matched yet. Match
			{
				nodes[ii - 1][i].value += 1;
				nodes[ii - 1][i].matched = 1;
				nodes[ii][i].value = 0;
				moved = 1;
				ii = 4; // Start from end of the column
				continue;
			}
			done = (ii == 1);
		}
	}
	return moved;
}

/*
Move nodes down, starting from the top nodes, working downwards
Returns a flag indicating of any nodes were moved
*/
int MoveDown(void)
{
	int moved = 0;
	for(int i = 0; i < 4; i++)
	{
		for(int ii = 0, done = 0; ii < 3 && !done; ii++)
		{
			if(!nodes[ii][i].value) // Empty. Ignore
			{
				continue;
			}
			if(!nodes[ii + 1][i].value) // The node above is empty, we can move
			{
				nodes[ii + 1][i] = nodes[ii][i];
				nodes[ii][i].value = 0;
				moved = 1;
				ii = -1;
				continue;
			}
			if(nodes[ii + 1][i].value == nodes[ii][i].value 
			&& (nodes[ii + 1][i].matched + nodes[ii][i].matched) == 0) // Current node and node above the same and were not matched yet. Match
			{
				nodes[ii + 1][i].value += 1;
				nodes[ii + 1][i].matched = 1;
				nodes[ii][i].value = 0;
				moved = 1;
				ii = -1; // Start beginning of the column
				continue;
			}
			done = (ii == 2);
		}
	}
	return moved;
}

/*
Move nodes to the left, starting from the rightmost nodes, working leftwards
Returns a flag indicating of any nodes were moved
*/
int MoveLeft(void)
{
	int moved = 0;
	for(int i = 0; i < 4; i++)
	{
		for(int ii = 3, done = 0; ii > 0 && !done; ii--)
		{
			if(!nodes[i][ii].value) // Empty. Ignore
			{
				continue;
			}
			if(!nodes[i][ii - 1].value) // The node above is empty, we can move
			{
				nodes[i][ii - 1] = nodes[i][ii];
				nodes[i][ii].value = 0;
				moved = 1;
				ii = 4;
				continue;
			}
			if(nodes[i][ii].value == nodes[i][ii - 1].value
			&& (nodes[i][ii].matched + nodes[i][ii - 1].matched) == 0) // Current node and node above the same and were not matched yet. Match
			{
				nodes[i][ii - 1].value += 1;
				nodes[i][ii - 1].matched = 1;
				nodes[i][ii].value = 0;
				moved = 1;
				ii = 4; // Start from the end of the row
				continue;
			}
			done = (ii == 1);
		}
	}
	return moved;
}

/*
Move nodes to the right, starting from the leftmost nodes, working rightwards
Returns a flag indicating of any nodes were moved
*/
int MoveRight(void)
{
	int moved = 0;
	for(int i = 0; i < 4; i++)
	{
		for(int ii = 0, done = 0; ii < 3 && !done; ii++)
		{
			if(!nodes[i][ii].value) // Empty. Ignore
			{
				continue;
			}
			if(!nodes[i][ii + 1].value) // The node above is empty, we can move
			{
				nodes[i][ii + 1] = nodes[i][ii];
				nodes[i][ii].value = 0;
				moved = 1;
				ii = -1;
				continue;
			}
			if(nodes[i][ii].value == nodes[i][ii + 1].value
			&& (nodes[i][ii].matched + nodes[i][ii + 1].matched) == 0) // Current node and node above the same and were not matched yet. Match
			{
				nodes[i][ii + 1].value += 1;
				nodes[i][ii + 1].matched = 1;
				nodes[i][ii].value = 0;
				moved = 1;
				ii = -1; // Start from the beginning of the row
				continue;
			}
			done = (ii == 2);
		}
	}
	return moved;
}

/*
Insert a sqaure with a random value (between 1 and 2) to the grid
*/
void AddRandomSquare(void)
{
	// Get locations of empty squares only
	int count = 0;
	Tuple empties[16];
	for(int i = 0; i < 4; i++)
	{
		for(int ii = 0; ii < 4; ii++)
		{
			if(nodes[i][ii].value == 0)
			{
				empties[count].a = i;
				empties[count++].b = ii;
			}
		}
	}

	if(count == 0) // No empty spaces
	{
		return;
	}
	int r = rand() % count;
	nodes[empties[r].a][empties[r].b].value = randValues[rand() % ARRAY_SIZE(randValues)];
}

int main(int argc, char * argv[])
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
	if(!pSurface)
	{
		return -2;
	}

	time_t t;
	srand((unsigned) time(&t));

	// Prepare the game
	for(int i = 0; i < 4; i++)
	{
		nodes[i][0].value = 0;
		nodes[i][1].value = 0;
		nodes[i][2].value = 0;
		nodes[i][3].value = 0;
	}
	AddRandomSquare();

	int quit  = 0;
	int moved = 0;
	while(!quit)
	{
		moved = 0;
		for(int i = 0; i < 4; i++)
		{
			nodes[i][0].matched = 0;
			nodes[i][1].matched = 0;
			nodes[i][2].matched = 0;
			nodes[i][3].matched = 0;
		}

		// Clear screen
		SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(pRenderer);
		SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

		// Poll events
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_QUIT)
			{
				quit = 1;
			}
			else if(e.type == SDL_KEYUP)
			{
				switch(e.key.keysym.sym)
				{
					case SDLK_UP:
						moved = MoveUp();
						break;

					case SDLK_DOWN:
						moved = MoveDown();
						break;

					case SDLK_LEFT:
						moved = MoveLeft();
						break;

					case SDLK_RIGHT:
						moved = MoveRight();
						break;

					case SDLK_ESCAPE:
						quit = 1;

					default:
						// No op
						break;
				}
			}
		}
		if(moved)
		{
			AddRandomSquare();
		}

		// Drawing
		for(int i = 0, y = 100; i < 4; i++, y += 100)
		{
			for(int ii = 0, x = 100; ii < 4; ii++, x += 100)
			{
				Colour colour = SetColour(nodes[i][ii].value);
				SDL_SetRenderDrawColor(pRenderer, colour.r, colour.g, colour.b, SDL_ALPHA_OPAQUE);

				SDL_Rect rect;
				rect.w = 75;
				rect.h = 75;
				rect.x = x;
				rect.y = y;
				SDL_RenderFillRect(pRenderer, &rect);
			}
		}

		// Render
		SDL_RenderPresent(pRenderer);
		SDL_Delay((1.0 / FPS) * 1000);
	}

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
