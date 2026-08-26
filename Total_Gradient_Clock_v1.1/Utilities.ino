/*****************************************************************************/
void UpdateDisplay()
/*****************************************************************************/
{
  static int offset = 0, rlOffset = 0;
  static int    tempHour;
  static int    upperShiftOffset = 0;
  static int    lowerShiftOffset = 0;
  static bool   punchSwitch = true;
  static int    upperShift, lowerShift;
  static int    whichDance;
  static String upperString, lowerString;

#define TIME_LOC   3
#define DATE_LOC 148
#define RL_LOC   147

  if (prevHour != tm->tm_hour) {  // || zoomy) {
    tft.fillScreen(TFT_BLACK);
  }

  safeLoadOFR_Font("DSEG7ClassicBoldSegmentsTTF",
                   DSEG7ClassicBoldSegmentsTTF,
                   sizeof(DSEG7ClassicBoldSegmentsTTF),
                   __LINE__, __func__);
  ofr.setFontSize(TIME_FONT_SIZE);

  // Convert 24 hour time to 12 hour time.
  tempHour = tm->tm_hour;
  if (tempHour == 0) tempHour = 12;   // Midnight
  if (tempHour > 12) tempHour -= 12;  // 1:00 pm through 11:59 pm

  /* Testing for spacing */
  //tempHour = 10;
  /* End Testing */

  sprintf(workTime, "%02d:%02d", tempHour, tm->tm_min);

  // Full OFR time for drawstring is 120 ms.
  // Using tft writes takes 68 ms.
  //  Serial.printf("tempHour %d\r\n", tempHour);
  if (tempHour > 9)
    drawMyString(workTime, -30, TIME_LOC, offset);  //10 - 12
  else
    drawMyString(workTime, 50, TIME_LOC, offset);  // 0 -  9

  // Animate rainbow shimmer on the digit segments.
  offset += 5; if (offset >= 360) offset = 0;

  // The following makes the line move to the left.
  //  rlOffset += 5; if (rlOffset >= 360) rlOffset = 0;
  // The following two make the line move to the right.
  rlOffset -= 5; if (rlOffset < 0) rlOffset += 360;
  drawRainbowLine(RL_LOC, rlOffset);

  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF),
                   __LINE__, __func__);

  // Every time through, move the colors either left or right.
  ofr.setFontSize(DATE_FONT_SIZE);
  punchSwitch = !punchSwitch;
  if (punchSwitch) {
    upperShift = (upperShift + 5) % spriteDate.width();
    spriteDate.fillSprite(TFT_BLACK);
    ofr.setCursor(tft.width() / 2, 0);
    ofr.cprintf(upperString.c_str());
    punchRainbowShift(spriteDate, rainbowTable, RAINBOW_SIZE,
                      upperShift);
    spriteDate.pushSprite(0, DATE_LOC);
  } else {
    lowerShift = (lowerShift - 5);
    if (lowerShift < 0) lowerShift = spriteDate.width();
    spriteDate.fillSprite(TFT_BLACK);
    ofr.setCursor(tft.width() / 2, 0);
    ofr.cprintf(lowerString.c_str());
    punchRainbowShift(spriteDate, rainbowTable, RAINBOW_SIZE,
                      lowerShift);
    spriteDate.pushSprite(0, DATE_LOC + spriteDateH);
  }

  // On hour change, recreate the bottom two text fields.
  if (prevHour != tm->tm_hour) {  // || zoomy) {
    safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                     sizeof(BritanicBoldTTF),
                     __LINE__, __func__);

    spriteDate.fillSprite(TFT_BLACK);
    upperString = dayStr(tm->tm_wday + 1);
    ofr.setFontColor(TFT_YELLOW, TFT_BLACK);
    ofr.setFontSize(DATE_FONT_SIZE);  // Redundant, really.
    ofr.setCursor(tft.width() / 2, 0);
    ofr.cprintf(upperString.c_str());
    punchRainbowShift(spriteDate, rainbowTable, RAINBOW_SIZE, 0);
    spriteDate.pushSprite(0, DATE_LOC);

    spriteDate.fillSprite(TFT_BLACK);
    lowerString =  monthShortStr(uint8_t(tm->tm_mon + 1));
    lowerString += " ";
    lowerString += String(tm->tm_mday);
    lowerString += th(tm->tm_mday);
    ofr.setCursor(tft.width() / 2, 0);
    ofr.cprintf(lowerString.c_str());
    punchRainbowShift(spriteDate, rainbowTable, RAINBOW_SIZE, 0);
    spriteDate.pushSprite(0, DATE_LOC + spriteDateH);

    HourDance(random(16));
    // HourDance(tm->tm_hour);
    // for (whichDance = 0; whichDance < 16; whichDance++) {
    //   doThis(2) {
    //     HourDance(whichDance);
    //     delay(2000);
    //   }
    // }
    Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                  tm->tm_hour, ihourlyBrilliance[tm->tm_hour]);
    ledcWrite(TFT_BL, ihourlyBrilliance[tm->tm_hour]);
    prevHour = tm->tm_hour;
  }
}
/*****************************************************************************/
void punchRainbowShift(TFT_eSprite & spr, uint16_t *rainbowLine,
                       int lineWidth, int shiftOffset)
