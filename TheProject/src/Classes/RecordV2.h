#pragma once
#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <SD.h>
#include <FS.h>
class RecordV2 : public GlobalParentClass
{
public:
    RecordV2(MyOS *os) : GlobalParentClass(os) {}
    File wavFile;        
    uint32_t dataSize = 0; 
    static const uint32_t SAMPLE_RATE = 16000;
    // static const uint32_t SAMPLE_RATE = 32000;
    static const uint16_t CHANNELS = 1;
    static const uint16_t BITS = 16;
    void Begin() override;
    void Loop() override;
    void Draw() override;

private:
    String FilePath = "";

    static constexpr size_t RECORD_NUMBER = 80; // 1 sec = 16000 samples / 200
    static constexpr size_t RECORD_LENGTH = 200;
    static constexpr size_t RECORD_SIZE = RECORD_NUMBER * RECORD_LENGTH;
    // static constexpr size_t RECORD_SAMPLERATE   = 16000;

    int16_t *recData = nullptr;

    size_t recWriteIdx = 0;
    size_t recDrawIdx = 0;

    bool isRecording = false;

    void startRecording();
    void startPlayback();

    String nextFile();
    void calculateWaveform();
    void stopRecording();
    void drawWaveform();

    void writeWavHeader(File &f, uint32_t dataLen);
    // void writeWavHeader(File &file, uint32_t sampleRate, uint16_t bits, uint16_t channels, uint32_t dataSize);
};