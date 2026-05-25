#include "RecordV2.h" // Includes the definitions for the RecordTest class
#include "./MyOS.h"    // Includes the definitions for your OS/GUI (mainOS etc.)
#include "./Classes/MainMenuV2.h"

#include "Fonts/DryBrush15.h"

void RecordV2::Begin()
{
    recData = new int16_t[RECORD_SIZE];                // Allocate array for storing samples (raw audio)
    memset(recData, 0, RECORD_SIZE * sizeof(int16_t)); // Initialize array (starting values = 0)
    
    // Changing CPU frequency to 80MHz (default is usually 240MHz)
    /* setCpuFrequencyMhz(80);

    // Print current frequency to verify it worked
    uint32_t freq = getCpuFrequencyMhz();
    Serial.print("CPU Frequency is set to: ");
    Serial.print(freq);
    Serial.println(" MHz"); */

    // M5Cardputer.Speaker.end(); // Stop speaker — cannot use microphone and speaker simultaneously
    // M5Cardputer.Mic.begin();   // Start microphone

    // startRecording(); // Start the recording process
}

int frameSkip = 0;
void RecordV2::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        if (isRecording)
        {
            stopRecording();
        }
        isRecording = false;
        delay(299);
        M5Cardputer.Mic.end();              // Close the microphone
        M5Cardputer.Speaker.begin();        // Enable the speaker
        M5Cardputer.Speaker.setVolume(255); // Set maximum volume

        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }

    if (isRecording)
    {
        calculateWaveform();

        // Improvement: Draw only every 10th frame to avoid overloading the CPU
        frameSkip++;
        if (frameSkip >= 10)
        {
            drawWaveform();
            frameSkip = 0;
        }
    }
    else
    {
        Draw();
    }

    // If the user pressed ENTER — play back what was recorded
    if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
    {
        if (isRecording)
        {
            stopRecording();
        }
        else // if not recording
        {
            startRecording();
        }
    }
}

void RecordV2::Draw()
{
    auto &scr = mainOS->sprite;

    scr.createSprite(240, 135 - TopOffset);
    scr.setCursor(10, 0);
    scr.setTextColor(ORANGE, BLACK);
    scr.setTextSize(1);
    M5Cardputer.Display.loadFont(DryBrush15);

    scr.print("Press ENTER To Start Recording");
    if (!FilePath.isEmpty())
    {
        scr.setCursor(2, 20);
        scr.unloadFont();
        String MSG = "";
        MSG += "File Save To:\n";
        MSG += FilePath;
        scr.print(MSG);
    }
    scr.pushSprite(0, 0);
}

void RecordV2::startRecording()
{
    FilePath = nextFile();
    dataSize = 0;

    wavFile = SD.open(FilePath, FILE_WRITE);
    writeWavHeader(wavFile, 0);

    isRecording = true; // Flag that we are recording
    recWriteIdx = 0;    // Reset write index to starting position
    recDrawIdx = 0;     // Reset drawing index to starting position

    M5Cardputer.Speaker.end(); // Ensure speaker is closed

    auto cfg = M5Cardputer.Mic.config(); // Get current microphone configuration
    cfg.magnification = 80;              // Set appropriate gain/volume
    cfg.sample_rate = SAMPLE_RATE;
    cfg.noise_filter_level = 1;  // Noise filter level
    M5Cardputer.Mic.config(cfg); // Apply new configuration

    M5Cardputer.Mic.begin(); // Start recording
}

void RecordV2::startPlayback()
{
    isRecording = false; // End recording mode

    // Wait until the microphone stops recording
    while (M5Cardputer.Mic.isRecording())
        delay(1);

    M5Cardputer.Mic.end();              // Close the microphone
    M5Cardputer.Speaker.begin();        // Enable the speaker
    M5Cardputer.Speaker.setVolume(255); // Set maximum volume

    // Play back the data as RAW
    int startPos = recWriteIdx * RECORD_LENGTH;
    if (startPos < RECORD_SIZE)
        M5Cardputer.Speaker.playRaw(&recData[startPos],
                                    RECORD_SIZE - startPos,
                                    SAMPLE_RATE, false);

    if (startPos > 0)
        M5Cardputer.Speaker.playRaw(recData,
                                    startPos,
                                    SAMPLE_RATE, false);

    // Wait until playback finishes
    while (M5Cardputer.Speaker.isPlaying())
        delay(1);

    // After playback — return to continuous recording mode (optional)
    // startRecording();
}

String RecordV2::nextFile()
{
    SD.mkdir("/AdvanceOS");
    SD.mkdir("/AdvanceOS/REC");
    for (int i = 1; i < 999; i++)
    {
        char name[32];
        sprintf(name, "/AdvanceOS/REC/REC_%03d.wav", i);
        if (!SD.exists(name))
            return String(name);
    }
    return "/AdvanceOS/REC/REC_LAST.wav";
}

