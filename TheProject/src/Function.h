#pragma once
#include <Arduino.h>

#include <AnimatedGIF.h>
#include <FS.h>
#include <vector>



// PNG stuff

#include <PNGdec.h>
extern AnimatedGIF gif;

extern PNG png;
extern File gifFile;

inline int cropX = 0, cropY = 0; 
#define MAX_IMAGE_WIDTH 400
inline int pic_zoom_out = 0; // 0 = normal, 1 = half, 2 = quarter...
inline int16_t IMG_x_POS = 0, IMG_y_POS = 0;










void MoveInVectorArray(std::vector<String>& reference, const String& stringToMove, bool moveUp);

void GIFDraw(GIFDRAW *pDraw);

void *GIFOpenFile(const char *fname, int32_t *pSize);

void GIFCloseFile(void *pHandle);

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition);



























void DrawPNGPartial(String path, int srcX, int srcY, int srcW, int srcH, int dstX, int dstY);
void DrawPNG(String path);
int pngDrawCropped(PNGDRAW *pDraw);


void *pngOpen(const char *filename, int32_t *size);

void pngClose(void *handle);

int32_t pngRead(PNGFILE *f, uint8_t *buffer, int32_t length);


int32_t pngSeek(PNGFILE *f, int32_t position);






#ifndef DRAW_IMAGE_TRANSPARENT_H
#define DRAW_IMAGE_TRANSPARENT_H
template <typename DrawFunc>
void drawImageTransparent(
    int16_t x, int16_t y,
    int imgW, int imgH,
    const uint16_t *img,
    DrawFunc drawer,
    uint16_t TransparentColor,
    bool useOverrideColor = false,
    uint16_t OverrideColor = 0
)
{
    for (int j = 0; j < imgH; j++)
    {
        for (int i = 0; i < imgW; i++)
        {
            uint16_t c = img[j * imgW + i];
            if (c == TransparentColor) continue;

            drawer(x + i, y + j,
                   useOverrideColor ? OverrideColor : c);
        }
    }
}

#endif






float lerpFloat(float StartPos, float EndPos, float Presentage_0_to_1);

int findStringInArrayStringReturnIndex(const String &value, String arr[], int size);

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);


