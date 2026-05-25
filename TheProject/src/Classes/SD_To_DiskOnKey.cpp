#include "SD_To_DiskOnKey.h"
#include "MyOS.h"
#include <USB.h>
#include <USBMSC.h>
    USBMSC msc;

void displayMessage(String message)
{

    M5.Lcd.fillRect(63, 28, 10, 17, TFT_BLACK);
    M5.Lcd.setCursor(66, 30);
    M5.Lcd.print(message.c_str());

}
void drawLedIndicator(bool plugged)
{

    M5.Lcd.fillRoundRect(54, 60, 5, 30, 5, plugged ? GREEN : RED);

}
int32_t usbWriteCallback(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    
    // Verify freespace
    uint64_t freeSpace = SD.totalBytes() - SD.usedBytes();
    if (bufsize > freeSpace)
    {
        return -1; // no space available
    }

    // Verify sector size
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0)
        return -1; // disk error

    // Write blocs
    for (uint32_t x = 0; x < bufsize / secSize; ++x)
    {
        uint8_t blkBuffer[secSize];
        memcpy(blkBuffer, buffer + secSize * x, secSize);
        if (!SD.writeRAW(blkBuffer, lba + x))
        {
            return -1; // write error
        }
    }
    return bufsize;

}




void drawUSBStickIcon()
{

    // Port USB
    M5.Lcd.fillRoundRect(155, 55, 40, 40, 5, LIGHTGREY);
    // Body
    M5.Lcd.fillRoundRect(45, 50, 112, 50, 5, DARKCYAN);
    // Small square on port
    M5.Lcd.fillRoundRect(164, 63, 20, 10, 5, DARKGREY);
    M5.Lcd.fillRoundRect(164, 78, 20, 10, 5, DARKGREY);

}

int32_t usbReadCallback(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{

    // Verify sector size
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0)
        return -1; // disk error

    // Read blocs
    for (uint32_t x = 0; x < bufsize / secSize; ++x)
    {
        if (!SD.readRAW(reinterpret_cast<uint8_t *>(buffer) + (x * secSize), lba + x))
        {
            return -1; // read error
        }
    }
    return bufsize;

}







bool usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject)
{
    return true;
}







void SD_To_DiskOnKey::Begin()
{
showTopBar=false;


 // USB MSC;

    M5Cardputer.Display.fillScreen(YELLOW);
    uint32_t secSize = SD.sectorSize();
    uint32_t numSectors = SD.numSectors();
    msc.vendorID("ESP32");
    msc.productID("USB_MSC");
    msc.productRevision("1.0");
    msc.onRead(usbReadCallback);
    msc.onWrite(usbWriteCallback);
    msc.onStartStop(usbStartStopCallback);
    msc.mediaPresent(true);
    msc.begin(numSectors, secSize);

    USB.onEvent([](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
                {
    if (event_base == ARDUINO_USB_EVENTS) {
      auto* data = reinterpret_cast<arduino_usb_event_data_t*>(event_data);
      switch (event_id) {
        case ARDUINO_USB_STARTED_EVENT:
          drawLedIndicator(true);
          break;
        case ARDUINO_USB_STOPPED_EVENT:
          drawLedIndicator(false);
          break;
        case ARDUINO_USB_SUSPEND_EVENT:
          displayMessage("USB suspend");
          break;
        case ARDUINO_USB_RESUME_EVENT:
          displayMessage("USB resume");
          break;
        default:
          break;
      }
    } });

    displayMessage("USB Stick is ready");
    drawUSBStickIcon();
    drawLedIndicator(false);
    USB.begin();

}

void SD_To_DiskOnKey::Loop()
{
        if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        ESP.restart();
    }
}

void SD_To_DiskOnKey::Draw()
{
}




