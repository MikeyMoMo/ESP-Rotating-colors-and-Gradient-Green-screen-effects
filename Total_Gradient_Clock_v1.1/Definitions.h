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

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite rainbow1LineSprite = TFT_eSprite(&tft);  // Single line sprite
TFT_eSprite spriteDate = TFT_eSprite(&tft);  // For punchthrough use

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

struct TextMetrics {
  int width;
  int height;
  int baselineOffset;
};
TextMetrics getTextMetrics(const char *text, int fontSize,
                           int descenderPad = 12);

bool   colonOn;
int    iYear, prevHour = -1;
int    yPos;
int    spriteDateW, spriteDateH;
uint16_t spriteW, spriteH;
time_t   workTime_t;
unsigned long startMillis;
struct tm * tm;
char   workTime[100], workDate[100];
String myBuildString;

const char *testDay = "Wednesday";  // Max width tester

#define RAINBOW_SIZE 360
uint16_t rainbowTable[RAINBOW_SIZE];

// Digit → segment map
int digitSegments[10][7] = {
  {0, 1, 2, 3, 4, 5}, // 0
  {1, 2},             // 1
  {0, 1, 6, 4, 3},    // 2
  {0, 1, 6, 2, 3},    // 3
  {5, 6, 1, 2},       // 4
  {0, 5, 6, 2, 3},    // 5
  {0, 5, 6, 2, 3, 4}, // 6
  {0, 1, 2},          // 7
  {0, 1, 2, 3, 4, 5, 6}, // 8
  {0, 1, 2, 3, 5, 6}  // 9
};
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
