#define doThis(n) for (int _Myi = 0; _Myi < (n); _Myi++)

#include <WiFi.h>
#include "FS.h"
#include <SPI.h>
#include <vector>
#include <cstdint>

#include "OpenFontRender.h"
OpenFontRender ofr;
char sizingChars[] = "ABCabcjyWM";
#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define SkyBlue RGB565(0x00, 0xe8, 0xff)
int dispWidth;
int dispHeight;
int xCenter;
int yCenter;
unsigned long loopWait;
#define dispLine1  10
#define dispLine2  60
#define dispLine3 110
#define dispLine4 160
#define dispLine5 210
#define dispLine6 260
#define DATE_FONT_SIZE  80
#define TIME_FONT_SIZE 130
#define TIME_LOC   3  //y location on display
#define DATE_LOC 148  //y location on display
#define RL_LOC   147  //y location on display
#define chrBufSize 250
#define SENSOR_PIN 34  // GPIO 34 (ADC1)

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite rainbow1LineSprite = TFT_eSprite(&tft);  // Single line sprite
TFT_eSprite spriteDate = TFT_eSprite(&tft);  // For punchthrough use
TFT_eSprite spritePic = TFT_eSprite(&tft);  // The picture for punchthrough

#include <WiFi.h>
#include "FS.h"
#include <SPI.h>
#include <vector>
#include <cstdint>

#include "WiFiManager.h"    // https://github.com/tzapu/WiFiManager
WiFiManager wifiManager;

const char* myPortalName = "ColorTest";

#include "Free_Fonts.h"
#include "My_Fonts.h"
#include "TimeLib.h"
#include "esp_sntp.h"
// Include the jpeg decoder library
//#include <TJpg_Decoder.h>
#include "TwoJPGs.h"

struct TextMetrics {
  int width;
  int height;
  int baselineOffset;
};
TextMetrics getTextMetrics(const char *text, int fontSize,
                           int descenderPad = 12);

bool     colonOn;
int      iYear, prevHour = -1;
int      yPos;
int      spriteDateW, spriteDateH;
int      displayMode = 1;
int      punchXOffset1 = 0, punchYOffset1 = 0;  // Upper text sprite
int      punchXOffset2 = 0, punchYOffset2 = 0;  // Lower text sprite

#define DM_FULL_SHIMMER   1  // Full Shimmer
#define DM_RL_SHIMMER     2  // Rainbow Line only
#define DM_STATIC_DIGITS  3  // Digits don't change.
#define DM_WHOLE_DIGIT    4  // All segments Shimmer in 1 color
int      inStrCt = 0;
uint16_t spriteW, spriteH;
time_t   workTime_t;
unsigned long startMillis;
uint16_t solidColor;
struct   tm * tm;
char     workTime[100], workDate[100];
String   myBuildString;
String   sTemp, sVer;
String   upperString, lowerString;

const char *testDay = "Wednesday";  // Max width tester

#define RAINBOW_SIZE 360
uint16_t rainbowTable[RAINBOW_SIZE];

// Digit → segment map
int digitSegments[10][7] = {
  {5, 4, 0,    3, 1, 2},  // 0  // 0, 1, 2, 3, 4, 5
  {               1, 2},  // 1  //    1, 2
  {   4, 0, 6, 3, 1   },  // 2  // 0, 1,    3, 4,    6
  {      0, 6, 3, 1, 2},  // 3  // 0, 1, 2, 3,       6
  {5,       6,    1, 2},  // 4  //    1, 2,       5, 6
  {5,    0, 6, 3,    2},  // 5  // 0,    2, 3,    5, 6
  {5, 4, 0, 6, 3,    2},  // 6  // 0,    2, 3, 4, 5, 6
  {      0,       1, 2},  // 7  // 0, 1, 2
  {5, 4, 0, 6, 3, 1, 2},  // 8  // 0, 1, 2, 3, 4, 5, 6
  {5,    0, 6, 3, 1, 2}   // 9  // 0, 1, 2, 3,    5, 6
};
//                    0  1  2  3  4  5  6  7  8  9
int digitCount[10] = {6, 2, 5, 5, 4, 5, 6, 3, 7, 6};

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
int ihourlyBrilliance[] = {
  //0   1    2     3    4    5
  40,  20,  10,   10,  10,  10,  //  0- 5
  //6   7    8     9   10   11
  20,  30,  40,   60, 80,  100,  //  6-11
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
