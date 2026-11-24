#pragma once
/*============================================================================
  Definitions.h  -  libraries, global variables, constants and wiring notes
  ----------------------------------------------------------------------------
  Everything here is shared by all the .ino tabs.  (The Arduino IDE glues
  all .ino tabs into one file, so globals defined here are visible
  everywhere.)

  REVIEW: This header DEFINES variables (not just declares them) and has no
          include guard.  Fine while only the main .ino includes it, but if
          it is ever included twice you get "multiple definition" errors.
          Add  #pragma once  as the first line.
  ==========================================================================*/

//----------------------------------------------------------------------------
// Small helper macros
//----------------------------------------------------------------------------
// iIN_RANGE: is v between low and high, INCLUSIVE?  Handles wrap-around
//            ranges too, e.g. iIN_RANGE(hour, 22, 6) is true for 22..23,0..6.
// xIN_RANGE: same idea but EXCLUSIVE of the end points.
// REVIEW: Neither macro is used anywhere at the moment.
#define iIN_RANGE(v, low, high) (((low) <= (high)) ? ((v) >= (low) && (v) <= (high)) : ((v) >= (low) || (v) <= (high)))
#define xIN_RANGE(v, low, high) (((low) <= (high)) ? ((v) > (low) && (v) < (high)) : ((v) > (low) || (v) < (high)))
// doThis(3) { ... }  repeats the block 3 times.  Used by the HourDances.
#define doThis(n) for (int _Myi = 0; _Myi < (n); _Myi++)

//----------------------------------------------------------------------------
// Libraries
//----------------------------------------------------------------------------
#include <WiFi.h>
#include "FS.h"
#include <SPI.h>
#include <vector>
#include <cstdint>

// OpenFontRender: draws TrueType (.ttf) fonts at any size, with
// anti-aliasing.  Used for all the "word" text (splash screen, day, date).
#include "OpenFontRender.h"
OpenFontRender ofr;
// Preferences: saves small settings in the ESP32's flash (survives reboot).
#include <Preferences.h>
Preferences prefs;

//----------------------------------------------------------------------------
// Screen layout and colours
//----------------------------------------------------------------------------
char sizingChars[] = "ABCabcjyWM";   // Tall + descending letters for sizing.
// Build a 16-bit RGB565 colour from 8-bit R, G, B values (0..255).
#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define SkyBlue RGB565(0x00, 0xe8, 0xff)     // Splash-screen background.
int dispWidth;                       // Set in setup(): 480
int dispHeight;                      // Set in setup(): 320
//int xCenter;                         // REVIEW: set but never used.
//int yCenter;                         // REVIEW: set but never used.
unsigned long loopWait;              // Counter for the 3 s wait in setup().
// Y positions (pixels from top) of the text lines on the splash screens.
#define dispLine1  10
#define dispLine2  60
#define dispLine3 110
#define dispLine4 160
#define dispLine5 210
#define dispLine6 260
#define DATE_FONT_SIZE  80   // Day / date text size (ofr font size).
//#define TIME_FONT_SIZE 130   // REVIEW: unused - the time font is fixed at
//         140 by the DSEG7_Classic_Bold140 data.
#define TIME_LOC   3  //y location on display  (top of the big digits)
#define DATE_LOC 148  //y location on display  (top of the day-name line)
#define RL_LOC   147  //y location on display  (the 1-px rainbow line)
#define chrBufSize 250       // Max length of a typed serial command.
#define SENSOR_PIN 34  // GPIO 34 (ADC1)  - future light sensor, unused now.

/*****************************************************************************/
// Segment activity bitmasks for digits 0–9
// Each bit (0–6) corresponds to a segment; 1 = active, 0 = inactive
//   bit0 = top, bit1 = upper right, bit2 = lower right, bit3 = bottom,
//   bit4 = lower left, bit5 = upper left, bit6 = middle
//   (Read right-to-left: 0b0000110 = bits 1 and 2 = the "1" digit.)
// Used by drawDigitCached() in AllScroll.ino.
const uint8_t digitMasks[10] = {
  0b0111111, // 0
  0b0000110, // 1
  0b1011011, // 2
  0b1001111, // 3
  0b1100110, // 4
  0b1101101, // 5
  0b1111101, // 6
  0b0000111, // 7
  0b1111111, // 8
  0b1101111  // 9
};