/*****************************************************************************/
{
  //  Serial.printf("Calling punchRainbowShift with offset %d\r\n",
  //                shiftOffset);
  static int hueIndex;
  int w = spr.width();
  int h = spr.height();

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint16_t c = spr.readPixel(x, y);
      if (c != TFT_BLACK) {
        // Map sprite X into rainbow table range, then add shift
        hueIndex = ((x - shiftOffset + w) * lineWidth) / w;
        spr.drawPixel(x, y, rainbowLine[hueIndex % lineWidth]);
      }
    }
  }
}
/*****************************************************************************/
void punchImage(TFT_eSprite & sprText, TFT_eSprite & sprImage)
/*****************************************************************************/
{
  for (int y = 0; y < sprText.height(); y++) {
    for (int x = 0; x < sprText.width(); x++) {
      uint16_t c = sprText.readPixel(x, y);
      if (c != TFT_BLACK) {
        // Map pixel from image sprite
        uint16_t imgColor = sprImage.readPixel(x % sprImage.width(),
                                               y % sprImage.height());
        sprText.drawPixel(x, y, imgColor);
      }
    }
  }
}
/*****************************************************************************/
// Return rainbow color at given index+offset
uint16_t rainbowColor(int index, int offset)
/*****************************************************************************/
{
  int hue = (index * 40 + offset) % 360;
  return tft.color565(
           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
         );
}
/*****************************************************************************/
void buildRainbowTable()
/*****************************************************************************/
{
  for (int i = 0; i < RAINBOW_SIZE; i++) {
    rainbowTable[i] = tft.color565(
                        (sin((i + 0) * 3.14159 / 180) * 127 + 128),
                        (sin((i + 120) * 3.14159 / 180) * 127 + 128),
                        (sin((i + 240) * 3.14159 / 180) * 127 + 128)
                      );
  }
}
/*****************************************************************************/
// Return rainbow color for a given pixel
uint16_t rainbowLineColor(int x, int offset)
/*****************************************************************************/
{
  // Spread rainbow across width by scaling x
  int hue = ((x * 360) / tft.width() + offset) % 360;
  return tft.color565(
           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
         );
  //  return (tft.color565(offset, offset + x, x));
}
/*****************************************************************************/
// Return contrast color (180° phase shift)
uint16_t contrastColor(int index, int offset)
/*****************************************************************************/
{
  int hue = (index * 40 + offset + 180) % 360; // shifted by 180°
  return tft.color565(
           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
         );
}
/*****************************************************************************/
// Draw a single digit at (x, y) with rainbow shimmer for active segments,
// and black for inactive segments
void drawDigit(int digit, int x, int y, int offset)
/*****************************************************************************/
{ // Copied from Test_8
  static int seg;

  //  Serial.printf("drawDigit called with digit %d\r\n", digit);
  // Mark active segments for this digit
  bool active[7] = {false};
  for (int i = 0; i < digitCount[digit]; i++) {
    int seg = digitSegments[digit][i];

    // Guard against bad values
    if (seg >= 0 && seg < 7) {
      active[seg] = true;
    } else {
      Serial.printf("⚠️ Bad seg index: %d for digit %d\r\n", seg, digit);
    }
  }

  // Mark active segments for this digit
  for (int i = 0; i < digitCount[digit]; i++) {
    //    Serial.printf("drawDigit doing %d\r\n", seg); Serial.flush();
    seg = digitSegments[digit][i];
    active[seg] = true;
  }

  // Draw all 7 segments
  for (int seg = 0; seg < 7; seg++) {
    if (active[seg]) {
      // active → rainbow
      tft.setTextColor(rainbowColor(seg, offset), TFT_BLACK);
      tft.drawString(String(seg), x, y);  // digit segment (0-6)
    } else {
      // inactive → blank
      tft.setTextColor(TFT_BLACK, TFT_BLACK);
      tft.drawString(String(seg), x, y);  // digit segment (0-6)
    }
  }
}
/*****************************************************************************/
void drawMyString(const char* str, int digitX, int digitY, int baseOffset)
/*****************************************************************************/
{ // From Test_8, renamed to work here.
  static int digit;
  static int tempHour;
  static int localOffset;

  // Convert 24 hour time to 12 hour time.
  tempHour = tm->tm_hour;
  if (tempHour == 0) tempHour = 12;  // Midnight
  if (tempHour > 12) tempHour -= 12;  // 1:00 pm through 11:59 pm

  for (int position = 0; str[position] != '\0'; position++) {
    localOffset = (baseOffset + position * 30) % 360;

    if (str[position] == ':') {
      // Flash on/off approx 1/2 second
      // colonOn = (millis() / 500) % 2;
      colonOn = tm->tm_sec % 2;  // Flash on/off approx 1/2 second
      if (colonOn) {
        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      } else {
        tft.setTextColor(TFT_BLACK);
      }
      //      tft.setCursor(digitX, digitY);
      if (tempHour > 9)
        tft.drawString(":", digitX - 11, digitY);  // Adjust the ":" spacing
      else
        tft.drawString(":", digitX - 3, digitY);  // Adjust the ":" spacing
      if (tempHour > 9)
        digitX += 10;
      else
        digitX += 30;

    } else {  // It's a digit.

      digit = str[position] - '0';
      if (digit < 0 || digit > 9) {
        Serial.printf("⚠️ Skipping non-digit char '%c' at position %d\n",
                      str[position], position);
        continue;
      }
      if (position == 0 && digit == 0) continue; // suppress leading zero
      drawDigit(digit, digitX, digitY, localOffset);
      if (tempHour > 9)
        digitX += 120;
      else
        digitX += 120;
    }
  }
}
/*****************************************************************************/
/*
   Call it thus:
   safeLoadOFR_Font("EldoradoRomanTTF", EldoradoRomanTTF,
                sizeof(EldoradoRomanTTF), __LINE__, __func__);
*/
bool safeLoadOFR_Font(const char* fontName, const uint8_t* fontData,
                      size_t size, int line, const char* funcName)
