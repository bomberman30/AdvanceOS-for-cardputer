#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>

#include <SD.h>
#include <AudioOutput.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorWAV.h>
#include <elapsedMillis.h>
#include <AudioOutputI2S.h> 
#define FFT_SIZE 256

class fft_t
{
    float _wr[FFT_SIZE + 1];
    float _wi[FFT_SIZE + 1];
    float _fr[FFT_SIZE + 1];
    float _fi[FFT_SIZE + 1];
    uint16_t _br[FFT_SIZE + 1];
    size_t _ie;

public:
    fft_t(void)
    {
#ifndef M_PI
#define M_PI 3.141592653
#endif
        _ie = logf((float)FFT_SIZE) / log(2.0) + 0.5;
        static constexpr float omega = 2.0f * M_PI / FFT_SIZE;
        static constexpr int s4 = FFT_SIZE / 4;
        static constexpr int s2 = FFT_SIZE / 2;
        for (int i = 1; i < s4; ++i)
        {
            float f = cosf(omega * i);
            _wi[s4 + i] = f;
            _wi[s4 - i] = f;
            _wr[i] = f;
            _wr[s2 - i] = -f;
        }
        _wi[s4] = _wr[0] = 1;

        size_t je = 1;
        _br[0] = 0;
        _br[1] = FFT_SIZE / 2;
        for (size_t i = 0; i < _ie - 1; ++i)
        {
            _br[je << 1] = _br[je] >> 1;
            je = je << 1;
            for (size_t j = 1; j < je; ++j)
            {
                _br[je + j] = _br[je] + _br[j];
            }
        }
    }

    void exec(const int16_t *in)
    {
        memset(_fi, 0, sizeof(_fi));
        for (size_t j = 0; j < FFT_SIZE / 2; ++j)
        {
            float basej = 0.25 * (1.0 - _wr[j]);
            size_t r = FFT_SIZE - j - 1;

            /// perform han window and stereo to mono convert.
            _fr[_br[j]] = basej * (in[j * 2] + in[j * 2 + 1]);
            _fr[_br[r]] = basej * (in[r * 2] + in[r * 2 + 1]);
        }

        size_t s = 1;
        size_t i = 0;
        do
        {
            size_t ke = s;
            s <<= 1;
            size_t je = FFT_SIZE / s;
            size_t j = 0;
            do
            {
                size_t k = 0;
                do
                {
                    size_t l = s * j + k;
                    size_t m = ke * (2 * j + 1) + k;
                    size_t p = je * k;
                    float Wxmr = _fr[m] * _wr[p] + _fi[m] * _wi[p];
                    float Wxmi = _fi[m] * _wr[p] - _fr[m] * _wi[p];
                    _fr[m] = _fr[l] - Wxmr;
                    _fi[m] = _fi[l] - Wxmi;
                    _fr[l] += Wxmr;
                    _fi[l] += Wxmi;
                } while (++k < ke);
            } while (++j < je);
        } while (++i < _ie);
    }

    uint32_t get(size_t index)
    {
        return (index < FFT_SIZE / 2) ? (uint32_t)sqrtf(_fr[index] * _fr[index] + _fi[index] * _fi[index]) : 0u;
    }
};



class MusicPlayerV2 : public GlobalParentClass
{
public:
    enum class FileType
    {
        UNKNOWN,
        MP3,
        WAV,

    };
    FileType CurrentFileType;
    MusicPlayerV2(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;
    void Loop() override;
    void Draw() override;
            void OnExit() override;

    // std::vector<String> playlist;
    bool PlayWAV = false;
    bool Focused = true;
    bool first_frame = true;
    bool isPlaying = false;
    // String CurrentFileType;
    // elapsedSeconds refrasFirstFrame;
    bool pause = false;
static void loop2(void *pvParameters);
   // TaskHandle_t taskCore2Handle = nullptr;
String SongList[200];
private:
fft_t fft;
    void stop();
    void play(const char *fname);

    AudioFileSourceSD AudioFile;
    AudioFileSourceID3 *id3 = nullptr;
    AudioGeneratorMP3 MP3_GENERATOR;
    AudioGeneratorWAV WAV_GENERATOR;

    // AudioGeneratorMIDI WAV_GENERATOR;

    size_t fileIndex = 0;
    void gfxSetup(LGFX_Device *gfx);


   // AudioOutput *output = nullptr;
//AudioOutputI2S *outRes;

   // AudioOutputI2S *outRes;
    int InMusicPlayLoop = 0;

    void WheneSongLoading();
    void WheneSongFinishLoadAndPlay();
    elapsedMillis CheackTimer;
    uint32_t lastCheackPos;
    int TotalSongSeconds = 0;
std::vector<int>SecondArray;
int SecondAverage;
void SetSongTimeByAverage();
    int seconds;
    int minutes;



    /*
                float percent = (AudioFile.getPos() * 100.0f) / AudioFile.getSize();

                mainOS->ShowOnScreenMessege(
                    "in pos: " + String(AudioFile.getPos()) +
                        " size: " + String(AudioFile.getSize()) +
                        " " + String(percent, 1) + "%",
                    2000); */
    // get file pos
};
