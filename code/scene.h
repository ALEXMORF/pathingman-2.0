#pragma once

#define TRIANGLE_DOUBLE_FACE 1

struct plane
{
    v3 P;
    v3 N;
    int MatIndex;
};

struct sphere
{
    v3 P;
    f32 R;
    int MatIndex;
};

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

int SampleCount = 0;
int NullMatIndex = 0;
static material *Mats;
static plane *Planes;
static sphere *Spheres;
static triangle *Triangles;