/*****************************************************************************/
{
  static const char* lastFontName = nullptr;  // remember last loaded font

  // If the same font is requested again, skip reload
  if (lastFontName && strcmp(lastFontName, fontName) == 0) {
    // Optional: log that we skipped
    //    time(&UTC); workTime_t = UTC + iHomeOffset;
    //    tm = localtime(&workTime_t);
    // Serial.printf("%02i:%02i:%02i Font '%s' already loaded, "
    //               "skipping reload (called from %s: %d)\n",
    //               tm->tm_hour, tm->tm_min, tm->tm_sec,
    //               fontName, funcName, line);
    return true;
  }

  int rc = 0;  try {
    ofr.unloadFont();
  } catch (...) {
    Serial.println("unloadFont failed, continuing anyway...");
  }

  rc = ofr.loadFont(fontData, size);
  if (rc) {
    printFontError(fontName, rc, funcName, line);
    return false;
  }

  // Update the last loaded font name
  lastFontName = fontName;
  return true;
}
/*****************************************************************************/
void printFontError(const char* fontName, int rc, const char* func, int line)
/*****************************************************************************/
{
  Serial.printf("Render loadFont error for %s\n", fontName);
  Serial.printf("In routine %s, near line %d\n", func, line - 2);
  Serial.printf("Error code %i\r\n", rc);
  // Optional: map known error codes to text
  switch (rc) {
    case 1: Serial.println("Error: invalid font data"); break;
    case 2: Serial.println("Error: allocation failed"); break;
    case 3: Serial.println("Error: font already loaded"); break;
    // Add more codes as you discover them
    default: Serial.println("Error: unknown code"); break;
  }
}

