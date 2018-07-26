#pragma once

#define ARRAY_COUNT(Array) sizeof(Array)/sizeof((Array)[0])

struct buf_header
{
    int Cap;
    int Len;
};

#define BufHeader(B) ((buf_header *)(B) - 1)
#define BufCap(B) ((B)? BufHeader(B)->Cap: 0)
#define BufLen(B) ((B)? BufHeader(B)->Len: 0)

#define BufGrow(B) (B) = (decltype(B))__BufGrow(B, sizeof(*B))
#define BufFit1(B) (BufCap(B) < BufLen(B)+1? BufGrow(B): 0)
#define BufPush(B, Elem) (BufFit1(B), B[BufHeader(B)->Len++] = Elem)

#define BufLast(B) ((B)? B + (BufLen(B)-1): 0)

void *__BufGrow(void *B, int ElemSize)
{
    if (B == 0)
    {
        buf_header *Header = (buf_header *)malloc(sizeof(buf_header) + ElemSize);
        Header->Cap = 1;
        Header->Len = 0;
        return (void *)(Header + 1);
    }
    
    buf_header *Header = BufHeader(B);
    
    int NewLen = Header->Len + 1;
    if (NewLen > Header->Cap)
    {
        int NewCap = Header->Cap * 2 + 1;
        ASSERT(NewCap > NewLen);
        
        Header = (buf_header *)realloc(Header, sizeof(buf_header) + 
                                       NewCap * ElemSize);
        Header->Cap = NewCap;
    }
    
    return (void *)(Header + 1);
}
