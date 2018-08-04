/*

 TODO:
 
 . have a fallback split method after SAH
. is middle index in SAH over by 1? investigate
. linearize BVH
. exploit MinT in BVH traversal
. faster custom rand() 
. render config (window size, sample count)

*/

#include "pathingman.h"
#include "asset.cpp"
#include "scene.cpp"

f32 Random()
{
    return (f32)rand() / (f32)RAND_MAX;
}

f32 RandomBilateral()
{
    return 2.0f * Random() - 1.0f;
}

u32 PackV3IntoRGB(v3 Value)
{
    u8 R = (u8)(Value.R * 255.0f);
    u8 G = (u8)(Value.G * 255.0f);
    u8 B = (u8)(Value.B * 255.0f);
    u32 PackedValue = (R << 16) | (G << 8) | B;
    return PackedValue;
}

v3 CosineDistrib(v3 N)
{
    v3 LocalX;
    if (Abs(N.Y) < 0.99f)
    {
        LocalX = Normalize(Cross(YAxis(), N));
    }
    else
    {
        LocalX = Normalize(Cross(N, XAxis()));
    }
    v3 LocalY = Normalize(Cross(N, LocalX));
    
    f32 Theta = 2.0f*Pi32 * Random();
    f32 R = sqrtf(Random());
    v3 SpherePoint = R * (LocalX * cosf(Theta) + LocalY * sinf(Theta)) + N * sqrtf(1.0f - R*R);  
    return SpherePoint;
}

inline bool
IntersectAABB(v3 Ro, v3 Rd, aabb Bound)
{
    f32 MinT = -F32Max;
    f32 MaxT = F32Max;
    
    if (Rd.X != 0.0f)
    {
        f32 T0X = (Bound.Min.X - Ro.X) / Rd.X;
        f32 T1X = (Bound.Max.X - Ro.X) / Rd.X;
        
        if (T0X > T1X) 
        {
            std::swap(T0X, T1X);
        }
        
        MinT = Max(T0X, MinT);
        MaxT = Min(T1X, MaxT);
    }
    
    if (Rd.Y != 0.0f)
    {
        f32 T0Y = (Bound.Min.Y - Ro.Y) / Rd.Y;
        f32 T1Y = (Bound.Max.Y - Ro.Y) / Rd.Y;
        
        if (T0Y > T1Y) 
        {
            std::swap(T0Y, T1Y);
        }
        
        MinT = Max(T0Y, MinT);
        MaxT = Min(T1Y, MaxT);
    }
    
    if (Rd.Z != 0.0f)
    {
        f32 T0Z = (Bound.Min.Z - Ro.Z) / Rd.Z;
        f32 T1Z = (Bound.Max.Z - Ro.Z) / Rd.Z;
        
        if (T0Z > T1Z) 
        {
            std::swap(T0Z, T1Z);
        }
        
        MinT = Max(T0Z, MinT);
        MaxT = Min(T1Z, MaxT);
    }
    
    return MaxT >= MinT;
}

inline f32
IntersectTriangle(v3 Ro, v3 Rd, triangle *Tri, f32 MinT, int *MatIndex, v3 *NextN)
{
    f32 Tolerance = 0.00001f;
    
    v3 E02 = Tri->E[2] - Tri->E[0];
    v3 E12 = Tri->E[2] - Tri->E[1];
    v3 ERo2 = Tri->E[2] - Ro;
    
    v3 Term2 = Cross(Rd, E02);
    
    f32 Denom = Dot(Term2, E12);
    if (Denom != 0.0f)
    {
        v3 Term1 = Cross(ERo2, E12);
        f32 U = Dot(Term1, Rd) / Denom;
        f32 V = Dot(Term2, ERo2) / Denom;
        
        if (U >= 0.0f && V >= 0.0f && U + V < 1.0f) 
        {
            f32 T = Dot(-Term1, E02) / Denom;
            if (T > Tolerance && T < MinT)
            {
                MinT = T;
                *MatIndex = Tri->MatIndex;
                
                *NextN = Tri->N;
#if TRIANGLE_DOUBLE_FACE
                if (Dot(*NextN, Rd) > 0.0f)
                {
                    *NextN = -*NextN;
                }
#endif
            }
        }
    }
    
    return MinT;
}

