# AdvanceOS
Music Player<br>
<img src="gitFiles/screen_105219.bmp?raw=true">
<img src="gitFiles/screen_71496.bmp?raw=true"><br>
File explorer <br>
<img src="gitFiles/screen_9116.bmp?raw=true">
<img src="gitFiles/screen_97751.bmp?raw=true"><br>
Painter V2<br>
<img src="gitFiles/PainterV2.bmp?raw=true"><br>use bucket tool and premitive shapes<br>
<img src="gitFiles/PAint example1.bmp?raw=true"><br>
<img src="gitFiles/screen_100373.bmp?raw=true">
<img src="gitFiles/PAint example2.bmp?raw=true"><br>
Infrared Emitter And Editor<br>
<img src="gitFiles/screen_52224.bmp?raw=true"><br>
Talk to (not) AI chatbot<br>
<img src="gitFiles/screen_11139.bmp?raw=true"><br>
Music Composer<br>
<img src="gitFiles/screen_29817.bmp?raw=true">
<img src="gitFiles/screen_311869.bmp?raw=true"><br>
3D renderer (OBJ files)<br>
<img src="gitFiles/screen_21074.bmp?raw=true">
<img src="gitFiles/screen_34278.bmp?raw=true"><br>
Games (ARDUBOY NES GBC games) <br>
<img src="gitFiles/screen_153126.bmp?raw=true"><br>
Wifi Spectrum<br>
<img src="gitFiles/screen_27807.bmp?raw=true"><br>
Partition Viewer<br>
<img src="gitFiles/screen_42323.bmp?raw=true"><br>
Loan Calculator<br>
<img src="gitFiles/screen_19291.bmp?raw=true"><br>




AdvanceOS is a lightweight operating system for the Cardputer ADV, designed primarily for media playback and file browsing.
Currently, it supports MP3 and WAV playback. In the future, Text editor and lots of cool stuff!
Upcoming features include text viewing and editing, as well as image display capabilities.

# Supported Files

* MP3 WAV : Music files
* IR : Infrared code files (to emit infrared code)
* GIF PNG JPEG BMP (24 Bit only): Picture files
* TXT JSON CONF : Text files (i can add more files type if you like. you can ask in issues Section)
* OBJ MTL : 3D Model files
* ESPRC : ESP remote control files to control other esp projects remotely See WIKI
* ARD : Arduboy game
* NES : NES games
* GBC GB : Old Gameboy and Gameboy Color games


## 🚀 Installation Options

#### * Option 1: Manual Flashing
Download the latest .bin file from the Releases section and flash it using the ESP Flash Download Tool.
Instructions and download link: [ESP Flash Download Tool](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html)
#### * Option 2: Recommended – M5Burner
Use M5Burner:
Select Cardputer from the sidebar, search for AdvanceOS, and install it directly.
#### * Option 3: Using the Launcher
If you already have the Launcher installed on your Cardputer, you can install AdvanceOS via SD card:
 Download the latest .bin file
 Copy it to your SD card
 Use the Launcher to install it
