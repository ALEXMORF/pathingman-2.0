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

struct bucket_info
{
    f32 Cost;
    int MiddleIndex;
};

struct bvh_node
{
    aabb Bound;
    bvh_node *Left;
    bvh_node *Right;
    
    int PrimitiveOffset;
    int PrimitiveCount;
};

struct bvh_linear_node
{
    aabb Bound;
    union
    {
        i32 PrimitiveOffset;
        i32 SecondChildOffset;
    };
    
    u16 PrimitiveCount;
    u8 IsLeafNode;
    u8 Pad[1]; //ensures 32 bytes
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
    bvh_linear_node *LinearNodes;
};

typedef int triangle_compare(const void *A, const void *B);
triangle_compare *TriangleCompares[3];

static asset_map Assets;
static scene Scene;
