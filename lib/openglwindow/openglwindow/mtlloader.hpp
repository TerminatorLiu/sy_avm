#ifndef MTLLOADER_H
#define MTLLOADER_H

#include <vector>


#define STRING_NAME_LENGTH 128
typedef struct _color3
{
    float r;
    float g;
    float b;
} color3;


typedef struct _material
{
    char name[STRING_NAME_LENGTH]; // materal name defined follow "usemtl"
    color3 ColorKa; // color defined for the materal
    color3 ColorKd; // color defined for the materal
    color3 ColorKs; // color defined for the materal
    int    Illum;
    color3 Alpha; // alpha defined for the materal
} MATERIAL;
bool loadMTL(
    const char *path,
    std::vector<MATERIAL> &out_materals
);

void EditMtl(std::vector<MATERIAL> &out_materials, const char *KeyWord, color3 ReplaceColor);

//#include <cstdio>
//#include<android/log.h>//6700
//#define LOG    "....hh"

//#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG,__VA_ARGS__)//6700
#define LOGE  printf

#endif
