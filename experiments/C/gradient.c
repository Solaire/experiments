#include <SDL2/SDL.h>
#include <math.h>

//https://gist.github.com/mjackson/5311256
//https://www.alanzucconi.com/2015/09/30/colour-sorting/
//https://stackoverflow.com/questions/58811499/generating-gradient-colors-in-python
//https://www.alanzucconi.com/2016/01/06/colour-interpolation/
//https://stackoverflow.com/questions/13488957/interpolate-from-one-color-to-another
//https://github.com/bokub/gradient-string

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
#define FPS_COUNT       25
#define TRAIL_LENGTH    100
#define COLOUR_SIZE     4
#define TOTAL_COLOURS   WINDOW_WIDTH / COLOUR_SIZE

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

/*
RGB structure representing red, green and blue
*/
typedef struct RGB
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;

/*
HSL structure representing hue, saturation and lightness
*/
typedef struct HSL
{
    float h;
    float s;
    float l;
} HSL;

// Convert hex-formatted rgb colour to a RGB structure
void Hex2RGB(char * hex, struct RGB * pRGB)
{
    pRGB->r = 0;
    pRGB->g = 0;
    pRGB->b = 0;
    int i = 1;
    while(*hex)
    {
        unsigned char byte = *hex++;
        if(byte >= '0' && byte <= '9')
        {
            byte = byte - '0';
        }
        else if(byte >= 'a' && byte <= 'f')
        {
            byte = byte - 'a' + 10;
        }
        else if(byte >= 'A' && byte <= 'F')
        {
            byte = byte - 'A' + 10;   
        }
        else
        {
            continue;
        }
        switch(i++)
        {
        case 1:
        case 2:
            pRGB->r = (pRGB->r << 4) | (byte & 0xF);
            break;

        case 3:
        case 4:
            pRGB->g = (pRGB->g << 4) | (byte & 0xF);
            break;

        case 5:
        case 6:
            pRGB->b = (pRGB->b << 4) | (byte & 0xF);
            break;

        default:
            break;
        }
    }
}

// Convert RGB colour to HSL colour
void RGB2HSL(const struct RGB rgb, struct HSL * pHSL)
{
    float r = rgb.r / 255.f;
    float g = rgb.g / 255.f;
    float b = rgb.b / 255.f;

    float max = __max(r, __max(g, b));
    float min = __min(r, __min(g, b));

    float h = (max + min) / 2.f;
    float s = (max + min) / 2.f;
    float l = (max + min) / 2.f;

    if(max == min)
    {
        h = 0.f;
        s = 0.f;
    }
    else
    {
        float d = max - min;
        s = (l > 0.5f) ? d / (2.f - max - min) : d / (max + min);

        if(max == r)
        {
            h = (g - b) / d + (g < b ? 6.f : 0.f);
        }
        else if(max == g)
        {
            h = (b - r) / d + 2.f;
        }
        else if(max == b)
        {
            h = (r - g) / d + 4.f;
        }
        h /= 6.f;
    }
    pHSL->h = h;
    pHSL->s = s;
    pHSL->l = l;
}

// Convert hue to an rgb colour
float Hue2RGB(float p, float q, float t)
{
    t = (t < 0)   ? t + 1.f : t;
    t = (t > 1.f) ? t - 1.f : t;
    if(t < 1.f / 6.f)
    {
        return p + (q - p) * 6.f * t;
    }
    if(t < 1.f / 2.f)
    {
        return q;
    }
    if(t < 2.f / 3.f)
    {
        return p + (q - p) * (2.f / 3.f - t) * 6.f;
    }
    return p;
}

// convert colour from RGB to HSL
void HSL2RGB(const struct HSL hsl, struct RGB * pRGB)
{
    pRGB->r = 0;
    pRGB->g = 0;
    pRGB->b = 0;

    if(hsl.s == 0)
    {
        pRGB->r = 255;
        pRGB->g = 255;
        pRGB->b = 255;
    }
    else
    {
        float q = (hsl.l < 0.5f) ? hsl.l * (1.f + hsl.s) : hsl.l + hsl.s - hsl.l * hsl.s;
        float p = 2.f * hsl.l - q;

        pRGB->r = Hue2RGB(p, q, hsl.h + 1.f / 3.f)  * 255;
        pRGB->g = Hue2RGB(p, q, hsl.h)              * 255;
        pRGB->b = Hue2RGB(p, q, hsl.h - 1.f / 3.f)  * 255;
    }
}

// Map a value from one range to another
double MapValue(const float val, const float inStart, const float inEnd, const float outStart, const float outEnd)
{
    return (val - inStart) * (outEnd - outStart) / (inEnd - inStart) + outStart;
}

/*
Return interpolated HSL colour
a = first colour
b = second colour
step = current step in the interpolation, between 0 (a) and 1 (b)
*/
HSL InterpolateHSL(HSL a, HSL b, double step)
{
    float h = 0;
    float d = b.h - a.h;
    if(a.h > b.h)
    {
        // Swap
        float tmp = b.h;
        b.h = a.h;
        a.h = tmp;

        d *= -1.f;
        step = 1 - step;
    }
    if(d > 0.5f)
    {
        a.h = a.h + 1.f;
        h   = remainder(a.h + step * (b.h - a.h), 1.f); // % (mod) doesn't work with non-integrals
    }
    if(d <= 0.5f)
    {
        h = a.h + step * d;
    }

    HSL out;
    out.h = h;
    out.s = a.s + step * (b.s - a.s);
    out.l = a.l + step * (b.l - a.l);
    return out;
}

// Draw HSL colour to the renderer
void DrawColourHSL(const HSL hsl, const int x, const int w, SDL_Renderer * pRenderer)
{
    RGB tmp;
    HSL2RGB(hsl, &tmp);
    SDL_SetRenderDrawColor(pRenderer, tmp.r, tmp.g, tmp.b, SDL_ALPHA_OPAQUE);
    SDL_Rect rect;
    rect.w = w;
    rect.h = WINDOW_HEIGHT;
    rect.x = x;
    rect.y = 0;
    SDL_RenderFillRect(pRenderer, &rect);
}

int main(int argc, char *argv[])
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

    // Convert the argument colours:
    // hex -> rgb
    // rgb -> hsl
    // HSL is easier to make a gradient
    struct HSL inHSL[argc - 1];
    for(int i = 1; i < argc; i++)
    {
        RGB tmp;
        Hex2RGB(argv[i], &tmp);
        RGB2HSL(tmp, &inHSL[i - 1]);
    }

    int quit = ARRAY_SIZE(inHSL) < 1;
    while(!quit)
    {
        // Clear screen and set renderer colour to white
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(pRenderer);

        // Basic event loop to make the program responsive
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0)
        {
            if(e.type == SDL_QUIT)
            {
                quit = 1;
            }
        }

        // Calculate the gradient between each stop and draw it.
        // Original colours are not drawn
        int x = 0;
        int w = COLOUR_SIZE;
        for(int i = 1; i < ARRAY_SIZE(inHSL); i++)
        {
            int steps = TOTAL_COLOURS / (ARRAY_SIZE(inHSL) - 1);
            for(int ii = 0; ii < steps; ii++)
            {
                double mappedStep = MapValue(ii, 0.f, (float)steps, 0.f, 1.f);
                HSL tmp = InterpolateHSL(inHSL[i - 1], inHSL[i], mappedStep);
                DrawColourHSL(tmp, x, w, pRenderer);
                x += w;
            }
        }
        // Render
        SDL_RenderPresent(pRenderer);
        SDL_Delay((1.0 / 1) * 1000);
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