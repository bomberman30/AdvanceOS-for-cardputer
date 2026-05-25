#ifndef _AUDIOOUTPUTM5_H
#define _AUDIOOUTPUTM5_H
#include <Arduino.h>
#include <AudioOutputI2S.h> 
#include <M5Cardputer.h>

class AudioOutputM5Speaker : public AudioOutput
{
public:
    int MyMusicEqualizer = 0;
    // 0 = Flat (ללא עיבוד)
    // 1 = Warm (Low Pass עדין - מרכך חדות)
    // 2 = Bass Boost
    // 3 = Treble Boost (בהירות)
    // 4 = Voice Clarity (מדגיש אמצע)

    AudioOutputM5Speaker(m5::Speaker_Class *m5sound, uint8_t virtual_sound_channel = 0)
    {
        _m5sound = m5sound;
        _virtual_ch = virtual_sound_channel;
    }
    virtual ~AudioOutputM5Speaker(void) {};
    virtual bool begin(void) override { return true; }

    virtual bool ConsumeSample(int16_t sample[2]) override
    {
        int32_t l = sample[0];
        int32_t r = sample[1];

        switch (MyMusicEqualizer)
        {
        case 0:
            // Flat - ללא שינוי
            break;

        case 1:
        {
            // Warm - Low Pass עדין, alpha גבוה = פחות אגרסיבי
            const float alpha = 0.55f;
            l = (int32_t)(lastL + alpha * (l - lastL));
            r = (int32_t)(lastR + alpha * (r - lastR));
            lastL = l;
            lastR = r;
            break;
        }

        case 2:
        {
            // Bass Boost - מחזק תדרים נמוכים
            smoothL = (smoothL * 3 + l) >> 2;
            smoothR = (smoothR * 3 + r) >> 2;
            l = l + (smoothL >> 1);
            r = r + (smoothR >> 1);
            // Clamp
            l = l > 32767 ? 32767 : (l < -32768 ? -32768 : l);
            r = r > 32767 ? 32767 : (r < -32768 ? -32768 : r);
            break;
        }

        case 3:
        {
            // Treble Boost - High Pass + חיזוק עדין
            int32_t hpL = l - lastL;
            int32_t hpR = r - lastR;
            lastL = l;
            lastR = r;
            l = l + (hpL >> 1);
            r = r + (hpR >> 1);
            // Clamp
            l = l > 32767 ? 32767 : (l < -32768 ? -32768 : l);
            r = r > 32767 ? 32767 : (r < -32768 ? -32768 : r);
            break;
        }

        case 4:
        {
            // Voice Clarity - Band pass פשוט (מדגיש טווח 800Hz-4kHz בערך)
            // Low pass
            smoothL = (smoothL + l) >> 1;
            smoothR = (smoothR + r) >> 1;
            // High pass על גבי Low pass = Band pass
            int32_t bandL = l - smoothL + (l - lastL);
            int32_t bandR = r - smoothR + (r - lastR);
            lastL = l;
            lastR = r;
            l = l + (bandL >> 2);
            r = r + (bandR >> 2);
            // Clamp
            l = l > 32767 ? 32767 : (l < -32768 ? -32768 : l);
            r = r > 32767 ? 32767 : (r < -32768 ? -32768 : r);
            break;
        }
        }

        sample[0] = (int16_t)l;
        sample[1] = (int16_t)r;

        if (_tri_buffer_index < tri_buf_size)
        {
            _tri_buffer[_tri_index][_tri_buffer_index]     = sample[0];
            _tri_buffer[_tri_index][_tri_buffer_index + 1] = sample[1];
            _tri_buffer_index += 2;
            return true;
        }

        flush();
        return false;
    }

    virtual void flush(void) override
    {
        if (_tri_buffer_index)
        {
            _m5sound->playRaw(_tri_buffer[_tri_index], _tri_buffer_index, hertz, true, 1, _virtual_ch);
            _tri_index = _tri_index < 2 ? _tri_index + 1 : 0;
            _tri_buffer_index = 0;
        }
    }

    virtual bool stop(void) override
    {
        flush();
        _m5sound->stop(_virtual_ch);
        return true;
    }

    const int16_t *getBuffer(void) const { return _tri_buffer[(_tri_index + 2) % 3]; }

private:
    int16_t lastL = 0;
    int16_t lastR = 0;

protected:
    m5::Speaker_Class *_m5sound;
    uint8_t _virtual_ch;
    static constexpr size_t tri_buf_size = 1536;
    int16_t _tri_buffer[3][tri_buf_size];
    size_t _tri_buffer_index = 0;
    size_t _tri_index = 0;

    int32_t smoothL = 0;
    int32_t smoothR = 0;
};

#endif