f32 
IntersectBvh(bvh_node *Node, v3 Ro, v3 Rd, f32 MinT, int *MatIndex, v3 *NextN)
{
    if (!Node)
    {
        return MinT;
    }
    
    //TODO(chen): test ray against node->bound
    if (IntersectAABB(Ro, Rd, Node->Bound))
    {
        if (Node->Left || Node->Right) // is intermediate node
        {
            MinT = IntersectBvh(Node->Left, Ro, Rd, MinT, MatIndex, NextN);
            MinT = IntersectBvh(Node->Right, Ro, Rd, MinT, MatIndex, NextN);
            return MinT;
        }
        else // is leaf node
        {
            for (int TriIndex = 0; TriIndex < Node->PrimitiveCount; ++TriIndex)
            {
                MinT = IntersectTriangle(Ro, Rd, 
                                         Node->Primitives + TriIndex, 
                                         MinT, MatIndex, NextN);
            }
            return MinT;
        }
    }
    
    return MinT;
}

inline v3 
TracePathRadiance(v3 Ro, v3 Rd)
{
    f32 Tolerance = 0.00001f;
    
    v3 Radiance = {};
    v3 Atten = V3(1);
    for (int BounceIndex = 0; BounceIndex < 8; ++BounceIndex)
    {
        int MatIndex = Scene.NullMatIndex;
        f32 MinT = F32Max;
        v3 NextN = {};
        
#if 1
        MinT = IntersectBvh(Scene.Root, Ro, Rd, MinT, &MatIndex, &NextN);
#else
        for (int TriangleIndex = 0; TriangleIndex < BufLen(Scene.Triangles); ++TriangleIndex)
        {
            MinT = IntersectTriangle(Ro, Rd, 
                                     Scene.Triangles + TriangleIndex, 
                                     MinT, &MatIndex, &NextN);
        }
#endif
        
        for (int PlaneIndex = 0; PlaneIndex < BufLen(Scene.Planes); ++PlaneIndex)
        {
            plane Plane = Scene.Planes[PlaneIndex];
            
            f32 Denom = Dot(Rd, Plane.N);
            if (Denom != 0)
            {
                f32 T = -Dot(Ro - Plane.P, Plane.N) / Denom;
                if (T > Tolerance && T < MinT)
                {
                    MinT = T;
                    MatIndex = Plane.MatIndex;
                    NextN = Plane.N;
                }
            }
        }
        
        for (int SphereIndex = 0; SphereIndex < BufLen(Scene.Spheres); ++SphereIndex)
        {
            sphere Sphere = Scene.Spheres[SphereIndex];
            
            f32 A = Dot(Rd, Rd);
            f32 B = 2.0f * Dot(Ro - Sphere.P, Rd);
            f32 C = Dot(Ro - Sphere.P, Ro - Sphere.P) - Sphere.R * Sphere.R;
            
            f32 SquareRootTerm = B*B - 4.0f*A*C;
            if (A != 0.0f && SquareRootTerm >= 0.0f)
            {
                f32 T1 = (-B - sqrtf(SquareRootTerm)) / (2.0f * A);
                f32 T2 = (-B + sqrtf(SquareRootTerm)) / (2.0f * A);
                
                f32 T = Min(T1, T2);
                if (T > Tolerance && T < MinT)
                {
                    MinT = T;
                    MatIndex = Sphere.MatIndex;
                    NextN = Normalize((Ro + T*Rd) - Sphere.P);
                }
            }
        }
        
        Radiance += Atten * Scene.Mats[MatIndex].Emission;
        if (MatIndex != Scene.NullMatIndex)
        {
            Atten *= Scene.Mats[MatIndex].RefColor;
            
            Ro = Ro + MinT*Rd;
            Rd = CosineDistrib(NextN);
        }
        else
        {
            break;
        }
    }
    
    return Radiance;
}

void RenderTile(image_tile ImageTile)
{
    int ImageWidth = ImageTile.Image->Width;
    int ImageHeight = ImageTile.Image->Height;
    
    v3 CamZ = Normalize(Scene.CamLookAt - Scene.CamRo);
    v3 CamX = Normalize(Cross(YAxis(), CamZ));
    v3 CamY = Cross(CamZ, CamX);
    f32 AR = (f32)ImageWidth / (f32)ImageHeight;
    
    for (int Y = ImageTile.MinY; Y < ImageTile.MaxY; ++Y)
    {
        for (int X = ImageTile.MinX; X < ImageTile.MaxX; ++X)
        {
            f32 SampleContrib = 1.0f / (f32)Scene.SampleCount;
            v3 Radiance = {};
            for (int I = 0; I < Scene.SampleCount; ++I)
            {
                v2 UVJitter = V2(0.5f - Random(), 0.5f - Random());
                v2 UV = V2(((f32)X + 0.5f + UVJitter.X) / (f32)ImageWidth, 
                           ((f32)Y + 0.5f + UVJitter.Y) / (f32)ImageHeight);
                UV = 2.0f * UV - V2(1.0f);
                UV.X *= AR;
                v3 Rd = CamX * UV.X + CamY * UV.Y + 2.0f * CamZ;
                
                Radiance += SampleContrib * TracePathRadiance(Scene.CamRo, Rd);
            }
            
            Radiance = V3(1) - Exp(-1.0f*Radiance);
            Radiance = Power(Radiance, 1.0f / 2.2f);
            ImageTile.Image->Buffer[Y*ImageWidth + X] = PackV3IntoRGB(Radiance);
        }
    }
}

