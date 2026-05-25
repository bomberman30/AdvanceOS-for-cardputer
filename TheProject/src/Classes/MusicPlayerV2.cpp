#include "MusicPlayerV2.h"

#include "./MyOS.h"
#include "FileBrowser.h"
#include "tools/AudioOutputM5Speaker.h"


static constexpr size_t WAVE_SIZE = 320;

static constexpr uint8_t m5spk_virtual_channel = 0;



static AudioOutputM5Speaker out(&M5.Speaker, m5spk_virtual_channel);
static bool wave_enabled = false;

static uint16_t prev_y[(FFT_SIZE / 2) + 1];
static uint16_t peak_y[(FFT_SIZE / 2) + 1];
static int16_t wave_y[WAVE_SIZE];
static int16_t wave_h[WAVE_SIZE];
static int16_t raw_data[WAVE_SIZE * 2];
static int header_height = 0;
static bool fft_enabled = false;

uint32_t bgcolor(LGFX_Device *gfx, int y)
{
    auto h = gfx->height();
    auto dh = h - header_height;
    int v = ((h - y) << 5) / dh;
    if (dh > 44)
    {
        int v2 = ((h - y - 1) << 5) / dh;
        if ((v >> 2) != (v2 >> 2))
        {
            return 0x666666u;
        }
    }
    return gfx->color888(v + 2, v, v + 6);
}

void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
    (void)cbData;
    if (string[0] == 0)
    {
        return;
    }
    if (strcmp(type, "eof") == 0)
    {
        M5.Display.display();
        return;
    }
    int y = M5.Display.getCursorY();
    if (y + 1 >= header_height)
    {
        return;
    }
    M5.Display.fillRect(0, y, M5.Display.width(), 12, M5.Display.getBaseColor());
    M5.Display.printf("%s: %s", type, string);
    M5.Display.setCursor(0, y + 12);
}

void MusicPlayerV2::stop()
{
    if (id3)
    {

        // outRes->stop();

        switch (CurrentFileType)
        {
        case FileType::MP3:
        {
            MP3_GENERATOR.stop();
            break;
        }
        case FileType::WAV:
        {
            WAV_GENERATOR.stop();
            break;
        }
        }

        id3->close();
        delete id3;
        id3 = nullptr;
        isPlaying = false;
    }
}

void MusicPlayerV2::play(const char *fname)
{
    if (!mainOS->SongReady)
    {
        return;
    }
    InMusicPlayLoop = 0;

    SecondArray.clear();
    if (id3 != nullptr)
    {
        switch (CurrentFileType)
        {
        case FileType::MP3:
        {
            MP3_GENERATOR.stop();
            break;
        }
        case FileType::WAV:
        {
            WAV_GENERATOR.stop();
            break;
        }
        }
    }
    /*     String msg = "Loading ";
        msg += mainOS->getFileNameFromPath(fname);

        mainOS->ShowOnScreenMessege(msg, 1); */
    M5.Display.setCursor(0, 8);
    // mainOS->ShowOnScreenMessege("1",500);
    AudioFile.open(fname);
    //     mainOS->ShowOnScreenMessege("2",500);

    id3 = new AudioFileSourceID3(&AudioFile);
    // draw metadata on screen
    // id3->RegisterMetadataCB(MDCallback, (void *)"ID3TAG");
    //   mainOS->ShowOnScreenMessege("3",500);

    id3->open(fname);
    // mainOS->ShowOnScreenMessege("4",500);

    String FileEXT = mainOS->GetExtensionLower(fname);
    if (FileEXT == "mp3")
    {
        CurrentFileType = FileType::MP3;
    }
    else if (FileEXT == "wav")
    {
        CurrentFileType = FileType::WAV;
    }

    switch (CurrentFileType)
    {
    case FileType::MP3:
    {
        //  mainOS->ShowOnScreenMessege("2",500);

        MP3_GENERATOR.begin(id3, &out);
        //      mainOS->ShowOnScreenMessege("3",500);

        break;
    }
    case FileType::WAV:
    {
        WAV_GENERATOR.begin(id3, &out);
        break;
    }
    }

    isPlaying = true;
    first_frame = true;
    WheneSongLoading();
    // mainOS->ShowOnScreenMessege("2",500);
}