//----------------------------------------------------------------------------
// Display driver and sprites
//   Pin numbers and the driver chip are set in TFT_eSPI's User_Setup.h
//   (in the library folder), NOT here.
//----------------------------------------------------------------------------
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite rainbow1LineSprite = TFT_eSprite(&tft);  // Single line sprite
TFT_eSprite spriteDate = TFT_eSprite(&tft);  // For punchthrough use
TFT_eSprite spritePic = TFT_eSprite(&tft);  // The picture for punchthrough

// REVIEW: These five includes are exact duplicates of the ones at the top.
//         Harmless (they have their own guards) but can be deleted.
//#include <WiFi.h>
//#include "FS.h"
//#include <SPI.h>
//#include <vector>
//#include <cstdint>

// WiFiManager: if no saved WiFi works, the ESP32 becomes an access point
// called myPortalName; join it with a phone and pick your network.
#include "WiFiManager.h"    // https://github.com/tzapu/WiFiManager
WiFiManager wifiManager;

const char* myPortalName = "ColorTest";

//#include "Free_Fonts.h"   // REVIEW: no Free Fonts are used - can be removed.
#include "My_Fonts.h"     // TTF fonts + the 7-segment smooth font.
#include "TimeLib.h"      // Only used for dayStr() and monthShortStr().
#include "esp_sntp.h"     // ESP32 network time.
// Include the jpeg decoder library
//#include <TJpg_Decoder.h>
#include "TwoJPGs.h"      // The two 434x80 pictures.

// Result of measuring a piece of text (see getTextMetrics in Utilities).
struct TextMetrics {
  int width;
  int height;
  int baselineOffset;
};
TextMetrics getTextMetrics(const char *text, int fontSize,
                           int descenderPad = 12);

//----------------------------------------------------------------------------
// Run-time state
//----------------------------------------------------------------------------
bool     colonOn;                 // Colon visible this second?
// REVIEW: This global prevHour is NEVER updated (UpdateDisplayAS() has its
//         own "static int prevHour" that hides it).  punchImageMoving()
//         reads THIS one, so its early-exit test never works.  Use one
//         prevHour only.
int      iYear, prevHour = -1;
int      yPos;                    
int      spriteDateW, spriteDateH;   // Size of the date sprite (setup()).
int      displayMode = 1;         // One of the DM_ values below.
// How far the picture has scrolled inside the day-name text.
int      punchXOffset1 = 0, punchYOffset1 = 0;  // Upper text sprite
//int      punchXOffset2 = 0, punchYOffset2 = 0;  // Lower text sprite (unused)

// Display modes (chosen over serial with 1..4, saved in flash).
#define DM_FULL_SHIMMER   1  // Full Shimmer
#define DM_RL_SHIMMER     2  // Rainbow Line only
#define DM_STATIC_DIGITS  3  // Digits don't change.
#define DM_WHOLE_DIGIT    4  // All segments Shimmer in 1 color

int      inStrCt = 0;             // Characters read after a serial command.
// Cached masks for each digit position (4 digits in your clock)
// Remembers which segments are currently lit, so a segment that goes out
// (e.g. 8 -> 0, the middle bar) can be painted black.
uint8_t currentMask[5] = {0};
//uint16_t spriteW, spriteH;        // REVIEW: unused.
time_t   workTime_t;              // Current time, seconds since 1970 (UTC).
unsigned long startMillis;        // For optional frame-time measurement.
uint16_t mode3Color = 1;          // Digit colour for mode 3 (from flash).
// Broken-down local time (hour, minute, weekday...).  Updated every frame.
// REVIEW: Naming a variable "tm" - the same as the type "struct tm" - is
//         legal but confusing.  Something like "now" or "tNow" is clearer.
struct   tm * tm;
char     workTime[100], workDate[100];   // Scratch text buffers.
String   myBuildString;
String   sTemp, sVer;                    // sVer = version from file name.
String   upperString, lowerString;       // "Monday" and "Nov 23rd".

const char *testDay = "Wednesday";  // Max width tester

// 360 pre-computed rainbow colours, one per degree of hue.  Built once by
// buildRainbowTable(); much faster than calling sin() for every pixel.
#define RAINBOW_SIZE 360
uint16_t rainbowTable[RAINBOW_SIZE];

