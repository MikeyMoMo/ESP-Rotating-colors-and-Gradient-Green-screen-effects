/*****************************************************************************/
bool inBox(int touchX, int touchY, int boxX, int boxY, int boxW, int boxH)
/*****************************************************************************/
{
  // touch X and Y are the point to be tested to be in the box.
  // box X and Y are the left and top of the area to be tested to be in.
  // box W and H are the width and height of the area to be tested to be in.
  return (touchX >= boxX && touchX <= (boxX + boxW) &&
          touchY >= boxY && touchY <= (boxY + boxH));
}
/*****************************************************************************/
void punchRainbowShift(TFT_eSprite & spr, uint16_t *rainbowLine,
                       int lineWidth, int shiftOffset)
/*****************************************************************************/
{
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
void punchImage(TFT_eSprite &sprText, TFT_eSprite &sprImage)
/*****************************************************************************/
{
  for (int y = 0; y < sprText.height(); y++) {
    for (int x = 0; x < sprText.width(); x++) {
      uint16_t c = sprText.readPixel(x, y);
      if (c != TFT_BLACK) {
        // Map pixel from the picture sprite
        uint16_t imgColor = sprImage.readPixel(x % sprImage.width(),
                                               y % sprImage.height());
        sprText.drawPixel(x, y, imgColor);
      }
    }
  }
}
/*****************************************************************************/
void punchImageMoving(TFT_eSprite &sprText, TFT_eSprite &sprImage,
                      int xOffset, int yOffset)
/*****************************************************************************/
{
  for (int y = 0; y < sprText.height(); y++) {
    for (int x = 0; x < sprText.width(); x++) {
      uint16_t c = sprText.readPixel(x, y);
      if (c != TFT_BLACK) {
        // Apply offsets to create rotation/scroll effect
        int imgX = (x + xOffset) % sprImage.width();
        int imgY = (y + yOffset) % sprImage.height();
        uint16_t imgColor = sprImage.readPixel(imgX, imgY);
        sprText.drawPixel(x, y, imgColor);
      }
    }
  }
}
/*****************************************************************************/
// This version blows up the middle to look more like rotation.
void punchImageRotated(TFT_eSprite &sprText, TFT_eSprite &sprImage,
                       float angle)
/*****************************************************************************/
{
  // Precompute sine/cosine for efficiency
  float cosA = cos(angle);
  float sinA = sin(angle);

  // Center of the text sprite
  int cx = sprText.width() / 2;
  int cy = sprText.height() / 2;

  // Center of the image sprite
  int icx = sprImage.width() / 2;
  int icy = sprImage.height() / 2;

  for (int y = 0; y < sprText.height(); y++) {
    for (int x = 0; x < sprText.width(); x++) {
      uint16_t c = sprText.readPixel(x, y);
      if (c != TFT_BLACK) {
        // Translate coordinates to text center
        int dx = x - cx;
        int dy = y - cy;

        // Apply rotation transform
        int rx = (int)(dx * cosA - dy * sinA);
        int ry = (int)(dx * sinA + dy * cosA);

        // Map back to image coordinates
        int imgX = (rx + icx) % sprImage.width();
        int imgY = (ry + icy) % sprImage.height();

        if (imgX >= 0 && imgY >= 0) {
          uint16_t imgColor = sprImage.readPixel(imgX, imgY);
          sprText.drawPixel(x, y, imgColor);
        }
      }
    }
  }
}
/*****************************************************************************/
// Return rainbow color at given index+offset
uint16_t rainbowColorMode1(int index, int offset)  // Original rainbowColor
/*****************************************************************************/
{
  int hue = (index * 40 + offset) % 360;
  //  return rainbowTable[hue];
  return tft.color565(
           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
         );
}
/*****************************************************************************/
// Calling this version paints 1 color per digit.
// Assign colors by (x,y) position instead of segment number
uint16_t rainbowColorMode4(int x, int y, int offset)  // washColor from CoPilot
/*****************************************************************************/
{
  int hue = (x * 40 + y * 20 + offset) % 360;
  return rainbowTable[hue];
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
  return rainbowTable[hue];
  //  return tft.color565(
  //           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
  //           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
  //           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
  //         );
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
        Serial.println("0-HourDance: 4 Inversions");
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
        Serial.println("1-HourDance: Collapsing and reviving rectangles");
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
            uint16_t color = rainbowColorMode1(i, rainbowOffset);
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
        Serial.println("2-HourDance: Concentric Circles (Ripple Effect)");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;
        int maxR = max(w, h) / 2;
        static int rainbowOffset = 0;

        doThis(3) {
          // Ripple outward
          for (int r = 0; r < maxR; r += 4) {
            uint16_t color = rainbowColorMode1(r, rainbowOffset);
            tft.drawCircle(cx, cy, r, color);
            delay(10);
          }
          rainbowOffset = (rainbowOffset + random(20)) % 360;
          //          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 3:
      //      {
      //        Serial.println("3-HourDance: Diagonal Sweep");
      //        int w = tft.width();
      //        int h = tft.height();
      //        static int rainbowOffset = 0;
      //
      //        doThis(2) {
      //          // Sweep diagonal lines across the screen
      //          for (int i = 0; i < w + h; i += 6) {
      //            uint16_t color = rainbowColorMode1, rainbowOffset);
      //            tft.drawLine(0, i, i, 0, color);   // top-left sweep
      //            bottom-right sweep
      //            tft.drawLine(w, h - i, w - i, h, color);
      //            delay(10);
      //          }
      //          rainbowOffset = (rainbowOffset + random(25)) % 360;
      //          tft.fillScreen(TFT_BLACK);
      //        }
      //        break;
      //      }
      {
        Serial.println("3-HourDance: Diagonal Sweep");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          // Sweep diagonal lines across the screen
          for (int i = 0; i < w + h; i += 6) {
            uint16_t color = rainbowColorMode1(i, rainbowOffset);

            // Original sweeps
            tft.drawLine(0, i, i, 0, color);            // top-left sweep
            tft.drawLine(w, h - i, w - i, h, color);    // bottom-right sweep

            // Added opposite sweeps
            tft.drawLine(w, i, w - i, 0, color);        // top-right sweep
            tft.drawLine(0, h - i, i, h, color);        // bottom-left sweep

            delay(10);
          }
          rainbowOffset = (rainbowOffset + random(25)) % 360;
          //          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 4:
      {
        Serial.println("4-HourDance: Expanding Filled Rectangles (Pulse)");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          for (int i = min(w, h) / 2; i >= 0; i -= 4) {
            uint16_t color = rainbowColorMode1(i, rainbowOffset);
            tft.fillRect(i, i, w - 2 * i, h - 2 * i, color);
            delay(15);
          }
          rainbowOffset = (rainbowOffset + random(30)) % 360;
          //          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 5:
      {
        Serial.println("5-HourDance: Spiral Sweep");
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
            uint16_t color = rainbowColorMode1(angle, rainbowOffset);
            tft.drawLine(prevX, prevY, x, y, color);       // connect points
            prevX = x;
            prevY = y;
            delay(1);
          }
          rainbowOffset = (rainbowOffset + random(20)) % 360;
          //          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 6:
      {
        Serial.println("6-HourDance: Vertical Bars Pulse");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(3) {
          for (int x = 0; x < w; x += 6) {
            uint16_t color = rainbowColorMode1(x, rainbowOffset);
            tft.fillRect(x, 0, 6, h, color);
            delay(10);
          }
          rainbowOffset = (rainbowOffset + random(25)) % 360;
          //          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 7:
      {
        Serial.println("7-HourDance: Starburst Lines");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;
        static int rainbowOffset = 0;

        doThis(2) {
          for (int angle = 0; angle < 360; angle += 10) {
            int x = cx + cos(angle * 3.14159 / 180) * cx;
            int y = cy + sin(angle * 3.14159 / 180) * cy;
            uint16_t color = rainbowColorMode1(angle, rainbowOffset);
            tft.drawLine(cx, cy, x, y, color);
            delay(5);
          }
          rainbowOffset = (rainbowOffset + random(30)) % 360;
          //          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 8:
      {
        Serial.println("8-HourDance: Checkerboard Flash");
        int w = tft.width();
        int h = tft.height();
        int size = 20;
        static int rainbowOffset = 0;

        doThis(3) {
          for (int y = 0; y < h; y += size) {
            for (int x = 0; x < w; x += size) {
              uint16_t color = rainbowColorMode1(x + y, rainbowOffset);
              if (((x / size) + (y / size)) % 2 == 0)
                tft.fillRect(x, y, size, size, color);
            }
          }
          delay(200);
          //          tft.fillScreen(TFT_BLACK);
          rainbowOffset = (rainbowOffset + random(40)) % 360;
        }
        break;
      }

    case 9: {
        static int fade;
        static uint16_t color;
        Serial.println("9-HourDance: The Big Bong");
        // int w = tft.width();
        // int h = tft.height();
        // int cx = w / 2;
        // int cy = h / 2;

        safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                         sizeof(BritanicBoldTTF), __LINE__, __func__);
        ofr.setFontSize(200);
        ofr.setDrawer(tft);

        // Measure string so we can center it
        // int textW = ofr.getTextWidth("BONG", 100);
        // int textH = ofr.getTextHeight("BONG", 100);
        // int x = cx - textW / 2;
        // int y = cy - textH / 2;

        doThis(3) {
          // Fade loop
          for (fade = 255; fade >= 0; fade -= 20) {
            color = tft.color565(fade, fade, fade);
            ofr.setFontColor(color);
            ofr.setCursor(tft.width() / 2, 0);
            ofr.cprintf("BONG");
            delay(50);
          }
        }
        ofr.setDrawer(spriteDate); // Link renderer sprite for date display.
        break;
      }

    case 10:
      {
        Serial.println("10-HourDance: Fill with Circles");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;

        for (int r = 0; r < min(w, h); r += 8) {
          uint16_t color = rainbowColorMode1(r, 0);
          tft.fillCircle(cx, cy, r, color);
          delay(20);
        }
        break;
      }

    case 11:
      {
        Serial.println("11-HourDance: Fill with Boxes");
        int w = tft.width();
        int h = tft.height();

        for (int i = 0; i < min(w, h) / 2; i += 8) {
          uint16_t color = rainbowColorMode1(i, 0);
          tft.fillRect(i, i, w - 2 * i, h - 2 * i, color);
          delay(20);
        }
        break;
      }

    case 12:
      {
        Serial.println("12-HourDance: Double Spiral");
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
            uint16_t color1 = rainbowColorMode1(angle, rainbowOffset);
            tft.drawLine(prevX1, prevY1, x1, y1, color1);
            prevX1 = x1; prevY1 = y1;

            // Spiral arm 2 (180° offset)
            int x2 = cx + cos((angle + 180) * 3.14159 / 180) * r;
            int y2 = cy + sin((angle + 180) * 3.14159 / 180) * r;
            uint16_t color2 = rainbowColorMode1(angle + 180, rainbowOffset);
            tft.drawLine(prevX2, prevY2, x2, y2, color2);
            prevX2 = x2; prevY2 = y2;

            delay(1);
          }

          rainbowOffset = (rainbowOffset + random(20)) % 360;
          //          tft.fillScreen(TFT_BLACK);
        }
        break;
      }

    case 13:
      {
        Serial.println("13-HourDance: Sparkle Field");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          for (int i = 0; i < 2000; i++) {   // number of sparkles per burst
            int x = random(w);
            int y = random(h);
            uint16_t color = rainbowColorMode1(x + y, rainbowOffset);
            tft.drawPixel(x, y, color);
            delay(2);
          }
          delay(300);                       // let them twinkle briefly
          //          tft.fillScreen(TFT_BLACK);        // clear for next burst
          rainbowOffset = (rainbowOffset + random(40)) % 360;
        }
        break;
      }

    case 14: {
        Serial.println("14-HourDance: Rainbow Wipe");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          // Sweep left to right
          for (int x = 0; x < w; x += 6) {
            uint16_t color = rainbowColorMode1(x, rainbowOffset);
            tft.fillRect(x, 0, 6, h, color);
            delay(5);
          }
          delay(300);  // pause to admire the full rainbow
          //          tft.fillScreen(TFT_BLACK);  // clear for next run
          rainbowOffset = (rainbowOffset + random(30)) % 360;
        }
        break;
      }

    case 15: {
        Serial.println("15-HourDance: Gradient Fade");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        doThis(2) {
          // Sweep hue across the screen gradually
          for (int shift = 0; shift < 360; shift += 15) {
            for (int y = 0; y < h; y++) {
              // Each row gets a slightly different hue
              uint16_t color = rainbowColorMode1(shift + y, rainbowOffset);
              tft.drawFastHLine(0, y, w, color);
            }
            delay(20);  // smooth fade speed
          }
          rainbowOffset = (rainbowOffset + random(50)) % 360;
          //          tft.fillScreen(TFT_BLACK);
        }
        break;
      }
    default: HourDance(0);  // Just in case...  Just in case!
  }
  tft.setTextDatum(origDatum);   // Whatever it was!
  tft.fillScreen(TFT_BLACK);
}
/***************************************************************************/
void printVers()
/***************************************************************************/
{
  int lastDot, lastV;

  //  Serial.println(__FILENAME__);  // Same as __FILE__
  sTemp = String(__FILE__);
  // Get rid of the trailing .ino tab name. In this case, "\Utilities.ino"
  sTemp = sTemp.substring(0, sTemp.lastIndexOf("\\"));
  Serial.print("Running from: ");
  Serial.println(sTemp);

  sTemp = String(__FILE__);  // Start again for the version number.
  lastDot = sTemp.lastIndexOf(".");
  if (lastDot > -1) {                     // Found a dot.
    lastV = sTemp.lastIndexOf("v");       // Find start of version number
    if (lastV > -1) {                     // Found version number
      sVer = sTemp.substring(lastV + 1, lastDot);  // Get version number
      lastV = sVer.lastIndexOf("\\");
      if (lastV > -1) sVer = sVer.substring(0, lastV);
    } else {
      sVer = "0.00";  // Unknown version.
    }
  } else {
    sVer = "n/a";  // Something badly wrong here!
  }
  Serial.print("Version " + sVer + " ");
  Serial.println("Compiled on: " + String(__DATE__) +
                 " at " + String(__TIME__));
}
/*****************************************************************************/
void handleUserReq()
/*****************************************************************************/
{
  static char cUserReq, prevCmd;
  static char inStr[chrBufSize];

  if (!Serial.available()) return;

  cUserReq = toupper(Serial.read());  // Read command character
  if (cUserReq == '\n') return;  // was 0x0a
  Serial.print("Command char read: "); Serial.println(cUserReq);

  // Clear buffer and reset pointer.
  memset(inStr, 0, chrBufSize); inStrCt = 0;

  while (Serial.available()) {
    inStr[inStrCt++] = Serial.read(); delay(20);
    if (inStrCt == chrBufSize) {
      Serial.println("Serial Input line too long. Ignored.");
      return;
    }
  }

  // if (inStrCt > 0) {
  //   Serial.printf("%i characters read from input steam\r\n", inStrCt);
  //   Serial.print("Input: /"); Serial.print(inStr); Serial.println("/");
  // }

  time(&workTime_t);
  tm = localtime(&workTime_t);
  switch (cUserReq)
  {
    case 'R':
      ESP.restart();
      break;
    case 'H': case '?':
      // Show Help screen on H or ?
      Serial.println("Help text to be added later.  Try typing I");
      //      for (int temp = 0; temp < helpLines; temp++) {
      //        Serial.println(helpString[temp]);
      //      }
      break;

    // Show selected operational information.
    case 'I':
      Serial.println(); Serial.print(ctime(&workTime_t));
      Serial.printf("Gradient Clock vers %s\r\n", sVer);
      Serial.printf("Display mode is %d\r\n", displayMode);
      Serial.println("Help text to be added later.");
      break;

    // All field shimmer.
    case '1':
      Serial.println("\r\n1 - Display mode set to 1 - Segment Shimmering");
      displayMode = DM_FULL_SHIMMER;
      break;
    // Only the rainbow line moves.
    case '2':
      Serial.println("\r\n2 - Display mode set to 2 - Limited Shimmering");
      displayMode = DM_RL_SHIMMER;
      break;
    // Only the rainbow line moves.
    case '4':
      Serial.println("\r\n4 - Display mode set to 4 - Wholedigit Shimmer");
      displayMode = DM_WHOLE_DIGIT;
      break;
    case '3':
      Serial.println("\r\n3 - Display mode set to 3 - Single Color per digit");
      Serial.println("Display mode 3 not implemented, defaulting to mode 1.");
      displayMode = DM_STATIC_DIGITS;
      solidColor = TFT_GREEN; // default bright green

      if (inStrCt > 0) {
        char arg = toupper(inStr[0]);  // first extra char
        switch (arg) {
          case 'R':
            solidColor = TFT_RED;
            Serial.println("Mode 3 color set TFT_RED");
            break;
          case 'G':
            solidColor = TFT_GREEN;
            Serial.println("Mode 3 color set to TFT_GREEN");
            break;
          case 'B':
            solidColor = TFT_BLUE;
            Serial.println("Mode 3 color set to TFT_BLUE");
            break;
          case 'C':
            solidColor = TFT_CYAN;
            Serial.println("Mode 3 color set to TFT_CYAN");
            break;
          case 'W':
            solidColor = TFT_WHITE;
            Serial.println("Mode 3 color set to TFT_WHITE");
          default:  // fallback
            solidColor = TFT_GREEN;  // Maybe be redundant.
            Serial.println("Mode 3 color set to TFT_GREEN");
        }
      } else {
        Serial.println("Mode 3 default color set to Bright Green");
      }
      break;

    default:
      Serial.println("\r\nCommand/Mode not implemented.  Enter:");
      Serial.println("1  - All Shimmer\r\n"
                     "2  - Only Rainbow line moves.\r\n"
                     "3x - Solid color per digit, where x is:\r\n"
                     "    R - Red\r\n"
                     "    G - Green\r\n"
                     "    B - Blue\r\n"
                     "    C - Cyan\r\n"
                     "    W - White");


      //Missing "End Switch" causing double outdent.
      break;
  }
  prevCmd = cUserReq;  // Remember the previous command in case of X
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
