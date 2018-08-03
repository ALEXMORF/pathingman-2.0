#pragma once

#include <stdio.h>
#include <string.h>

struct triangle
{
    v3 E[3];
    v3 N;
    int MatIndex;
};

struct material
{
    v3 RefColor;
    v3 Emission;
};

struct mesh
{
    triangle *E;
    char Name[255];
};

struct asset_map
{
    char *Dir;
    mesh *Meshes;
    
    mesh *operator[](char *Name);
};