// Rearrangement to try to make more random colors.
// Found another way.
// Digit → segment map
// REVIEW: digitSegments / digitCount are only used by drawDigitAS(), which
//         is no longer called (drawDigitCached + digitMasks replaced it).
//         They can be removed together.
//int digitSegments[10][7] = {
//  {5, 4, 0,    3, 1, 2},  // 0  // 0, 1, 2, 3, 4, 5
//  {               1, 2},  // 1  //    1, 2
//  {   4, 0, 6, 3, 1   },  // 2  // 0, 1,    3, 4,    6
//  {      0, 6, 3, 1, 2},  // 3  // 0, 1, 2, 3,       6
//  {5,       6,    1, 2},  // 4  //    1, 2,       5, 6
//  {5,    0, 6, 3,    2},  // 5  // 0,    2, 3,    5, 6
//  {5, 4, 0, 6, 3,    2},  // 6  // 0,    2, 3, 4, 5, 6
//  {      0,       1, 2},  // 7  // 0, 1, 2
//  {5, 4, 0, 6, 3, 1, 2},  // 8  // 0, 1, 2, 3, 4, 5, 6
//  {5,    0, 6, 3, 1, 2}   // 9  // 0, 1, 2, 3,    5, 6
//};
//                    0  1  2  3  4  5  6  7  8  9
//int digitCount[10] = {6, 2, 5, 5, 4, 5, 6, 3, 7, 6};

/*
   // Classic C array: compiler only knows 7 slots
  int digitSegments[10][7] = {
  {0,1,2,3,4,5}, // 0
  {1,2},         // 1
  ...
  };
  int digitCount[10] = {6,2,...}; // manual count

  // Modern C++ vector: runtime knows the size
  std::vector<std::vector<int>> digitSegmentsVec = {
  {0,1,2,3,4,5}, // 0
  {1,2},         // 1
  ...
  };
  // Now you can just use digitSegmentsVec[d].size()
*/

// Backlight PWM level (0..255) for each hour of the day, applied at the
// top of every hour.  Dim at night, brightest mid-afternoon.
int ihourlyBrilliance[] = {
  //0   1    2     3    4    5
  40,  20,  20,   20,  20,  20,  //  0- 5
  //6   7    8     9   10   11
  30,  40,  50,   60, 80,  100,  //  6-11
  //12  13   14   15   16   17
  120, 140, 160, 160, 160, 160,  // 12-17
  //18  19   20   21   22   23
  140, 120, 100,  80,  60,  50   // 18-23
};

// ILI9488
// Pins I used for this, screen and touch from the left as labeled
//    by the card.
//    01 - VCC   - 3.3v   (Guess)
//    02 - GND   - Ground (One more guess)
//    03 - CS    - D5     (Display chip select)
//    04 - Reset - D22    (Display reset)
//    05 - DC    - D33    (Data/Command switchover)
//    06 - MOSI  - D23(D) (Display data from the ESP board)
//    07 - SCK   - D18(D) (SPI Clock)
//    08 - LED   - D4     (Backlight control PWM pin)
//    09 - MISO  - xx     (This is display MISO which is not used)
//    10 - TCLK  - D18    (Parallel with display SCK)
//    11 - TCS   - D21    (Touch Chip Select aka, TOUCH_CS)
//    12 - TDIN  - D23    (Touch MOSI. Parallel with display MOSI)
//    13 - TD0   - D19    (This is touch MISO)
//

// XIAO v1
// Pins I used for this, screen and touch from the left as labeled
//    by the card.
//    01 - VCC   - 3.3v   (Guess)
//    02 - GND   - Ground (One more guess)
//    03 - CS    - D7     (Display chip select)
//    04 - Reset - D9     (Display reset)
//    05 - DC    - D8     (Data/Command switchover)
//    06 - MOSI  - D6(D)  (Display data from the ESP board)
//    07 - SCK   - D5(D)  (SPI Clock)
//    08 - LED   - D4     (Backlight control PWM pin)
//    09 - MISO  - -1     (This is display MISO which is not used)
//    10 - T_CLK - D5(D)  (Parallel with display SCK)
//    11 - T_CS  - D10    (Touch Chip Select aka, TOUCH_CS)
//    12 - T_DIN - D6(D)  (Touch MOSI. Parallel with display MOSI)
//    13 - T_D0  - D12    (This is touch MISO)
//    14 - T_IRQ - D11    (This is touch IRQ)