/*****************************************************************************/
void startWiFiManager()
/***startWiFiManager**********************************************************/
{

  //  wifiManager.resetSettings(); // wipe settings -- emergency use only.

  wifiManager.setTimeout(60);  // 30 second connect timeout then reboot.
  wifiManager.disconnect();
  if (!wifiManager.autoConnect(myPortalName, "BigLittleClock")) {
    Serial.println(F("failed to connect and hit timeout"));
    //Reboot so we can try again.
#ifdef ESP32
    ESP.restart();
#else
    ESP.reset();
#endif
  }
  // Set callback that gets called when connecting to previous WiFi fails,
  //  and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //Fetches ssid and pass and tries to connect
  //If it does not connect it starts an access point with the specified name
  //here  "ESPWiFiMgr"
  //and goes into a blocking loop awaiting configuration
  wifiManager.setDebugOutput(false);  // Quieter on the Serial Monitor
  if (!wifiManager.autoConnect(myPortalName)) {
    Serial.println(F("Failed to connect and hit timeout."));
    //Reset and try again, or maybe put it to deep sleep
#ifdef ESP32
    ESP.restart();
#else
    ESP.reset();
#endif
  }
  // If you get here you have connected to the WiFi
  //  Serial.println(F("Now connected."));
}
/*****************************************************************************/
void configModeCallback (WiFiManager * myWiFiManager)
/*****************************************************************************/
{
  Serial.println(F("Entered config mode..."));
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  int yPos = 10;
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);  // FG color, BG color
  tft.drawString("Need SSID", tft.width() / 2, yPos);
  tft.drawString("Connect to", tft.width() / 2, yPos + 43);
  tft.drawString(myPortalName, tft.width() / 2, yPos + 85);
}
/*****************************************************************************/
void timeSyncCallback(struct timeval * tv)
/*****************************************************************************/
{
  //  struct timeval {
  //   time_t      tv_sec;   // Number of whole seconds of elapsed time
  //   long int    tv_usec;  // Number of microseconds of rest of
  //                             elapsed time minus tv_sec.
  //                            Always less than one million
  //};
  Serial.println("\n----Time Sync Obtained-----");
  Serial.printf("Time sync at %u ms.\r\nUTC Epoch: ", millis());
  Serial.println(tv->tv_sec);
  Serial.println(ctime(&tv->tv_sec));
  time(&workTime_t);
}
/*****************************************************************************/
String th(int theDate)
/*****************************************************************************/
{
  switch (theDate) {  // Make it English-like a little bit
    case 1: case 21: case 31: return "st"; break;
    case 2: case 22:          return "nd"; break;
    case 3: case 23:          return "rd"; break;
    default:                  return "th"; break;
  }
}
/*****************************************************************************/
void drawRainbowLine(int y, int offset)
/*****************************************************************************/
{
  static int hueIndex;

  for (int x = 0; x < tft.width(); x++) {
    hueIndex = ((x * 360) / tft.width() + offset) % 360;
    rainbow1LineSprite.drawPixel(x, 0, rainbowTable[hueIndex]);
  }
  rainbow1LineSprite.pushSprite(0, y);  // blast the sprite to tft
}
/*****************************************************************************/
// Helper function to measure text and return sprite metrics
TextMetrics getTextMetrics(const char *text, int fontSize,
                           int descenderPad)
