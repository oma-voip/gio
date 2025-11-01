// SPDX-License-Identifier: Unlicense OR MIT

// +build windows
#include <stdlib.h>
#include <string.h>
#include "os_windows.h"

static uint32_t rowStride(uint32_t bpp, uint32_t width)
{
    return ((bpp * width + 31u) & ~31u) >> 3;
}

BmpBuffer MakeBMPFromDIB(void *dib)
{
    BmpBuffer out = {NULL, 0};
    if (!dib)
        return out;

    const uint8_t *dibStart = (const uint8_t *)dib;
    const BITMAPINFOHEADER *bih = (const BITMAPINFOHEADER *)dib;

    uint32_t clrUsed = bih->biClrUsed;
    if (clrUsed == 0 && bih->biBitCount <= 8)
        clrUsed = 1u << bih->biBitCount;

    uint32_t paletteSize = clrUsed * sizeof(RGBQUAD);
    uint32_t dibHeaderSize = bih->biSize;
    uint32_t imgSize = bih->biSizeImage;

    if (imgSize == 0)
        imgSize = rowStride(bih->biBitCount, bih->biWidth) * abs(bih->biHeight);

    uint32_t dibSize = dibHeaderSize + paletteSize + imgSize;

    BITMAPFILEHEADER bfh;
    bfh.bfType = 0x4D42;
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + dibHeaderSize + paletteSize;
    bfh.bfSize = bfh.bfOffBits + imgSize;

    out.bytes = (uint8_t *)malloc(bfh.bfSize);
    if (!out.bytes)
        return out;

    uint8_t *p = out.bytes;
    memcpy(p, &bfh, sizeof(bfh));
    p += sizeof(bfh);
    memcpy(p, dibStart, dibSize);

    out.length = bfh.bfSize;
    return out;
}

static char *wstr_to_utf8(const WCHAR *ws)
{
    int need = WideCharToMultiByte(CP_UTF8, 0, ws, -1, NULL, 0, NULL, NULL);
    if (need == 0)
        return NULL;

    char *utf8 = (char *)malloc((size_t)need);
    if (!utf8)
        return NULL;

    WideCharToMultiByte(CP_UTF8, 0, ws, -1, utf8, need, NULL, NULL);
    return utf8;
}

PathList GetDropFilePaths(void *dropPtr)
{
    HDROP hDrop = (HDROP)dropPtr;
    PathList pl = {NULL, 0};
    if (!hDrop)
        return pl;

    UINT cFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
    if (cFiles == 0)
        return pl;

    pl.paths = (char **)malloc(sizeof(char *) * cFiles);
    if (!pl.paths)
        return pl;
    pl.count = cFiles;

    for (UINT i = 0; i < cFiles; ++i)
    {
        UINT chars = DragQueryFileW(hDrop, i, NULL, 0); /* length w/o NUL */
        WCHAR *wbuf = (WCHAR *)malloc(sizeof(WCHAR) * (chars + 1));
        if (!wbuf)
        {
            pl.paths[i] = NULL;
            continue;
        }

        DragQueryFileW(hDrop, i, wbuf, chars + 1);
        pl.paths[i] = wstr_to_utf8(wbuf);
        free(wbuf);
    }
    return pl;
}

void FreePathList(PathList *pl)
{
    if (!pl || !pl->paths)
        return;

    for (uint32_t i = 0; i < pl->count; ++i)
        free(pl->paths[i]);

    free(pl->paths);
    pl->paths = NULL;
    pl->count = 0;
}