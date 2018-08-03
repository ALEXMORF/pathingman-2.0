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

struct bvh_node
{
    aabb Bound;
    bvh_node *Left;
    bvh_node *Right;
    
    triangle *Primitives;
    int PrimitiveCount;
};

struct scene
{
    int SampleCount;
    int NullMatIndex;
    
    v3 CamRo;
    v3 CamLookAt;
    
    material *Mats;
    plane *Planes;
    sphere *Spheres;
    triangle *Triangles;
    
    bvh_node *Root;
};

typedef int triangle_compare(const void *A, const void *B);
triangle_compare *TriangleCompares[3];

static asset_map Assets;
static scene Scene;