More info: [Launcher GitHub](https://github.com/bmorcelli/Launcher)


# 🎮 How to Use
- A microSD card is required. The file system must be FAT or FAT32. NTFS and exFAT are not supported. (New card (Fast 
How to format to FAT https://www.instructables.com/Format-USB-Flash-Drive-to-FATFAT16-not-FAT32/
- Navigation is simple:
- ESC → Go back
- Enter → Select
  
## In file system
- ENTER open file menu
- -> fast execute file

## Painter !!NEW!! V2 is out 14/4/26
<img src="gitFiles/PainterV2.bmp?raw=true"><br>

- Arrows move courser,
- ESC open menu (you can see al shortcut in this menu)
- ENTER Paint / Accept in menu
- +/- change brush size
- P Change Color
- B brush mode
- L line mode
- T triangle mode
- I Eyedropper mode
- W Paint Word (depends on brush size try 1 2 or 3)
- C circle mode
- G bucket mode (draw on in color area)
- R Rectangle mode
- E ellipse mode
- F toggle fill mode (if not fill only draw outline)
- S quick save
- A reload image Again from file (good for UNDO)
- O outline for specific color select the color with the mouse,(the outline size by brush size and the outline color by current color)<br>
  Outline for specific color (purple)<br>
   <img src="gitFiles/screen_106742.bmp?raw=true">
      <img src="gitFiles/screen_428194.bmp?raw=true">
<br>Outline All (From Menu)<br>
 <img src="gitFiles/screen_155461.bmp?raw=true"><br>

- 2 and 3 Buttons Zoom In Or Out (Grid Will Appear in zoom 5. It's for pixel art editing)<br>
 <img src="gitFiles/screen_218137.bmp?raw=true"><br>

## Music Composer BETA (needs to be improved)
- S KEY to save
- A key to reload from the file
- \ KEY to put or delete Note
- ENTER to play your masterpiece (From the start)
- SPACE BAR to play from current Screen (good for long song for not play all the song from the start)
- Left Arrow Keys D, F, G, T,: move notes from the Right of the courser to any direction (good to fix lot of notes if not in place)

## Games
- ESC exit the game and back to file explorer 
- Arrow key
- Z is A button
- X is B button
- ENTER is START button
- BACKSPACE / DEL button is SELECT button
- in emulator NES or GBC also "L" and "K" "1" and "2" and "WASD" keys suppurted

## Media Player
- [ back 7 second
- ] forward 7 second
- -> move next song
- <- move back song
- PLUS key + or MINUS key _ to change volume
- BETA press 1-5 KEYS to change Equalizer The Default Is Always 1  (Some jarring in the headphones Tell Me What You Prefer In Issues ), 

## Password Vault
- DEL delete entry
- Plus key to Add Entry

## 3D renderer
* use file 3d file example from 3D_MODEL folder or create yourself in 3D software like BLENDER
* 1 key normal view press again to change beckground
* 2 key Grey view like Blendrer software
* 3 key wireframe view
- Arrow keys to rotate
- E S D A move camera , arrows key rotate camera
- W or R keys move up or down

# 🛠️ Current Features Quick Explanation 
- [X] 3d Renderer , it can play small OBJ files, you can use models in 3D_MODEL folder (it renders only simple materials like colors in faces not Texture!, the materials is in MTL files near the OBJ file, they must be the same name to load the materials), you can create 3d model in blender, export as OBJ, you can assign simple shader like color and assign to the faces, if the load freeze maybe it's too large file, (FPS added to see how the CARDPUTER can handle it)
- [X] File management: delete, copy, files 
- [X] Text editor: view create and edit .txt .json .conf files
- [x] Paint draw new pictures or edit old pictures Can Open BMP file Or PNG and GIF you can choose any frame you want from the gif ( i can't do UNDO function like ctrl+z, the cardputer don't have much RAM you can save in S and Do risky thing . and if it's not good Reaload the image from file by press A)
- [X] Timer functionality
- [X] Step Counter!
- [X] Loan Calculator. calculate Loan by intrest ,loan amount and Time
- [X] Music Creator You can creat your old retro music , save the project, and export it as WAV file
- [x] Alarm Clock. set the current time and wake up time, the carputer calculate the time and Enter DEEP SLEEP mode to save battery life After he wake up (you can wake him up manualy by press RESET After RESET you need to set the alarm again) he will sound the alarm and turn off himself (while the cardputer in deep sleep mode don't turn off the power switch or it will never sound the alarm)
- [x] Image viewer (PNG ,GIF,JPEG) can zoom in and out and pan image by arrow keys and +/- keys(need small size PIC limited by Cardputer RAM)
- [x] GIF viewer work with animation (needs small GIFs, the Cardputer RAM is limited)
- [x] Notes for daily use
- [x] Voice Recorder
- [x] ESP project remote control see more info [here](https://github.com/bomberman30/AdvanceOS-for-cardputer/wiki)
- [ ] Radio (Maybe)
- [x] Password Vault, Store all your passwords in encrypted file! (if you want to reset the vault, you must delete the file inside "AdvanceOS/Vault" folder, it will delete the stored passwords as well. so remember your vault password!)
- [x] IR Sender Put IR files in sd card and send via the cardputer you can find IR files [here](https://github.com/Lucaslhm/Flipper-IRDB)
- [X] IR Editor you can rename command name and edit the actual signal by connecting IR reciver to pin 1 (or use m5stack IR REMOTE chip) use names like UP, DOWN , UP, RIGHT,POWER, OK ,VOL+ VOL-, MUTE to make it appears as special key in the sender ( like arrows etc...) the special keys bind yo keys in the cardputer like the cardputer arrows
- [x] Screenshot press Button G0 (on the top) to take a screenshot (will be saved in AdvanceOS folder)
- [x] Custom theme you can create themes and select them ! enter extra menu-> theme manager You can add custom picture to every Menu download TEST_TEHEME folder and put in SD card Activate it in file explorer, you can edit the PNG files as you like and edit the theme file via the cardputer itself
- [X] Games! you can Download Arduboy games or NES or GBC games (you need to put "Emulator.extension" file in "AdvanceOS" folder in your SD card) (put the games in "Games" folder in the root of the SD card to see the games in GAME menu, or just run the games via file explorer) if you install AdvanceOS via launcher you need to change partition type to anable the games to work!!!! the OS will ask it Automatic to change if you enter to Games in main menu.
- you can add picture to a game in game menu!! 
How to add:
if your game is MARIO.nes Create PNG file and rename it MARIO.png
the image must be 135X135 size and PNG format, the image needs to be in the same "Games" folder

#💡 Suggestions are Welcome!
If you have practical ideas or feature requests, feel free to share! here https://github.com/bomberman30/AdvanceOS-for-cardputer/issues
