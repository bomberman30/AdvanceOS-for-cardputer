#include "Function.h"
#include "MyOS.h"
#include "M5Cardputer.h"

 PNG png;
File pngfile;

AnimatedGIF gif;
File gifFile;

void MoveInVectorArray(std::vector<String> &reference, const String &stringToMove, bool moveUp)
{
    auto it = std::find(reference.begin(), reference.end(), stringToMove);

    if (it == reference.end())
        return; // לא נמצא

    if (moveUp)
    {
        if (it != reference.begin())
            std::iter_swap(it, it - 1);
    }
    else
    {
        if (it != reference.end() - 1)
            std::iter_swap(it, it + 1);
    }
}
void GIFDraw(GIFDRAW *pDraw)
{
    uint16_t *usPalette = pDraw->pPalette;
    uint8_t *s = pDraw->pPixels;
    int step = 1 << pic_zoom_out;

    int srcY = pDraw->iY + pDraw->y;
    if (srcY % step != 0)
        return;

    int drawY = IMG_y_POS + (srcY / step);
    if (drawY < 0 || drawY >= SCREEN_H)
        return;

    int iWidth = pDraw->iWidth;
    int baseX = IMG_x_POS + (pDraw->iX / step);

    // טיפול נכון ב-disposal=2 (כמו הדוגמה הרשמית)
    if (pDraw->ucDisposalMethod == 2)
    {
        for (int x = 0; x < iWidth; x++)
        {
            if (s[x] == pDraw->ucTransparent)
                s[x] = pDraw->ucBackground; // החלף שקוף → רקע
        }
        pDraw->ucHasTransparency = 0; // בטל שקיפות
    }

    if (pDraw->ucHasTransparency)
    {
        uint8_t ucTransparent = pDraw->ucTransparent;

        for (int i = 0; i < iWidth; i += step)
        {
            int drawX = baseX + (i / step);
            if (drawX < 0 || drawX >= SCREEN_W)
                continue;

            if (s[i] == ucTransparent)
                continue; // דלג על שקוף

            M5Cardputer.Display.drawPixel(drawX, drawY, usPalette[s[i]]);
        }
    }
    else
    {
        // אין שקיפות — שלח שורה שלמה בבת אחת
        for (int i = 0; i < iWidth; i += step)
        {
            int drawX = baseX + (i / step);
            if (drawX < 0 || drawX >= SCREEN_W)
                continue;

            M5Cardputer.Display.drawPixel(drawX, drawY, usPalette[s[i]]);
        }
    }
}

void *GIFOpenFile(const char *fname, int32_t *pSize)
{
    gifFile.close();
    gifFile = SD.open(fname);
    if (gifFile)
    {
        *pSize = gifFile.size();
        return (void *)&gifFile;
    }
    return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
    File *gifFile = static_cast<File *>(pHandle);
    if (gifFile != NULL)
        gifFile->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
        iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
        return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
    int i = micros();
    File *f = static_cast<File *>(pFile->fHandle);
    f->seek(iPosition);
    pFile->iPos = (int32_t)f->position();
    i = micros() - i;
    //  Serial.printf("Seek time = %d us\n", i);
    return pFile->iPos;
} /* GIFSeekFile() */

int pngDrawCropped(PNGDRAW *pDraw)
{
    if (pDraw->y < cropY || pDraw->y >= cropY + SCREEN_H)
        return 1;

    uint16_t fullLine[MAX_IMAGE_WIDTH];
    png.getLineAsRGB565(pDraw, fullLine, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

    int startX = cropX;
    int endX = cropX + SCREEN_W;

    if (startX < 0)
        startX = 0;
    if (endX > pDraw->iWidth)
        endX = pDraw->iWidth;

    int outW = endX - startX;

    M5Cardputer.Display.pushImage(
        IMG_x_POS,
        IMG_y_POS + (pDraw->y - cropY),
        outW,
        1,
        &fullLine[startX]);

    return 1;
}

int pngDraw(PNGDRAW *pDraw)
{
    int skip = pic_zoom_out + 1;

    if (pic_zoom_out > 0)
    {
        if (pDraw->y % skip != 0)
            return 1;
    }

    uint16_t fullLine[MAX_IMAGE_WIDTH];
    png.getLineAsRGB565(pDraw, fullLine, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

    if (pic_zoom_out == 0)
    {
        M5Cardputer.Display.pushImage(
            IMG_x_POS,
            IMG_y_POS + pDraw->y,
            pDraw->iWidth,
            1,
            fullLine);
        return 1;
    }

    static uint16_t smallLine[MAX_IMAGE_WIDTH];
    int outIndex = 0;

    for (int x = 0; x < pDraw->iWidth; x += skip)
    {
        smallLine[outIndex++] = fullLine[x];
    }

    M5Cardputer.Display.pushImage(
        IMG_x_POS,
        IMG_y_POS + (pDraw->y / skip),
        outIndex,
        1,
        smallLine);

    return 1;
}

void *pngOpen(const char *filename, int32_t *size)
{
    //  if (pngfile) pngfile.close(); // ← הוסף את זה!

    pngfile = SD.open(filename, "r");
    if (!pngfile)
        return NULL;
    *size = pngfile.size();
    return &pngfile;
}

void pngClose(void *handle)
{
    if (pngfile)
        pngfile.close();
}

int32_t pngRead(PNGFILE *f, uint8_t *buffer, int32_t length)
{
    return pngfile.read(buffer, length);
}

int32_t pngSeek(PNGFILE *f, int32_t position)
{
    return pngfile.seek(position);
}

void DrawPNG(String path)
{
    /*     if (!SD.exists(path))
        {
            Serial.println("PNG not found!");
            return;
        } */

    int16_t rc = png.open(
        path.c_str(),
        pngOpen,
        pngClose,
        pngRead,
        pngSeek,
        pngDraw);

    if (rc != PNG_SUCCESS)
    {
        Serial.printf("PNG open failed: %d\n", rc);
        return;
    }

    M5Cardputer.Display.startWrite();

  //  if (png.getWidth() <= MAX_IMAGE_WIDTH && png.getHeight() <= MAX_IMAGE_WIDTH)
    //{
        png.decode(NULL, 0);
   // }


    png.close();
    M5Cardputer.Display.endWrite();
}

void DrawPNGPartial(String path, int srcX, int srcY, int srcW, int srcH, int dstX, int dstY)
{
    cropX = srcX;
    cropY = srcY;
    /*     cropW = srcW;
        cropH = srcH; */

    IMG_x_POS = dstX;
    IMG_y_POS = dstY;

    int16_t rc = png.open(
        path.c_str(),
        pngOpen,
        pngClose,
        pngRead,
        pngSeek,
        pngDrawCropped);

    if (rc != PNG_SUCCESS)
    {
        Serial.println("PNG open failed");
        return;
    }

    M5Cardputer.Display.startWrite();
    png.decode(NULL, 0);
    png.close();
    M5Cardputer.Display.endWrite();
}

float lerpFloat(float StartPos, float EndPos, float Presentage_0_to_1)
{
    return StartPos + Presentage_0_to_1 * (EndPos - StartPos);
}
int findStringInArrayStringReturnIndex(const String &value, String arr[], int size)
{

    for (int i = 0; i < size; i++)
        if (arr[i] == value)
            return i;

    return -1;
}

// i think we have alredy built in function like this but still....
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
