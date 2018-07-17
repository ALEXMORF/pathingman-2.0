#pragma once
#include <stdint.h>
#include <stdio.h>

namespace CH_BMP
{
    
#pragma pack(push, 1)
    struct bmp_file_header
    {
        //file header
        char bfType[2];
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffbits;
    };
    
    struct bmp_image_header
    {
        //image header
        uint32_t biSize;
        int32_t biWidth;
        int32_t biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        uint32_t biXPelsPerMeter;
        uint32_t biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };
#pragma pack(pop)
    
    static bool 
        WriteImageToBMP(const char *Path, uint32_t *Buffer, int Width, int Height)
    {
        bool Result = false;
        
        FILE *File = fopen(Path, "wb");
        if (File)
        {
            bmp_file_header BMPFileHeader = {};
            BMPFileHeader.bfType[0] = 'B';
            BMPFileHeader.bfType[1] = 'M';
            BMPFileHeader.bfOffbits = sizeof(bmp_file_header) + sizeof(bmp_image_header);
            
            bmp_image_header BMPImageHeader = {};
            BMPImageHeader.biSize = sizeof(BMPImageHeader);
            BMPImageHeader.biWidth = Width;
            BMPImageHeader.biHeight = Height;
            BMPImageHeader.biPlanes = 1;
            BMPImageHeader.biBitCount = 32;
            
            fwrite(&BMPFileHeader, sizeof(BMPFileHeader), 1, File);
            fwrite(&BMPImageHeader, sizeof(BMPImageHeader), 1, File);
            fwrite(Buffer, sizeof(uint32_t), Width * Height, File);
            fclose(File);
            
            Result = true;
        }
        
        return Result;
    }
};