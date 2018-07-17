#pragma once

struct plane
{
    v3 N;
    v3 P;
    int MatIndex;
};

struct sphere
{
    v3 P;
    f32 R;
    int MatIndex;
};

struct material
{
    v3 Emission;
    v3 RefColor;
};

int NullMatIndex = 0;
material Mats[4] = {};
plane Planes[1] = {};
sphere Spheres[2] = {};

void InitScene()
{
    Mats[NullMatIndex].Emission = V3(0.001f);
    Mats[1].RefColor = V3(0.63f);
    Mats[2].RefColor = V3(0.92f, 0.92f, 0.92f);
    Mats[3].RefColor = V3(1);
    Mats[3].Emission = V3(3);
    
    Planes[0].N = YAxis();
    Planes[0].P = {};
    Planes[0].MatIndex = 1;
    
    Spheres[0].P = {0.0f, 1.0f, 2.0f};
    Spheres[0].R = 1.0f;
    Spheres[0].MatIndex = 2;
    
    Spheres[1].P = {2.5f, 3.0f, 0.0f};
    Spheres[1].R = 0.5f;
    Spheres[1].MatIndex = 3;
}