void RecordV2::calculateWaveform()
{
    int16_t *buf = &recData[recWriteIdx * RECORD_LENGTH];

    // Read audio from the microphone
    if (!M5Cardputer.Mic.record(buf, RECORD_LENGTH, SAMPLE_RATE))
        return;

    // Write to SD - ensure you use the file's internal buffer
    if (wavFile) {
        // Use direct buffer writing without unnecessary loops
        wavFile.write((const uint8_t *)buf, RECORD_LENGTH * sizeof(int16_t));
        dataSize += RECORD_LENGTH * sizeof(int16_t);
    }

    recWriteIdx++;
    if (recWriteIdx >= RECORD_NUMBER) recWriteIdx = 0;
}

void RecordV2::stopRecording()
{
    isRecording = false;

    if (wavFile)
    {
        wavFile.seek(0);
        writeWavHeader(wavFile, dataSize);
        wavFile.close();
    }
}

void RecordV2::drawWaveform()
{
    auto &scr = mainOS->sprite;
    scr.createSprite(240, 135 - TopOffset);

    int waveformTop = 15;
    int waveformH = scr.height() - waveformTop;

    scr.fillRect(10, waveformTop, RECORD_LENGTH, waveformH, BLACK);

    // Draw only existing samples
    int16_t *drawBuf = &recData[recDrawIdx * RECORD_LENGTH];

    int centerY = waveformTop + waveformH / 2;
    const int shift = 8;

    for (int x = 0; x < RECORD_LENGTH; x++)
    {
        int sample = drawBuf[x] >> shift;
        int y = centerY + sample;

        if (y < waveformTop)
            y = waveformTop;
        if (y >= waveformTop + waveformH)
            y = waveformTop + waveformH - 1;

        scr.drawPixel(x + 10, y, WHITE);
        scr.drawPixel(x + 11, y, WHITE);
        scr.drawPixel(x + 10, y + 1, WHITE);
        scr.drawPixel(x + 11, y + 1, WHITE);
    }

    scr.setCursor(10, 0);
    scr.setTextColor(ORANGE, BLACK);
    scr.setTextSize(1);
    scr.print("Press ENTER To Stop Recording");

    // Update drawing index only
    recDrawIdx++;
    if (recDrawIdx >= RECORD_NUMBER)
        recDrawIdx = 0;

    scr.pushSprite(0, TopOffset);
}

void RecordV2::writeWavHeader(File &f, uint32_t dataLen)
{
    uint32_t fileSize = dataLen + 44 - 8;
    uint32_t sampleRate = SAMPLE_RATE;
    uint16_t numChannels = CHANNELS;
    uint16_t bitsPerSample = BITS;
    uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
    uint16_t blockAlign = numChannels * bitsPerSample / 8;

    uint8_t header[44];

    // RIFF
    memcpy(&header[0], "RIFF", 4);
    header[4] = (uint8_t)(fileSize & 0xFF);
    header[5] = (uint8_t)((fileSize >> 8) & 0xFF);
    header[6] = (uint8_t)((fileSize >> 16) & 0xFF);
    header[7] = (uint8_t)((fileSize >> 24) & 0xFF);
    memcpy(&header[8], "WAVE", 4);
    memcpy(&header[12], "fmt ", 4);

    // Subchunk1Size = 16
    header[16] = 0x10;
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;

    // AudioFormat = 1 (PCM)
    header[20] = 0x01;
    header[21] = 0x00;

    // NumChannels
    header[22] = (uint8_t)(numChannels & 0xFF);
    header[23] = (uint8_t)((numChannels >> 8) & 0xFF);

    // SampleRate
    header[24] = (uint8_t)(sampleRate & 0xFF);
    header[25] = (uint8_t)((sampleRate >> 8) & 0xFF);
    header[26] = (uint8_t)((sampleRate >> 16) & 0xFF);
    header[27] = (uint8_t)((sampleRate >> 24) & 0xFF);

    // ByteRate
    header[28] = (uint8_t)(byteRate & 0xFF);
    header[29] = (uint8_t)((byteRate >> 8) & 0xFF);
    header[30] = (uint8_t)((byteRate >> 16) & 0xFF);
    header[31] = (uint8_t)((byteRate >> 24) & 0xFF);

    // BlockAlign
    header[32] = (uint8_t)(blockAlign & 0xFF);
    header[33] = (uint8_t)((blockAlign >> 8) & 0xFF);

    // BitsPerSample
    header[34] = (uint8_t)(bitsPerSample & 0xFF);
    header[35] = (uint8_t)((bitsPerSample >> 8) & 0xFF);

    // data chunk
    memcpy(&header[36], "data", 4);
    header[40] = (uint8_t)(dataLen & 0xFF);
    header[41] = (uint8_t)((dataLen >> 8) & 0xFF);
    header[42] = (uint8_t)((dataLen >> 16) & 0xFF);
    header[43] = (uint8_t)((dataLen >> 24) & 0xFF);

    // Write header: rewind to 0 and attempt block write
    f.seek(0);

// Attempt block write (if supported)
#if defined(ARDUINO) || defined(ESP32)
    // Most Arduino/ESP File classes support write(buffer, len)
    size_t wrote = f.write(header, 44);
    if (wrote == 44)
        return; // Success
#endif

    // Fallback — byte-by-byte write
    for (int i = 0; i < 44; ++i)
    {
        f.write(header[i]);
    }
}

