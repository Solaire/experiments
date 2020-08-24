#include "common.h"

#include <stdlib.h>
#include <stdio.h>

/*
    Reverse bit order in a char
    Return new char as an unsigned int
*/
unsigned int ReverseBits(unsigned int value)
{
    unsigned int count = (INT_BITS - 1);
    unsigned int tmp   = value;
    value >>= 1; // Shift because LSB alreadt assigned to tmp

    while(value)
    {
        tmp <<= 1; // Already have the LSB. Shift
        tmp |= value & 1; // Putting the set of bits to num
        value >>= 1;
        count--;
    }
    tmp << count; // When value is 0, shift temp from remaining counts
    return tmp;
}

/*
    Convert a hexadecimal string to an unsigned int value
    length = stores the length, in bits, of the input
*/
unsigned int Hex2Int(char * hex, unsigned int * length)
{
    int value   = 0;
    int i       = 0;
    (*length)   = 0;
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
        value = (value << 4) | (byte & 0xF);
        (*length)++;
    }
    (*length) *= 4; // 1 Hex char is 4 bits
    return ReverseBits(value);
}

/*
    Resize array by reallocating memory
    If successful, the function will return TRUE
    If failure, function will free the array memory and return FALSE
    ppArray     = double pointer to the array
    sizeArray   = desired size of the array (number of elements)
    sizeType    = size of the type of the array
*/
BOOL ResizeArray(void ** ppArray, unsigned int sizeArray, unsigned int sizeType)
{
    void * resized = realloc(*ppArray, sizeArray * sizeType);
    if(resized)
    {
        *ppArray = resized;
        return TRUE;
    }
    free(*ppArray);
    return FALSE;
}

/*
    2D array access function. Return value of specified cell
    x = column (ie. how much to the right)
    y = row (ie. how far down to go)

    https://www.geeksforgeeks.org/pass-2d-array-parameter-c/
    https://dyclassroom.com/c/c-pointers-and-two-dimensional-array
*/
unsigned int GetCell(Map * map, int x, int y)
{
    // height is the offset for accessing 2D array
    return *((map->data + x * map->height) + y);
}

/*
    2D array access function. Set the value of specified cell
    x = column (ie. how much to the right)
    y = row (ie. how far down to go)
*/
void SetCell(Map * map, int x, int y, unsigned int value)
{
    // height is the offset for accessing 2D array
    *((map->data + x * map->height) + y) = value;
}

/*
    Create and return new Map object
    Set w and h and set all cells to the specified value
*/
void Init(Map * map, int rows, int cols, int value)
{
    map->width  = rows;
    map->height = cols;
    map->data = (unsigned int *)malloc(rows * cols * sizeof(unsigned int));
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
    map->width  = 0;
    map->height = 0;
    free(map->data);
}

/*
    Generate a random starting and ending point on the top and bottom of the map
    Function will ensure that the points have access to the rest of the map (or at least to another free cell)
    If cannot find a suitable point (e.g. all cells are non-free), the function will timeout after 100 loop iterations
*/
BOOL GenerateStartEndPoints(Map * map)
{
    int min     = 1;
    int max     = map->width - 2;
    int counter = 0;
    do
    {
        if(counter++ > 100) // Timeout
        {
            return FALSE;
        }
        map->start  = (unsigned int)(rand() % (max + 1 - min) + min);
        map->end    = (unsigned int)(rand() % (max + 1 - min) + min);
    } while( GetCell(map, map->start, min) || GetCell(map, map->end, max) );
    SetCell(map, map->start, 0, 0);
    SetCell(map, map->end, map->width - 1, 0);

    return TRUE;
}

/*
    Add Row data to the map
    data = cell data for the entire row, as a long
*/
void AddRowData(Map * map, unsigned long data, int row)
{
    for(int i = 0; i < map->width; i++)
    {
        SetCell(map, i, row, (data & 0x01));
        data >>= 1;
    }
}

/*
    Load map from file and extract the data into the map array
*/
BOOL LoadMap(Map * map, char * filename)
{
    int rows = 0;
    int cols = 0;
    FILE * file;
    if((file = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr, "openerror for file, errno = %d\n", errno);
        return FALSE;
    }

    fscanf(file, "%d, %d\n", &rows, &cols);
    Init(map, rows, cols, FREE);

    char str[128];
    int row = 0;
    while(fgets(str, 128, file) != NULL)
    {
        unsigned int l = 0;
        unsigned int data = Hex2Int(str, &l);
        AddRowData(map, data, row++);
    }
    GenerateStartEndPoints(map);
    fclose(file);
    return (map->width | map->height > 0);
}

/*
    Save map to file
*/
BOOL SaveMap(Map * map, char * filename)
{
    FILE * file;
    if((file = fopen(filename, "w")) == NULL)
    {
        fprintf(stderr, "openerror for file, errno = %d\n", errno);
        return FALSE;
    }
    fprintf(file, "%d, %d\n", map->width, map->height); // Write width + height
    for(int i = 0; i < map->height; i++)
    {
        unsigned long row = 0;
        for(int ii = 0; ii < map->width; ii++)
        {
            char value = (char)Map_namespace.GetCell(map, ii, i);
            row = (row << 1) | (value & 0x1);
        }
        fprintf(file, "%x\n", row);
    }
    fclose(file);
    return TRUE;
}

// Map namespace struct, contains function pointers related to the Map struct
const struct map_namespace Map_namespace = 
{
    .Init = Init,
    .Destroy = Destroy,
    .GenerateStartEndPoints = GenerateStartEndPoints,
    .AddRowData = AddRowData,
    .LoadMap = LoadMap,
    .SaveMap = SaveMap,
    .GetCell = GetCell,
    .SetCell = SetCell
};