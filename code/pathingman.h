#pragma once
#include <atomic>

struct image
{
    u32 *Buffer;
    int Width;
    int Height; 
};

struct image_tile
{
    image *Image;
    int MinX; 
    int MinY;
    int MaxX;
    int MaxY;
};

struct image_tile_queue
{
    image_tile *Tiles;
    
    int TileCount;
    volatile std::atomic<int> TilesCompleted;
    volatile std::atomic<int> NextTile;
};