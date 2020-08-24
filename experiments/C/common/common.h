#ifndef COMMON_H
#define COMMON_H

#define INT_BITS (sizeof(int) * 8)

typedef char BOOL;
#define TRUE 1
#define FALSE 0

#define FREE 0
#define WALL 1
#define PATH 2

unsigned int ReverseBits(unsigned int value);
unsigned int Hex2Int(char * hex, unsigned int * length);
BOOL ResizeArray(void ** ppArray, unsigned int sizeArray, unsigned int sizeType);

typedef struct Map
{
    int width;
    int height;
    int start;  // Starting x coordinate in the first row
    int end;    // End x coordinate in the last row
    unsigned int * data;
} Map;

typedef struct map_namespace
{    
    void (* Init)(Map * map, int rows, int cols, int value);
    void (* Destroy)(Map * map);
    BOOL (* GenerateStartEndPoints)(Map * map);
    void (* AddRowData)(Map * map, unsigned long data, int row);
    BOOL (* LoadMap)(Map * map, char * filename);
    BOOL (* SaveMap)(Map * map, char * filename);
    unsigned int (* GetCell)(Map * map, int x, int y);
    void (* SetCell)(Map * map, int x, int y, unsigned int value);
} map_namespace;

extern const struct map_namespace Map_namespace;

#endif // COMMON_H