void WorkOnTiles(image_tile_queue *TileQueue)
{
    while (TileQueue->NextTile.load() < TileQueue->TileCount)
    {
        int TileIndex = TileQueue->NextTile++;
        
        image_tile Tile = TileQueue->Tiles[TileIndex];
        RenderTile(Tile);
        
        TileQueue->TilesCompleted++;
        printf("\rTiles done: %d/%d", TileQueue->TilesCompleted.load(), TileQueue->TileCount);
    }
}

void RunTests()
{
    int *Array = 0;
    BufPush(Array, 1);
    BufPush(Array, 2);
    BufPush(Array, 3);
    BufPush(Array, 4);
    
    int *Last = BufLast(Array);
    ASSERT(*Last == 4);
    ASSERT(BufLen(Array) == 4);
    
    BufFree(Array);
}

int main()
{
    RunTests();
    
    //image buffer init
    image Image = {};
    Image.Width = 1080;
    Image.Height = 742;
    Image.Buffer = (u32 *)malloc(sizeof(u32) * Image.Width * Image.Height);
    
    //system init
    srand((unsigned int)time(0));
    int CoreCount = std::thread::hardware_concurrency();
    if (CoreCount > 0)
    {
        printf("Detected %d cores\n", CoreCount);
    }
    else
    {
        CoreCount = 1;
        printf("Unable to detect number of cores, assuming 1 core\n");
    }
    
    clock_t scene_t = clock();
    printf("\ninitializing scene ...\n");
    
    InitScene();
    printf("total triangle count: %d\n", BufLen(Scene.Triangles));
    
    scene_t = clock() - scene_t;
    printf("scene init took %.2f seconds\n", (f32)scene_t / (f32)CLOCKS_PER_SEC);
    
    clock_t render_t = clock();
    printf("\nrendering scene (%d spp) ...\n", Scene.SampleCount);
    
    int TileWidth = 64;
    int TileHeight = 64;
    int TileCountInX = (Image.Width - 1) / TileWidth + 1;
    int TileCountInY = (Image.Height - 1) / TileHeight + 1;
    int TileCount = TileCountInX * TileCountInY;
    
    image_tile *Tiles = (image_tile *)calloc(TileCount, sizeof(image_tile));
    for (int TileY = 0; TileY < TileCountInY; ++TileY)
    {
        for (int TileX = 0; TileX < TileCountInX; ++TileX)
        {
            image_tile *Tile = Tiles + (TileX + TileCountInX * TileY);
            Tile->Image = &Image;
            
            Tile->MinX = TileX * TileWidth;
            Tile->MinY = TileY * TileHeight;
            Tile->MaxX = Min(Tile->MinX + TileWidth, Image.Width);
            Tile->MaxY = Min(Tile->MinY + TileHeight, Image.Height);
        }
    }
    
    image_tile_queue TileQueue = {Tiles, TileCount, 0, 0};
    
    std::thread *WorkerThreads = (std::thread *)calloc(CoreCount, sizeof(std::thread));
    for (int ThreadIndex = 0; ThreadIndex < CoreCount; ++ThreadIndex)
    {
        WorkerThreads[ThreadIndex] = std::thread(WorkOnTiles, &TileQueue);
    }
    for (int ThreadIndex = 0; ThreadIndex < CoreCount; ++ThreadIndex)
    {
        WorkerThreads[ThreadIndex].join();
    }
    
    render_t = clock() - render_t;
    f32 ElapsedTimeInS = (f32)render_t / (f32)CLOCKS_PER_SEC;
    f32 ElapsedSeconds = fmodf(ElapsedTimeInS, 60.0f);
    int ElapsedMinutes = Floor(ElapsedTimeInS / 60.0f);
    
    printf("\nscene rendered in ");
    if (ElapsedMinutes != 0.0f)
    {
        printf("%d minute%s, ", ElapsedMinutes, ElapsedMinutes > 1? "s": "");
    }
    printf("%.2f seconds\n", ElapsedSeconds);
    
    CH_BMP::WriteImageToBMP("render.bmp", Image.Buffer, Image.Width, Image.Height);
    return 0;
}

