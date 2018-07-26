#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <thread>
#include <atomic>
#include "ch_bmp.h"
#include "ch_math.h"
#include "pathingman_math.h"
#include "pathingman_utils.h"
#include "scene.h"

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