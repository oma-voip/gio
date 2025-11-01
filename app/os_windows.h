// SPDX-License-Identifier: Unlicense OR MIT

// +build windows
#ifndef DIB2BMP_H
#define DIB2BMP_H

#include <stdint.h>
#include <windows.h>

typedef struct
{
    uint8_t *bytes;
    uint32_t length;
} BmpBuffer;

typedef struct
{
    char **paths;
    uint32_t count;
} PathList;

BmpBuffer MakeBMPFromDIB(void *dib);
PathList GetDropFilePaths(void *dropPtr);
void FreePathList(PathList *pl);
#endif