/*****************************************************************************/
{
  TextMetrics m;
  m.width  = ofr.getTextWidth(text, fontSize);
  m.height = ofr.getTextHeight(text, fontSize) + descenderPad;

  // Use "A" as a proxy for ascent to get baseline offset
  m.baselineOffset = m.height - ofr.getTextHeight("A", fontSize);

  return m;
}
/*****************************************************************************/
void HourDance(int which)
/*****************************************************************************/
{
  static int whichDance = 0;
  static int origDatum = tft.getTextDatum();

  switch (which)
  {
    case 0:
      {
        Serial.println("HourDance: 4 Inversions");
        tft.invertDisplay(true);  delay(200);
        tft.invertDisplay(false); delay(200);
        tft.invertDisplay(true);  delay(200);
        tft.invertDisplay(false); delay(200);
        tft.invertDisplay(true);  delay(200);
        tft.invertDisplay(false); delay(200);
        tft.invertDisplay(true);  delay(200);
        tft.invertDisplay(false);
        break;
      }

    case 1:
      {
        Serial.println("HourDance: Collapsing and reviving rectangles");
        static int rainbowOffset = 0;
        int w = tft.width();
        int h = tft.height();
        int step = 2;

        doThis(3) {
          // Collapse inward (plain white)
          for (int i = 0; i < min(w, h) / 2; i += step) {
            int x = i;
            int y = i;
            int rectW = w - 2 * i;
            int rectH = h - 2 * i;

            tft.drawRect(x, y, rectW, rectH, TFT_WHITE);
            delay(3);
          }

          // Expand outward (rainbow colors)
          for (int i = min(w, h) / 2; i >= 0; i -= step) {
            int x = i;
            int y = i;
            int rectW = w - 2 * i;
            int rectH = h - 2 * i;

            // Use rainbowColor with index=i and offset=rainbowOffset
            uint16_t color = rainbowColor(i, rainbowOffset);
            tft.drawRect(x, y, rectW, rectH, color);
            delay(3);
          }
          // Advance offset so next hour shifts rainbow
          rainbowOffset = (rainbowOffset + random(15)) % 360;
        }
        break;
      }

    case 2:
      {
        Serial.println("HourDance: Concentric Circles (Ripple Effect)");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;
        int maxR = max(w, h) / 2;
        static int rainbowOffset = 0;

        doThis(3) {
          // Ripple outward
          for (int r = 0; r < maxR; r += 4) {
            uint16_t color = rainbowColor(r, rainbowOffset);
            tft.drawCircle(cx, cy, r, color);
            delay(10);
          }
          rainbowOffset = (rainbowOffset + random(20)) % 360;
          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 3:
      //      {
      //        Serial.println("HourDance: Diagonal Sweep");
      //        int w = tft.width();
      //        int h = tft.height();
      //        static int rainbowOffset = 0;
      //
      //        doThis(2) {
      //          // Sweep diagonal lines across the screen
      //          for (int i = 0; i < w + h; i += 6) {
      //            uint16_t color = rainbowColor(i, rainbowOffset);
      //            tft.drawLine(0, i, i, 0, color);   // top-left sweep
      //            tft.drawLine(w, h - i, w - i, h, color); // bottom-right sweep
      //            delay(10);
      //          }
      //          rainbowOffset = (rainbowOffset + random(25)) % 360;
      //          tft.fillScreen(TFT_BLACK);
      //        }
      //        break;
      //      }
      {
        Serial.println("HourDance: Diagonal Sweep");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          // Sweep diagonal lines across the screen
          for (int i = 0; i < w + h; i += 6) {
            uint16_t color = rainbowColor(i, rainbowOffset);

            // Original sweeps
            tft.drawLine(0, i, i, 0, color);            // top-left sweep
            tft.drawLine(w, h - i, w - i, h, color);    // bottom-right sweep

            // Added opposite sweeps
            tft.drawLine(w, i, w - i, 0, color);        // top-right sweep
            tft.drawLine(0, h - i, i, h, color);        // bottom-left sweep

            delay(10);
          }
          rainbowOffset = (rainbowOffset + random(25)) % 360;
          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 4:
      {
        Serial.println("HourDance: Expanding Filled Rectangles (Pulse)");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          for (int i = min(w, h) / 2; i >= 0; i -= 4) {
            uint16_t color = rainbowColor(i, rainbowOffset);
            tft.fillRect(i, i, w - 2 * i, h - 2 * i, color);
            delay(15);
          }
          rainbowOffset = (rainbowOffset + random(30)) % 360;
          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 5:
      {
        Serial.println("HourDance: Spiral Sweep");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;
        static int rainbowOffset = 0;

        doThis(2) {
          int prevX = cx;
          int prevY = cy;
          for (int angle = 0; angle < 3600; angle += 2) {   // finer steps
            int r = angle / 15;                    // Radius growth
            int x = cx + cos(angle * 3.14159 / 180) * r;
            int y = cy + sin(angle * 3.14159 / 180) * r;
            uint16_t color = rainbowColor(angle, rainbowOffset);
            tft.drawLine(prevX, prevY, x, y, color);       // connect points
            prevX = x;
            prevY = y;
            delay(1);
          }
          rainbowOffset = (rainbowOffset + random(20)) % 360;
          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 6:
      {
        Serial.println("HourDance: Vertical Bars Pulse");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(3) {
          for (int x = 0; x < w; x += 6) {
            uint16_t color = rainbowColor(x, rainbowOffset);
            tft.fillRect(x, 0, 6, h, color);
            delay(10);
          }
          rainbowOffset = (rainbowOffset + random(25)) % 360;
          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 7:
      {
        Serial.println("HourDance: Starburst Lines");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;
        static int rainbowOffset = 0;

        doThis(2) {
          for (int angle = 0; angle < 360; angle += 10) {
            int x = cx + cos(angle * 3.14159 / 180) * cx;
            int y = cy + sin(angle * 3.14159 / 180) * cy;
            uint16_t color = rainbowColor(angle, rainbowOffset);
            tft.drawLine(cx, cy, x, y, color);
            delay(5);
          }
          rainbowOffset = (rainbowOffset + random(30)) % 360;
          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 8:
      {
        Serial.println("HourDance: Checkerboard Flash");
        int w = tft.width();
        int h = tft.height();
        int size = 20;
        static int rainbowOffset = 0;

        doThis(3) {
          for (int y = 0; y < h; y += size) {
            for (int x = 0; x < w; x += size) {
              uint16_t color = rainbowColor(x + y, rainbowOffset);
              if (((x / size) + (y / size)) % 2 == 0)
                tft.fillRect(x, y, size, size, color);
            }
          }
          delay(200);
          tft.fillScreen(TFT_BLACK);
          rainbowOffset = (rainbowOffset + random(40)) % 360;
        }
        break;
      }

    case 9:
      {
        Serial.println("HourDance: The Big Bong");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;

        safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                         sizeof(BritanicBoldTTF), __LINE__, __func__);
        tft.setTextDatum(MC_DATUM);   // center text

        // Draw "BONG" in bright red, then fade to black
        doThis(3) {
          for (int fade = 255; fade >= 0; fade -= 15) {
            uint16_t color = tft.color565(fade, 0, 0); // red channel only
            tft.fillScreen(TFT_BLACK);                 // clear each frame
            tft.setTextColor(color);
            tft.drawString("BONG", cx, cy);
            delay(50); // ~15 steps → about 750ms total
          }
        }
        break;
      }

    case 10:
      {
        Serial.println("HourDance: Fill with Circles");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;

        for (int r = 0; r < min(w, h); r += 8) {
          uint16_t color = rainbowColor(r, 0);
          tft.fillCircle(cx, cy, r, color);
          delay(20);
        }
        break;
      }

    case 11:
      {
        Serial.println("HourDance: Fill with Boxes");
        int w = tft.width();
        int h = tft.height();

        for (int i = 0; i < min(w, h) / 2; i += 8) {
          uint16_t color = rainbowColor(i, 0);
          tft.fillRect(i, i, w - 2 * i, h - 2 * i, color);
          delay(20);
        }
        break;
      }

    case 12:
      {
        Serial.println("HourDance: Double Spiral");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;
        static int rainbowOffset = 0;

        doThis(2) {
          int prevX1 = cx, prevY1 = cy;
          int prevX2 = cx, prevY2 = cy;

          for (int angle = 0; angle < 3600; angle += 3) {
            int r = angle / 20; // radius grows faster

            // Spiral arm 1
            int x1 = cx + cos(angle * 3.14159 / 180) * r;
            int y1 = cy + sin(angle * 3.14159 / 180) * r;
            uint16_t color1 = rainbowColor(angle, rainbowOffset);
            tft.drawLine(prevX1, prevY1, x1, y1, color1);
            prevX1 = x1; prevY1 = y1;

            // Spiral arm 2 (180° offset)
            int x2 = cx + cos((angle + 180) * 3.14159 / 180) * r;
            int y2 = cy + sin((angle + 180) * 3.14159 / 180) * r;
            uint16_t color2 = rainbowColor(angle + 180, rainbowOffset);
            tft.drawLine(prevX2, prevY2, x2, y2, color2);
            prevX2 = x2; prevY2 = y2;

            delay(1);
          }

          rainbowOffset = (rainbowOffset + random(20)) % 360;
          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 13:
      {
        Serial.println("HourDance: Sparkle Field");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          for (int i = 0; i < 2000; i++) {   // number of sparkles per burst
            int x = random(w);
            int y = random(h);
            uint16_t color = rainbowColor(x + y, rainbowOffset);
            tft.drawPixel(x, y, color);
            delay(2);
          }
          delay(300);                       // let them twinkle briefly
          tft.fillScreen(TFT_BLACK);        // clear for next burst
          rainbowOffset = (rainbowOffset + random(40)) % 360;
        }
        break;
      }

    case 14: {
        Serial.println("HourDance: Rainbow Wipe");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          // Sweep left to right
          for (int x = 0; x < w; x += 6) {
            uint16_t color = rainbowColor(x, rainbowOffset);
            tft.fillRect(x, 0, 6, h, color);
            delay(5);
          }
          delay(300);  // pause to admire the full rainbow
          tft.fillScreen(TFT_BLACK);  // clear for next run
          rainbowOffset = (rainbowOffset + random(30)) % 360;
        }
        break;
      }

    case 15: {
        Serial.println("HourDance: Gradient Fade");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          // Sweep hue across the screen gradually
          for (int shift = 0; shift < 360; shift += 15) {
            for (int y = 0; y < h; y++) {
              // Each row gets a slightly different hue
              uint16_t color = rainbowColor(shift + y, rainbowOffset);
              tft.drawFastHLine(0, y, w, color);
            }
            delay(20);  // smooth fade speed
          }
          rainbowOffset = (rainbowOffset + random(50)) % 360;
          tft.fillScreen(TFT_BLACK);
        }
        break;
      }
    default: HourDance(0);  // Just in case...  Just in case!
  }
  tft.setTextDatum(origDatum);   // Whatever it was!
  tft.fillScreen(TFT_BLACK);
}

/*
  4 Inversions
  OK
  Collapsing and reviving rectangles
  OK
  Concentric Circles (Ripple Effect)
  Not big enough. Stops at small dimension
  Diagonal Sweep
  Should go from other angle, angled the other way
  maybe without wiping the previous until both are done
  Expanding Filled Rectangles (Pulse)
  OK but might go longer like the filled circles
  Spiral Sweep
  Add a second one at 180º
  Vertical Bars Pulse
  Kinda ugly.  Colors should be wider. Add horizontal strips, too.
  Starburst Lines
  Looks good but, is over too quickly.  Maybe draw multiple of each color
  Checkerboard Flash
  Colors change far too slowly
  The Big Bong
  No text just boxes
  Fill with Circles
  OK but lasts too long
  Fill with Boxes
  OK. Just short running
  Double Spiral
  Stops far too soon
  Sparkle Field
  Needs more spots
  Rainbow Wipe
  Missed it
  Gradient Fade
  Runs far too long and does not fade
*/
