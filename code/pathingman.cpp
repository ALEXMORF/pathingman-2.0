/*

 TODO:
 
. change scene.h to use stretchy buffer
. OBJ loading
. hotload config (window size, sample count)
 . scene file config (hotload)
. BVH

*/

#include "pathingman.h"
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

inline v3 
TracePathRadiance(v3 Ro, v3 Rd)
{
    f32 Tolerance = 0.00001f;
    
    v3 Radiance = {};
    v3 Atten = V3(1);
    for (int BounceIndex = 0; BounceIndex < 8; ++BounceIndex)
    {
        int MatIndex = NullMatIndex;
        f32 MinT = F32Max;
        v3 NextN = {};
        
        for (int TriangleIndex = 0; TriangleIndex < BufLen(Triangles); ++TriangleIndex)
        {
            triangle Tri = Triangles[TriangleIndex];
            
            v3 E02 = Tri.E[2] - Tri.E[0];
            v3 E12 = Tri.E[2] - Tri.E[1];
            v3 ERo2 = Tri.E[2] - Ro;
            
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
                        MatIndex = Tri.MatIndex;
                        
                        NextN = Tri.N;
#if TRIANGLE_DOUBLE_FACE
                        if (Dot(NextN, Rd) > 0.0f)
                        {
                            NextN = -NextN;
                        }
#endif
                    }
                }
            }
        }
        
        for (int PlaneIndex = 0; PlaneIndex < BufLen(Planes); ++PlaneIndex)
        {
            plane Plane = Planes[PlaneIndex];
            
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
        
        for (int SphereIndex = 0; SphereIndex < BufLen(Spheres); ++SphereIndex)
        {
            sphere Sphere = Spheres[SphereIndex];
            
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
        
        Radiance += Atten * Mats[MatIndex].Emission;
        if (MatIndex != NullMatIndex)
        {
            Atten *= Mats[MatIndex].RefColor;
            
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
    
    v3 At = {0, 1, 0};
    v3 Ro = {0, 2.0f, -3.0f};
    v3 CamZ = Normalize(At - Ro);
    v3 CamX = Normalize(Cross(YAxis(), CamZ));
    v3 CamY = Cross(CamZ, CamX);
    f32 AR = (f32)ImageWidth / (f32)ImageHeight;
    
    for (int Y = ImageTile.MinY; Y < ImageTile.MaxY; ++Y)
    {
        for (int X = ImageTile.MinX; X < ImageTile.MaxX; ++X)
        {
            v2 UV = {(f32)X / (f32)ImageWidth, (f32)Y / (f32)ImageHeight};
            UV = 2.0f * UV - V2(1.0f);
            UV.X *= AR;
            v3 Rd = CamX * UV.X + CamY * UV.Y + 2.0f * CamZ;
            
            f32 SampleContrib = 1.0f / (f32)SampleCount;
            v3 Radiance = {};
            for (int I = 0; I < SampleCount; ++I)
            {
                Radiance += SampleContrib * TracePathRadiance(Ro, Rd);
            }
            
            Radiance = V3(1) - Exp(-2.0f*Radiance);
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
}

int main()
{
    RunTests();
    
    //image buffer init
    image Image = {};
    Image.Width = 1080;
    Image.Height = 724;
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
    
    InitScene();
    
    clock_t t = clock();
    
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
    
    t = clock() - t;
    printf("\nRender took %f seconds\n", (f32)t / (f32)CLOCKS_PER_SEC);
    
    CH_BMP::WriteImageToBMP("render.bmp", Image.Buffer, Image.Width, Image.Height);
    return 0;
}