void MusicPlayerV2::loop2(void *pvParameters)
{
    MusicPlayerV2 *Player = static_cast<MusicPlayerV2 *>(pvParameters);
    while (1)
    {

        if (Player->MP3_GENERATOR.isRunning())
        {
            if (!Player->pause)
            {
                Player->InMusicPlayLoop++;
                if (!Player->MP3_GENERATOR.loop())
                {
                    // mainOS->ShowOnScreenMessege("song finish", 100);
                    Player->mainOS->InCurrentSong++;
                    if (Player->mainOS->InCurrentSong >= Player->mainOS->SongsInTheList)
                    {
                        Player->mainOS->InCurrentSong = 0;
                    }
                    Player->mainOS->saveSettings();

                    Player->MP3_GENERATOR.stop();
                    Player->play(Player->SongList[Player->mainOS->InCurrentSong].c_str());
                }
            }
        }

        else if (Player->WAV_GENERATOR.isRunning())
        {
            if (!Player->pause)
            {
                Player->InMusicPlayLoop++;

                if (!Player->WAV_GENERATOR.loop())
                {
                    // mainOS->ShowOnScreenMessege("song finish", 100);
                    Player->mainOS->InCurrentSong++;
                    if (Player->mainOS->InCurrentSong >= Player->mainOS->SongsInTheList)
                    {
                        Player->mainOS->InCurrentSong = 0;
                    }
                    Player->mainOS->saveSettings();

                    Player->WAV_GENERATOR.stop();
                    Player->play(Player->SongList[Player->mainOS->InCurrentSong].c_str());
                }
            }
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void MusicPlayerV2::Begin()
{
    /*     if (taskCore2Handle == nullptr)
        {
            xTaskCreatePinnedToCore(
                loop2,            // function
                "Core2Loop",      // name
                10240 / 4,        // stack (words) ~ 4KB
                this,             // param
                1,                // priority
                &taskCore2Handle, // handle
                1                 // core (1)
            );
        }
     */
    
    showTopBar = false;

    first_frame = true;
    if (mainOS->SongsInTheList <= 0)
    {
        if (!mainOS->LastFolderPlaySongInSave.isEmpty())
        {
            mainOS->GetAllSoundFileFromFolderToPlaylist(mainOS->LastFolderPlaySongInSave.c_str());
            mainOS->SongReady = true;
        }
        else // if never play song (no song dir in save) and dont have song list in RAM
        {
            mainOS->ShowOnScreenMessege("Choose Song From File Explorer", 3000);
            // mainOS->ExitMusicPlayer();
            // mainOS->ChangeMenu(new FileBrowser(mainOS));
        }
    }
    // outRes = new AudioOutputI2S();
    //   output = new AudioOutput();

    M5.Speaker.begin();

    if (!isPlaying)
    {
        play(SongList[mainOS->InCurrentSong].c_str());
    }
}

void MusicPlayerV2::Loop()
{
    // outRes->SetGain(random());

    // claculate time by size and position (NotAcurate becose the read bytes not leniar)
    if (CheackTimer > 1000)
    {
        CheackTimer = 0;
        uint32_t totalBytes = AudioFile.getSize();                     // let's say 8000
        uint32_t BitPassInSecond = AudioFile.getPos() - lastCheackPos; // lest say current pos 500 pass in second

        if (BitPassInSecond > 0)
        {
            // mainOS->ShowOnScreenMessege("sample bytes",100);
            TotalSongSeconds = (uint32_t)totalBytes / BitPassInSecond;
            // Serial.println(totalSeconds);
            SecondArray.push_back(TotalSongSeconds);
            // seconds = (int)TotalSongSeconds % 60;
            // minutes = (int)TotalSongSeconds / 60;
        }

        lastCheackPos = AudioFile.getPos();
    }
    // end of // claculate time by size and position
    if (InMusicPlayLoop == 3) // finish load
    {
        first_frame = true;
        WheneSongFinishLoadAndPlay();
    }
    if (MP3_GENERATOR.isRunning())
    {
        if (!pause)
        {
            InMusicPlayLoop++;
            if (!MP3_GENERATOR.loop())
            {
                // mainOS->ShowOnScreenMessege("song finish", 100);
                mainOS->InCurrentSong++;
                if (mainOS->InCurrentSong >= mainOS->SongsInTheList)
                {
                    mainOS->InCurrentSong = 0;
                }
                mainOS->saveSettings();

                MP3_GENERATOR.stop();
                play(SongList[mainOS->InCurrentSong].c_str());
            }
        }
    }

    else if (WAV_GENERATOR.isRunning())
    {
        if (!pause)
        {
            InMusicPlayLoop++;

            if (!WAV_GENERATOR.loop())
            {
                // mainOS->ShowOnScreenMessege("song finish", 100);
                mainOS->InCurrentSong++;
                if (mainOS->InCurrentSong >= mainOS->SongsInTheList)
                {
                    mainOS->InCurrentSong = 0;
                }
                mainOS->saveSettings();

                WAV_GENERATOR.stop();
                play(SongList[mainOS->InCurrentSong].c_str());
            }
        }
    }

    else
    {
        M5.delay(1);
    }
    if (Focused) // if focus senale keys
    {
        // AudioFileSourceID3.
        if (mainOS->NewKey.ifKeyJustPress('1')) // esc
        {
            out.MyMusicEqualizer =0;

        }
        if (mainOS->NewKey.ifKeyJustPress('2')) // esc
        {
            out.MyMusicEqualizer =1;
        }
                if (mainOS->NewKey.ifKeyJustPress('3')) // esc
        {
            out.MyMusicEqualizer =2;
        }
                if (mainOS->NewKey.ifKeyJustPress('4')) // esc
        {
            out.MyMusicEqualizer =3;
        }
                if (mainOS->NewKey.ifKeyJustPress('5')) // esc
        {
            out.MyMusicEqualizer =4;
        }
        /*         if (mainOS->NewKey.ifKeyJustPress('1')) // esc
                {
                    float percent = (AudioFile.getPos() * 100.0f) / AudioFile.getSize();
                    SetSongTimeByAverage();
                    mainOS->ShowOnScreenMessege(
                        "in pos: " + String(AudioFile.getPos()) +
                            " size: " + String(AudioFile.getSize()) +
                            " " + String(percent, 1) + "%" + "\n" + "time" + int(minutes) + ":" + int(seconds) + "\n" + "bits in last second " + String(AudioFile.getPos() - lastCheackPos),
                        2000);

                    // need in every click to not sample second of the song to make the right duration of the song
                    CheackTimer = 0;
                    lastCheackPos = AudioFile.getPos();
                } */
        if (mainOS->NewKey.KeyPressWorkInMilliSecond(']', 100)) // esc
        {
            uint32_t pos = AudioFile.getPos();
            uint32_t size = AudioFile.getSize();

            uint32_t SkipAmount = AudioFile.getSize() / 60; // 1/20 part of the song
            uint32_t newPos = pos + SkipAmount;
            if (newPos > size)
                newPos = size;

            AudioFile.seek(newPos, SEEK_SET);
            // need in every click to not sample second of the song to make the right duration of the song
            CheackTimer = 0;
            lastCheackPos = AudioFile.getPos();
        }
        if (mainOS->NewKey.KeyPressWorkInMilliSecond('[', 100)) // esc
        {
            uint32_t pos = AudioFile.getPos();
            uint32_t size = AudioFile.getSize();
            uint32_t SkipAmount = AudioFile.getSize() / 60; // 1/20 part of the song

            int32_t newPos = (int32_t)pos - SkipAmount;
            if (newPos < 0)
                newPos = 0;

            AudioFile.seek(newPos, SEEK_SET);
            // need in every click to not sample second of the song to make the right duration of the song
            CheackTimer = 0;
            lastCheackPos = AudioFile.getPos();
        }
        if (mainOS->NewKey.ifKeyJustPress('`')) // esc
        {
            mainOS->ExitMusicPlayer();
            GlobalParentClass::BackToMainMenu();
        }

        // GlobalParentClass::Loop();

        // M5.update();
        if (mainOS->NewKey.ifKeyJustRelease('/'))
        {
            // M5.Speaker.tone(500, 20);
            stop();
            mainOS->InCurrentSong++;

            // mainOS->InCurrentSong >= mainOS->SongsInTheList
            if (mainOS->InCurrentSong >= mainOS->SongsInTheList)
            {
                mainOS->InCurrentSong = 0;
            }
            mainOS->saveSettings();

            play(SongList[mainOS->InCurrentSong].c_str());
            // need in every click to not sample second of the song to make the right duration of the song
            CheackTimer = 0;
            lastCheackPos = AudioFile.getPos();
        }
        if (mainOS->NewKey.ifKeyJustRelease(','))
        {
            //  M5.Speaker.tone(500, 20);
            stop();
            mainOS->InCurrentSong--;

            // mainOS->InCurrentSong >= mainOS->SongsInTheList
            if (mainOS->InCurrentSong < 0)
            {
                mainOS->InCurrentSong = mainOS->SongsInTheList - 1;
            }
            mainOS->saveSettings();

            play(SongList[mainOS->InCurrentSong].c_str());
            // need in every click to not sample second of the song to make the right duration of the song
            CheackTimer = 0;
            lastCheackPos = AudioFile.getPos();
        }

        if (mainOS->NewKey.ifKeyJustPress(KEY_ENTER))
            if (!pause)
            {
                pause = true;
            }
            else
            {
                pause = false;
            }
    }
    if (mainOS->NewKey.ApplyPushedKeyEveryMS('-', 20)) // volume down
    {
        // uint8_t v = mainOS->volume;

        mainOS->volume -= 3;
        if (mainOS->volume <= 0)
        {
            mainOS->volume = 0;
        }
        M5.Speaker.setVolume(mainOS->volume);
        //  outRes->SetGain(M5.Speaker.getVolume() / 255.0);
    }
    if (mainOS->NewKey.ApplyPushedKeyEveryMS('=', 20)) // volume up
    {
        // uint8_t v = M5.Speaker.getVolume();

        mainOS->volume += 3;
        if (mainOS->volume >= 255)
        {
            mainOS->volume = 255;
        }
        M5.Speaker.setVolume(mainOS->volume);
        // outRes->SetGain(M5.Speaker.getVolume() / 255.0);
    }
    if (Focused && mainOS->screenOff == false)
    {
        Draw();
    }
}
// Function to set up graphics display (initialization of UI elements)
void MusicPlayerV2::gfxSetup(LGFX_Device *gfx)
{
    // Check if the graphics pointer is valid; return early if not
    if (gfx == nullptr)
    {
        return;
    }

    // Draw a black header bar at top of screen (height = 50px)
    gfx->fillRect(0, 0, 240, 47, BLACK);

    // Set font for text rendering to Japanese Gothic font at size 12
    gfx->setFont(&fonts::lgfxJapanGothic_12);

    // Enable fast EPD mode (for e-ink displays) or default behavior otherwise
    // gfx->setEpdMode(epd_mode_t::epd_fastest);

    // Set text color to orange for visibility on dark background
    gfx->setTextColor(ORANGE);

    if (mainOS->SongsInTheList > 0)
    {
        gfx->setCursor(3, 7);

        String TopMessege;
        TopMessege = String(mainOS->InCurrentSong + 1) + " / " + String(mainOS->SongsInTheList);
        gfx->print(TopMessege);
    }

    // Print "MP3 player" label in top right corner
    gfx->setCursor(160, 6);
    gfx->print("Music player");

    // Print current song name at bottom-left of header
    gfx->setCursor(2, 22);
    String fileName = "";
    if (InMusicPlayLoop < 2)
    {
        fileName += "Loading... ";
    }
    else
    {
        fileName += "Playing: ";
    }
    fileName += mainOS->getFileNameFromPath(SongList[mainOS->InCurrentSong]);
    gfx->print(fileName);
    /*     gfx->setCursor(2, 45);
                float percent = (AudioFile.getPos() * 100.0f) / AudioFile.getSize();
    gfx->print(percent); */

    // Disable text wrapping for precise positioning
    /*     gfx->setTextWrap(false);
    gfx->print(fileName); */
    // Draw a thin black line below the header
    gfx->fillRect(0, 6, gfx->width(), 2, TFT_BLACK);

    // Define header height as 45 pixels
    header_height = 45;

    // Enable FFT visualization only if not using e-ink display (non-EPD)
    fft_enabled = !gfx->isEPD();

    // If FFT is enabled, check whether wave visualization should be shown:
    // Not supported on M5UnitLCD board due to limited resolution or rendering capabilities
    if (fft_enabled)
    {
        wave_enabled = (gfx->getBoard() != m5gfx::board_M5UnitLCD);

        // Fill the rest of the screen with gradient-like background colors based on vertical position
        for (int y = header_height; y < gfx->height(); ++y)
        {
            gfx->drawFastHLine(0, y, gfx->width(), bgcolor(gfx, y));
        }
    }
    // draw volume bar
    /*    static int px;                      // Previous x coordinate for volume bar update
       uint8_t v = M5.Speaker.getVolume(); // Get current speaker volume (0-255)
       int x = v * (gfx->width()) >> 8;    // Scale to screen width

       gfx->fillRect(x, 6, px - x, 2, px < x ? 0xAAFFAAu : 0u); // Draw new volume bar
       gfx->display();                                          // Refresh display
       px = x; */

    // Initialize previous FFT bar heights to maximum value (used for drawing animation)
    for (int x = 0; x < (FFT_SIZE / 2) + 1; ++x)
    {
        prev_y[x] = INT16_MAX;
        peak_y[x] = INT16_MAX;
    }

    // Initialize wave data arrays with default values
    for (int x = 0; x < WAVE_SIZE; ++x)
    {
        wave_y[x] = gfx->height(); // Starting Y position of wave line
        wave_h[x] = 0;             // Height of wave segment to draw
    }
}
void MusicPlayerV2::WheneSongLoading()
{
}
void MusicPlayerV2::WheneSongFinishLoadAndPlay()
{
}
void MusicPlayerV2::SetSongTimeByAverage()
{
    if (SecondArray.empty())
        return;

    int32_t counter = 0;
    for (size_t i = 0; i < SecondArray.size(); i++)
    {
        counter += SecondArray[i];
    }

    counter /= SecondArray.size();

    seconds = counter % 60;
    minutes = counter / 60;
}
// Main drawing function that updates the display in real time
void MusicPlayerV2::Draw()
{
    auto gfx = &M5Cardputer.Display; // Get reference to Cardputer's display object

    if (gfx == nullptr)
    {
        return; // Early exit if no valid display exists
    }

    // Run setup once at first frame
    if (first_frame)
    {
        first_frame = false;
        gfxSetup(&M5.Display); // Initialize graphics layout
    }
    // song progress bar (horizontal)
    int barX = 0;
    int barY = 42;
    int barW = 240;
    int barH = 4;

    uint32_t cur = AudioFile.getPos();
    uint32_t total = AudioFile.getSize();
    int fillW = 0;

    if (total > 0)
    {
        // fillW = (int)((uint64_t)cur * (barW - 2) / total);
        fillW = (int)((float)cur / total * (barW - 2));
    }

    gfx->drawFastHLine(barX, barY, barW, BLACK);
    gfx->drawFastHLine(barX, barY, fillW, DARKGREEN);
    // Update volume bar only when not busy with other operations
    if (!gfx->displayBusy())
    {
        static int px;                      // Previous x coordinate for volume bar update
        uint8_t v = M5.Speaker.getVolume(); // Get current speaker volume (0-255)
        int x = v * (gfx->width()) >> 8;    // Scale to screen width

        if (px != x) // Only redraw if volume changed
        {
            gfx->fillRect(x, 6, px - x, 2, px < x ? 0xAAFFAAu : 0u); // Draw new volume bar
            gfx->display();                                          // Refresh display
            px = x;
        }
    }

    // If FFT is enabled and speaker is playing music, draw visualizations
    if (fft_enabled && !gfx->displayBusy() && M5.Speaker.isPlaying(m5spk_virtual_channel) > 1)
    {
        static int prev_x[2]; // Previous X positions for stereo meters
        static int peak_x[2]; // Peak level indicators for stereo channels

        auto buf = out.getBuffer(); // Get audio buffer from output stream

        if (buf)
        {
            memcpy(raw_data, buf, WAVE_SIZE * 2 * sizeof(int16_t)); // Copy stereo data into raw buffer
            gfx->startWrite();                                      // Begin batch write to display for performance

            // Stereo Level Meter Drawing Loop
            for (size_t i = 0; i < 2; ++i)
            {
                int32_t level = 0;
                for (size_t j = i; j < 640; j += 32) // Sample every 32 samples across stereo channel
                {
                    uint32_t lv = abs(raw_data[j]); // Take absolute value of sample
                    if (level < lv)
                    {
                        level = lv;
                    }
                }

                int32_t x = (level * gfx->width()) / INT16_MAX; // Scale level to screen width
                int32_t px = prev_x[i];                         // Previous x position

                if (px != x) // Only redraw if changed
                {
                    gfx->fillRect(x, i * 3, px - x, 2, px < x ? 0xFF9900u : 0x330000u); // Draw level bar
                    prev_x[i] = x;
                }

                px = peak_x[i]; // Get previous peak position
                if (px > x)     // Erase old peak indicator
                {
                    gfx->writeFastVLine(px, i * 3, 2, TFT_BLACK);
                    px--;
                }
                else
                {
                    px = x;
                }

                if (peak_x[i] != px) // Update peak position
                {
                    peak_x[i] = px;
                    gfx->writeFastVLine(px, i * 3, 2, TFT_WHITE); // Draw white peak indicator
                }
            }

            gfx->display(); // Display updated stereo meter

            // Perform FFT calculation on audio data
            fft.exec(raw_data);

            size_t bw = gfx->width() / 60; // Bar width for FFT visualization
            if (bw < 3)
            {
                bw = 3;
            }

            int32_t dsp_height = gfx->height();                  // Full screen height
            int32_t fft_height = dsp_height - header_height - 1; // Available space for FFT bars

            size_t xe = gfx->width() / bw; // Number of FFT bars to draw
            if (xe > (FFT_SIZE / 2))
            {
                xe = (FFT_SIZE / 2);
            }

            // Calculate next wave point based on audio input
            int32_t wave_next = ((header_height + dsp_height) >> 1) +
                                (((256 - (raw_data[0] + raw_data[1])) * fft_height) >> 17);

            uint32_t bar_color[2] = {0x000033u, 0x99AAFFu}; // Background and foreground colors for FFT bars

            // Draw FFT Bars
            for (size_t bx = 0; bx <= xe; ++bx)
            {
                size_t x = bx * bw;

                if ((x & 7) == 0) // Periodically yield control back to scheduler
                {
                    gfx->display(); // Update display during loop
                    taskYIELD();    // Allow other tasks to run
                }

                int32_t f = fft.get(bx);            // Get FFT value at index bx
                int32_t y = (f * fft_height) >> 18; // Normalize FFT output to screen height

                if (y > fft_height)
                {
                    y = fft_height;
                }

                y = dsp_height - y; // Flip Y axis so higher values go upward

                int32_t py = prev_y[bx]; // Previous bar height

                if (y != py) // Redraw only when changed
                {
                    gfx->fillRect(x, y, bw - 1, py - y, bar_color[(y < py)]); // Draw color-coded bar
                    prev_y[bx] = y;
                }

                py = peak_y[bx] + 1; // Adjust peak height to be one pixel higher than previous

                if (py < y)
                {
                    gfx->writeFastHLine(x, py - 1, bw - 1, bgcolor(gfx, py - 1)); // Erase old peak line
                }
                else
                {
                    py = y - 1;
                }

                if (peak_y[bx] != py) // Update peak level
                {
                    peak_y[bx] = py;
                    gfx->writeFastHLine(x, py, bw - 1, TFT_WHITE); // Draw white peak indicator
                }

                // Optional: Draw waveform overlay on top of FFT bars
                if (wave_enabled)
                {
                    for (size_t bi = 0; bi < bw; ++bi)
                    {
                        size_t i = x + bi;
                        if (i >= gfx->width() || i >= WAVE_SIZE)
                        {
                            break;
                        }

                        y = wave_y[i];
                        int32_t h = wave_h[i];
                        bool use_bg = (bi + 1 == bw);
                        if (h > 0)
                        { /// erase previous wave.
                            gfx->setAddrWindow(i, y, 1, h);
                            h += y;
                            do
                            {
                                uint32_t bg = (use_bg || y < peak_y[bx]) ? bgcolor(gfx, y)
                                              : (y == peak_y[bx])        ? 0xFFFFFFu
                                                                         : bar_color[(y >= prev_y[bx])];
                                gfx->writeColor(bg, 1);
                            } while (++y < h);
                        }

                        size_t i2 = i << 1;
                        int32_t y1 = wave_next;
                        wave_next = ((header_height + dsp_height) >> 1) +
                                    (((256 - (raw_data[i2] + raw_data[i2 + 1])) * fft_height) >> 17);
                        int32_t y2 = wave_next;

                        if (y1 > y2)
                        {
                            int32_t tmp = y1;
                            y1 = y2;
                            y2 = tmp;
                        }

                        y = y1;
                        h = y2 + 1 - y;
                        wave_y[i] = y;
                        wave_h[i] = h;

                        if (h > 0)
                        { /// draw new wave.
                            gfx->setAddrWindow(i, y, 1, h);
                            h += y;
                            do
                            {
                                uint32_t bg = (y < prev_y[bx]) ? 0xFFCC33u : 0xFFFFFFu;
                                gfx->writeColor(bg, 1);
                            } while (++y < h);
                        }
                    }
                }
            }

            gfx->display();  // Final display update
            gfx->endWrite(); // End batch write
        }
    }
}

void MusicPlayerV2::OnExit()
{
    // if (!isPlaying)
    //{
    // mainOS->DeleteMusicPlayer=true;
    //}
}