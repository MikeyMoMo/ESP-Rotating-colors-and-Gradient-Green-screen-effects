/*============================================================================
  Utilities.ino  -  helper routines
  ----------------------------------------------------------------------------
  Sections, in order:
    A. Touch helper ......................... inBox()               (unused)
    B. "Punch-through" effects .............. punchRainbowShift(),
                                              punchImage(), punchImageMoving(),
                                              punchImageRotated()
    C. Colour helpers ....................... rainbowColorMode1/4(),
                                              buildRainbowTable(),
                                              rainbowLineColor(), contrastColor()
    D. Font loading ......................... safeLoadOFR_Font(),
                                              printFontError()
    E. WiFi and time ........................ startWiFiManager(),
                                              configModeCallback(),
                                              timeSyncCallback()
    F. Small drawing / text helpers ......... th(), drawRainbowLine(),
                                              getTextMetrics()
    G. Hourly animations .................... HourDance()
    H. Version, serial commands, settings ... printVers(), handleUserReq(),
                                              loadColorMode(), saveColorMode(),
                                              ShowHelpText()

  "PUNCH-THROUGH" (used a lot below):
    Text is drawn in yellow on a black sprite.  Then every pixel that is
    NOT black is replaced by a pixel from something else (a rainbow or a
    photo).  The result looks like the photo is seen through letter-shaped
    holes cut in black card.
  ==========================================================================*/


/*===========================================================================
  A. TOUCH HELPER
  ===========================================================================*/
///*****************************************************************************/
//bool inBox(int touchX, int touchY, int boxX, int boxY, int boxW, int boxH)
///*****************************************************************************/
//// True if the point (touchX, touchY) is inside the rectangle.
//// REVIEW: Not used - leftover from a touch-screen project.  Keep it if you
////         plan to add touch buttons, otherwise remove.
//{
//  // touch X and Y are the point to be tested to be in the box.
//  // box X and Y are the left and top of the area to be tested to be in.
//  // box W and H are the width and height of the area to be tested to be in.
//  return (touchX >= boxX && touchX <= (boxX + boxW) &&
//          touchY >= boxY && touchY <= (boxY + boxH));
//}
/*===========================================================================
  B. PUNCH-THROUGH EFFECTS
  ---------------------------------------------------------------------------
  REVIEW (speed): All four functions call readPixel() + drawPixel() for
  every pixel - about 40,000 pairs of function calls per line of text, per
  frame.  TFT_eSprite::getPointer() gives direct access to the pixel
  buffer, and a plain array loop over it is many times faster.
  REVIEW (looks): The text is anti-aliased - its edge pixels are dark
  yellow, fading into black.  "c != TFT_BLACK" treats a 5%-yellow edge
  pixel the same as a solid one, so the smooth edges are lost and the
  letters look slightly jagged.  For smoother letters, use the brightness
  of the yellow pixel as a mix amount: tft.alphaBlend(alpha, picColour,
  TFT_BLACK).
  ===========================================================================*/
/*****************************************************************************/
//void punchRainbowShift(TFT_eSprite & spr, uint16_t *rainbowLine,
//                       int lineWidth, int shiftOffset)
///*****************************************************************************/
//// Fills the text in "spr" with a left-to-right rainbow.
////   rainbowLine - colour table (rainbowTable)
////   lineWidth   - number of colours in it (360)
////   shiftOffset - how far to slide the rainbow sideways (animation)
//// The full 360 colours are stretched across the sprite width.
//{
//  static int hueIndex;   // REVIEW: does not need to be static.
//  int w = spr.width();
//  int h = spr.height();
//
//  //  for (int y = 0; y < h; y++) {
//  //    for (int x = 0; x < w; x++) {
//  //      uint16_t c = spr.readPixel(x, y);
//  //      if (c != TFT_BLACK) {
//  //        // Map sprite X into rainbow table range, then add shift
//  //        // (+ w keeps the number positive before scaling.)
//  //        hueIndex = ((x - shiftOffset + w) * lineWidth) / w;
//  //        spr.drawPixel(x, y, rainbowLine[hueIndex % lineWidth]);
//  //      }
//  //    }
//  //  }
//  uint16_t* pixels = static_cast<uint16_t*>(spr.getPointer());
//
//  for (int y = 0; y < h; y++) {
//    uint16_t* row = pixels + y * w;
//
//    for (int x = 0; x < w; x++) {
//      if (row[x] != TFT_BLACK) {
//        // Map sprite X into the rainbow table range, then add shift.
//        hueIndex = ((x - shiftOffset + w) * lineWidth) / w;
//        row[x] = rainbowLine[hueIndex % lineWidth];
//      }
//    }
//  }
//}

// NOTE:
// Direct buffer access via spr.getPointer() was tested here.
// It was faster but produced visible color banding / loss of
// smooth gradients in the rainbow date punch-through.
// Using readPixel()/drawPixel() preserves appearance.
//
// Tested and reverted: Nov 2026.
/*****************************************************************************/
void punchRainbowShift(TFT_eSprite & spr, uint16_t *rainbowLine,
                       int lineWidth, int shiftOffset)
/*****************************************************************************/
// Fills the text in "spr" with a left-to-right rainbow.
//   rainbowLine - colour table (rainbowTable)
//   lineWidth   - number of colours in it (360)
//   shiftOffset - how far to slide the rainbow sideways (animation)
// The full 360 colours are stretched across the sprite width.
{
  static int hueIndex;   // REVIEW: does not need to be static.
  int w = spr.width();
  int h = spr.height();

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint16_t c = spr.readPixel(x, y);
      if (c != TFT_BLACK) {
        // Map sprite X into rainbow table range, then add shift
        // (+ w keeps the number positive before scaling.)
        hueIndex = ((x - shiftOffset + w) * lineWidth) / w;
        spr.drawPixel(x, y, rainbowLine[hueIndex % lineWidth]);
      }
    }
  }
}
///*****************************************************************************/
//void punchImage(TFT_eSprite &sprText, TFT_eSprite &sprImage)
///*****************************************************************************/
//// Fills the text with the picture, not moving.  (Picture tiles/repeats if
//// the text sprite is bigger.)
//// REVIEW: Not used - punchImageMoving() with offsets 0,0 does the same.
//{
//  for (int y = 0; y < sprText.height(); y++) {
//    for (int x = 0; x < sprText.width(); x++) {
//      uint16_t c = sprText.readPixel(x, y);
//      if (c != TFT_BLACK) {
//        // Map pixel from the picture sprite
//        uint16_t imgColor = sprImage.readPixel(x % sprImage.width(),
//                                               y % sprImage.height());
//        sprText.drawPixel(x, y, imgColor);
//      }
//    }
//  }
//}
/*****************************************************************************/
void punchImageMoving(TFT_eSprite &sprText, TFT_eSprite &sprImage,
                      int xOffset, int yOffset)
/*****************************************************************************/
// Fills the text with the picture, shifted by xOffset/yOffset.  Changing
// the offsets a little each frame makes the picture glide behind the text.
// The "%" (remainder) wraps the picture around, so it repeats seamlessly.
{
  // Intended: skip the work if nothing moved and the hour has not changed.
  // REVIEW (BUG): "prevHour" here is the GLOBAL one, which is never updated
  //         (stays -1), so this test is never true and never skips.  The
  //         offsets are also almost never both 0.  Either fix it or delete
  //         the line.
  if ((xOffset + yOffset == 0) && (prevHour == tm->tm_hour)) return;

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

///*****************************************************************************/
//// This version blows up the middle to look more like rotation.
//// Fills the text with the picture rotated by "angle" (radians) around the
//// centre.
//// REVIEW: Not used.  Also, (negative number) % width is negative in C, so
////         for some pixels imgX/imgY < 0 and the "if" below skips them,
////         leaving yellow pixels in the text.  Wrap with
////         ((v % w) + w) % w  to keep it in range.
//void punchImageRotated(TFT_eSprite &sprText, TFT_eSprite &sprImage,
//                       float angle)
///*****************************************************************************/
//{
//  // Precompute sine/cosine for efficiency
//  float cosA = cos(angle);
//  float sinA = sin(angle);
//
//  // Center of the text sprite
//  int cx = sprText.width() / 2;
//  int cy = sprText.height() / 2;
//
//  // Center of the image sprite
//  int icx = sprImage.width() / 2;
//  int icy = sprImage.height() / 2;
//
//  for (int y = 0; y < sprText.height(); y++) {
//    for (int x = 0; x < sprText.width(); x++) {
//      uint16_t c = sprText.readPixel(x, y);
//      if (c != TFT_BLACK) {
//        // Translate coordinates to text center
//        int dx = x - cx;
//        int dy = y - cy;
//
//        // Apply rotation transform
//        int rx = (int)(dx * cosA - dy * sinA);
//        int ry = (int)(dx * sinA + dy * cosA);
//
//        // Map back to image coordinates
//        int imgX = (rx + icx) % sprImage.width();
//        int imgY = (ry + icy) % sprImage.height();
//
//        if (imgX >= 0 && imgY >= 0) {
//          uint16_t imgColor = sprImage.readPixel(imgX, imgY);
//          sprText.drawPixel(x, y, imgColor);
//        }
//      }
//    }
//  }
//}
/*===========================================================================
  C. COLOUR HELPERS
  ---------------------------------------------------------------------------
  How the rainbow is made: three sine waves, 120 degrees apart, one each for
  red, green and blue.  As "hue" goes 0 -> 359 the colour goes once round
  the colour wheel.  sin() gives -1..+1; *127+128 turns it into 1..255.
  ===========================================================================*/

/*****************************************************************************/
// Return rainbow color at given index+offset
uint16_t rainbowColorMode1(int index, int offset)  // Original rainbowColor
/*****************************************************************************/
// index  - which item (segment number, ring number...) - each step = 40 deg
// offset - animation offset in degrees
{
  int hue = (index * 40 + offset) % 360;
  // REVIEW: The formula below is identical to the one in
  //         buildRainbowTable(), so "return rainbowTable[hue];" gives the
  //         same colour without three sin() calls.  (Needs hue >= 0, which
  //         is true for every current caller.)
  return rainbowTable[hue];
  //  return tft.color565(
  //           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
  //           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
  //           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
  //         );
}

/*****************************************************************************/
// Calling this version paints 1 color per digit.
// Assign colors by (x,y) position instead of segment number
uint16_t rainbowColorMode4(int x, int y, int offset)  // washColor from CoPilot
/*****************************************************************************/
{
  // REVIEW (BUG): For hours 10-12 the first digit has x = -30, so
  //         x * 40 = -1200 and "hue" comes out NEGATIVE ("%" keeps the
  //         sign in C).  rainbowTable[negative] reads memory outside the
  //         table -> random colour, possible crash.  Fix:
  int hue = ((x * 40 + y * 20 + offset) % 360 + 360) % 360;
  //  int hue = (x * 40 + y * 20 + offset) % 360;
  return rainbowTable[hue];
  // REVIEW: Everything below is unreachable (after "return") - delete it.
  //  return tft.color565(
  //           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
  //           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
  //           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
  //         );
}

/*****************************************************************************/
void buildRainbowTable()  // The 360º version for a 360 pixel display
/*****************************************************************************/
// Called once in setup().  Stores the colour for every degree 0..359 in
// rainbowTable[] so drawing code can just look it up.
{
  for (int i = 0; i < RAINBOW_SIZE; i++) {
    rainbowTable[i] = tft.color565(
                        (sin((i + 0) * 3.14159 / 180) * 127 + 128),
                        (sin((i + 120) * 3.14159 / 180) * 127 + 128),
                        (sin((i + 240) * 3.14159 / 180) * 127 + 128)
                      );
  }
}

/* 3 routines for the T7 graph rainbow line */

///*****************************************************************************/
//// Build rainbow table scaled to display width
//void buildRainbowTable(int displayWidth)  // To be used on T7 XRate graph
///*****************************************************************************/
//{
//  for (int i = 0; i < displayWidth; i++) {
//    // Map pixel index [0..displayWidth-1] into [0..360)
//    float hue = (i * 360.0f) / displayWidth;
//
//    rainbowTable[i] = tft.color565(
//                        (sin((hue + 0)   * 3.14159 / 180) * 127 + 128),
//                        (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
//                        (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
//                      );
//  }
//
//}
//int rainbowOffset = 0;   // current animation offset
//int displayWidth = 240;  // your new display width
//
//void drawRainbowLine() {
//  for (int x = 0; x < displayWidth; x++) {
//    // Wrap around the rainbow table using modulo
//    int index = (x + rainbowOffset) % displayWidth;
//    tft.drawPixel(x, 0, rainbowTable[index]);  // draw at row 0, adjust as needed
//  }
//}
//
//void animateRainbow() {
//  rainbowOffset++;
//  if (rainbowOffset >= displayWidth) {
//    rainbowOffset = 0;   // wrap back to start
//  }
//  drawRainbowLine();
//}
//
//End T7 routines

///*****************************************************************************/
//// Return rainbow color for a given pixel
//uint16_t rainbowLineColor(int x, int offset)
///*****************************************************************************/
//// REVIEW: Not used (drawRainbowLine() does this with the table).
//{
//  // Spread rainbow across width by scaling x
//  int hue = ((x * 360) / tft.width() + offset) % 360;
//  return tft.color565(
//           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
//           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
//           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
//         );
//  //  return (tft.color565(offset, offset + x, x));
//}
///*****************************************************************************/
//// Return contrast color (180° phase shift)
//uint16_t contrastColor(int index, int offset)
///*****************************************************************************/
//// The colour on the opposite side of the colour wheel.
//// REVIEW: Not used.
//{
//  int hue = (index * 40 + offset + 180) % 360; // shifted by 180°
//  return rainbowTable[hue];
//  //  return tft.color565(
//  //           (sin((hue + 0) * 3.14159 / 180) * 127 + 128),
//  //           (sin((hue + 120) * 3.14159 / 180) * 127 + 128),
//  //           (sin((hue + 240) * 3.14159 / 180) * 127 + 128)
//  //         );
//}
/*===========================================================================
  D. FONT LOADING
  ===========================================================================*/

/*****************************************************************************/
/*
   Call it thus:
   safeLoadOFR_Font("EldoradoRomanTTF", EldoradoRomanTTF,
                sizeof(EldoradoRomanTTF), __LINE__, __func__);
*/
// Loads a TrueType font into OpenFontRender, but ONLY if it is not already
// the loaded font - so it is cheap to call every frame.
//   fontName - any label; used to recognise "same font as last time"
//   __LINE__ / __func__ - where it was called from, for error messages
// Returns true on success.
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

  // REVIEW: unloadFont() does not throw C++ exceptions, so try/catch does
  //         nothing here.  A plain ofr.unloadFont(); is enough.
  int rc = 0;  // try {
  ofr.unloadFont();
  //  } catch (...) {
  //    Serial.println("unloadFont failed, continuing anyway...");
  //  }

  rc = ofr.loadFont(fontData, size);   // 0 = OK, anything else = error
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
// Prints a readable font-load error on the serial monitor.
{
  Serial.printf("Render loadFont error for %s\n", fontName);
  // REVIEW: "line - 2" is odd - __LINE__ is already the line of the call.
  Serial.printf("In routine %s, near line %d\n", func, line - 2);
  Serial.printf("Error code %i\r\n", rc);
  // Optional: map known error codes to text
  // REVIEW: OpenFontRender returns FreeType error codes, where
  //         1 = cannot open resource, 2 = unknown file format,
  //         3 = invalid file format.  The texts below are guesses and do
  //         not match.  Printing the number is the reliable part.
  switch (rc) {
    case 1: Serial.println("Error: Cannot open font file."); break;
    case 2: Serial.println("Error: Unknown font file format."); break;
    case 3: Serial.println("Error: Invalid font file format."); break;
    // Add more codes as you discover them
    default: Serial.println("Error: Unknown font file code."); break;
  }
}
/*===========================================================================
  E. WIFI AND TIME
  ===========================================================================*/

/*****************************************************************************/
void startWiFiManager()
/***startWiFiManager**********************************************************/
// Connects to WiFi.  If the saved network fails, opens an access point
// called "ColorTest" (password "BigLittleClock").  Join it from a phone, a
// web page appears, pick your WiFi and enter its password.  If nothing
// happens within the timeout, the ESP32 reboots and tries again.
//
// REVIEW (BUG): autoConnect() is called TWICE.  The first call does all the
//         real work.  The callback that shows "Need SSID" on the screen is
//         only set up AFTER it, so the screen message never appears when
//         it is actually needed.  The second call is redundant (already
//         connected).  Suggested order:
//           setDebugOutput(false); setAPCallback(...); setTimeout(...);
//           if (!autoConnect(myPortalName, "BigLittleClock")) restart;
{

  //  wifiManager.resetSettings(); // wipe settings -- emergency use only.

  // REVIEW: The comment says 30 s but the value is 60 s.  Also note this
  //         is the PORTAL timeout - how long it waits for you to configure.
  wifiManager.setTimeout(60);  // 30 second connect timeout then reboot.
  wifiManager.disconnect();    // REVIEW: not needed before autoConnect().
  //  if (!wifiManager.autoConnect(myPortalName, "BigLittleClock")) {
  //    Serial.println(F("failed to connect and hit timeout"));
  //    //Reboot so we can try again.
  //#ifdef ESP32
  //    ESP.restart();
  //#else
  //    ESP.reset();
  //#endif
  //}
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
// Called by WiFiManager when it opens the set-up access point.  Shows a
// red "Need SSID / Connect to / ColorTest" screen.
// REVIEW: At this point TFT_eSPI is still using its tiny built-in font and
//         top-left text datum, so the text is very small and starts at the
//         centre instead of being centred.  Add tft.setTextDatum(TC_DATUM)
//         and a bigger font (or use ofr, which is already loaded).
{
  Serial.println(F("Entered config mode..."));
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  yPos = 10;
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);  // FG color, BG color
  tft.drawString("Need SSID", tft.width() / 2, yPos);
  tft.drawString("Connect to", tft.width() / 2, yPos + 43);
  tft.drawString(myPortalName, tft.width() / 2, yPos + 85);
}

/*****************************************************************************/
void timeSyncCallback(struct timeval * tv)
/*****************************************************************************/
// Called automatically every time SNTP sets the clock (at start-up, then
// about once a day).  Just logs it to the serial monitor.
// REVIEW: This runs in the network task, not in loop().  Writing the shared
//         global workTime_t from here can collide with loop() using it.
//         It is not needed anyway (loop() reads the time every frame) -
//         remove the time(&workTime_t) line.
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
  //  time(&workTime_t);
}


/*===========================================================================
  F. SMALL DRAWING / TEXT HELPERS
  ===========================================================================*/

/*****************************************************************************/
String th(int theDate)
/*****************************************************************************/
// Ordinal suffix for a day of the month: 1st, 2nd, 3rd, 4th ... 11th,
// 12th, 13th ... 21st, 22nd, 23rd ... 31st.  (11-13 correctly get "th".)
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
// Draws the 1-pixel rainbow divider across the full width at height y.
// The whole 360-degree rainbow is spread over the screen width; "offset"
// slides it sideways for animation.  Drawn into a sprite first, then sent
// to the screen in one go (no flicker).
{
  static int hueIndex;   // REVIEW: does not need to be static.

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
// Measures "text" in the currently loaded ofr font at "fontSize".
//   width          - pixels wide
//   height         - pixels tall + descenderPad (room for g, j, y tails)
//   baselineOffset - rough distance from baseline to bottom
{
  TextMetrics m;
  m.width  = ofr.getTextWidth(text, fontSize);
  m.height = ofr.getTextHeight(text, fontSize) + descenderPad;

  // Use "A" as a proxy for ascent to get baseline offset
  m.baselineOffset = m.height - ofr.getTextHeight("A", fontSize);

  return m;
}


/*===========================================================================
  G. HOURLY ANIMATIONS
  ---------------------------------------------------------------------------
  HourDance(n) plays animation n (0..15).  The clock is frozen while it
  runs (each takes a few seconds).  Most use "static int rainbowOffset"
  so the colours are a little different every time that dance plays.
  Every dance finishes by clearing the screen to black.
  Your own test notes about each dance are at the bottom of this file.
  ===========================================================================*/

/*****************************************************************************/
void HourDance(int which)
/*****************************************************************************/
{
  //  static int whichDance = 0;                     // REVIEW: unused.
  // REVIEW: A static is only initialised on the FIRST call, so this
  //         remembers the datum from the first dance forever.  A plain
  //         local (no "static") saves the CURRENT datum each time.
  int prevDatum = tft.getTextDatum();

  switch (which)
  {
    //------------------------------------------------------------------------
    case 0:   // Screen flashes negative 4 times
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

    //------------------------------------------------------------------------
    case 1:   // White outlines shrink to the centre, rainbow ones grow back
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

    //------------------------------------------------------------------------
    case 2:   // Rainbow rings ripple out from the centre
      {
        Serial.println("2-HourDance: Concentric Circles (Ripple Effect)");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;
        // REVIEW: Your note says this "stops at small dimension".  The
        //         corners are sqrt(240^2 + 160^2) = ~289 px from the centre,
        //         but maxR is 240.  Use:  int maxR = sqrt(cx*cx + cy*cy) + 1;
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

    //------------------------------------------------------------------------
    case 3:   // Lines sweep in from all four corners
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

    //------------------------------------------------------------------------
    case 4:   // Solid rainbow rectangles grow from the centre
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

    //------------------------------------------------------------------------
    case 5:   // One rainbow spiral winds out from the centre (10 turns)
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

    //------------------------------------------------------------------------
    case 6:   // Thin rainbow bars wipe left to right
      {
        Serial.println("6-HourDance: Vertical Bars Pulse");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        // REVIEW: Your note says it looks ugly and the colours should be
        //         wider.  The index is x, which steps by 6, and
        //         rainbowColorMode1 multiplies it by 40 - so each bar is 240
        //         degrees round the colour wheel from its neighbour and the
        //         colours clash.  Pass x / 6 (40 deg per bar) or x / 20 for
        //         smooth, wider bands.
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

    //------------------------------------------------------------------------
    case 7:   // Rainbow rays from the centre to the edges
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

    //------------------------------------------------------------------------
    case 8:   // Checkerboard of rainbow squares
      {
        Serial.println("8-HourDance: Checkerboard Flash");
        int w = tft.width();
        int h = tft.height();
        int size = 20;
        static int rainbowOffset = 0;

        // REVIEW: Your note says the colours change too slowly - only 3
        //         frames, offset moves 0..39 degrees between them.  A bigger
        //         jump (e.g. +60) and more repeats would help.
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

    //------------------------------------------------------------------------
    case 9: {   // "BONG" fades red -> green -> blue
        static int fade;
        static uint16_t color;

        tft.setTextDatum(MC_DATUM);   // REVIEW: affects tft text, not ofr.

        Serial.println("9-HourDance: The Big Bong");

        int h = tft.height();
        int cy = h / 2;

        safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                         sizeof(BritanicBoldTTF), __LINE__, __func__);
        // REVIEW: At size 200, check ofr.getTextWidth("BONG") fits in 480 px.
        //         Your note says "No text just boxes" - try a smaller size
        //         first to see whether it is a size / cache problem.
        ofr.setFontSize(200);
        ofr.setDrawer(tft);           // Draw straight onto the screen.

        int textH = ofr.getTextHeight("BONG", ofr.getFontSize());
        int y = cy - textH / 2;       // Vertically centred.

        // Fade loops
        for (fade = 255; fade >= 0; fade -= 20) {
          color = tft.color565(fade, 0, 0);
          ofr.setFontColor(color);
          ofr.setCursor(tft.width() / 2, y);
          ofr.cprintf("BONG");
          delay(50);
        }
        for (fade = 255; fade >= 0; fade -= 20) {
          color = tft.color565(0, fade, 0);
          ofr.setFontColor(color);
          ofr.setCursor(tft.width() / 2, y);
          ofr.cprintf("BONG");
          delay(50);
        }
        for (fade = 255; fade >= 0; fade -= 20) {
          color = tft.color565(0, 0, fade);
          ofr.setFontColor(color);
          ofr.setCursor(tft.width() / 2, y);
          ofr.cprintf("BONG");
          delay(50);
        }

        ofr.setDrawer(spriteDate); // back to date display
        tft.setTextDatum(prevDatum); // restore
        break;
      }

    //------------------------------------------------------------------------
    case 10:   // Growing filled rainbow circles
      {
        Serial.println("10-HourDance: Fill with Circles");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;

        // REVIEW: Goes to r = 320 but the corners are covered at ~289, so
        //         the last few circles are wasted time (your "lasts too
        //         long" note).
        for (int r = 0; r < min(w, h); r += 8) {
          uint16_t color = rainbowColorMode1(r, 0);
          tft.fillCircle(cx, cy, r, color);
          delay(20);
        }
        break;
      }

    //------------------------------------------------------------------------
    case 11:   // Nested filled rainbow rectangles, outside -> in
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

    //------------------------------------------------------------------------
    case 12:   // Two spiral arms, opposite each other
      {
        Serial.println("12-HourDance: Double Spiral");
        int w = tft.width();
        int h = tft.height();
        int cx = w / 2;
        int cy = h / 2;
        static int rainbowOffset = 0;

        // REVIEW: Your note says it stops too soon: at angle 3600, r = 180,
        //         which only reaches the top/bottom edges (160) - not the
        //         sides (240) or corners (289).  Use angle/12 (r = 300) to
        //         reach the corners.
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

    //------------------------------------------------------------------------
    case 13:   // Random coloured dots
      {
        Serial.println("13-HourDance: Sparkle Field");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        // REVIEW: "Needs more spots": 2000 dots on 153,600 pixels is ~1%.
        //         The delay(2) per dot is what limits it (4 s per burst).
        //         Drawing small fillCircle()s or removing the delay gives
        //         far more sparkle in the same time.
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

    //------------------------------------------------------------------------
    case 14: {   // Rainbow wipe left to right
        Serial.println("14-HourDance: Rainbow Wipe");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        // REVIEW: Nearly the same as dance 6 (6-px bars, same clashing
        //         colours) - which may be why you "missed it".  For one
        //         smooth rainbow across the screen use
        //           rainbowTable[(x * 360 / w + rainbowOffset) % 360]
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

    //------------------------------------------------------------------------
    case 15: {   // Horizontal rainbow stripes scroll through the colours
        Serial.println("15-HourDance: Gradient Fade");
        int w = tft.width();
        int h = tft.height();
        static int rainbowOffset = 0;

        // REVIEW: rainbowColorMode1 multiplies the index by 40, so each
        //         row is 40 degrees from the next and the rainbow repeats
        //         every 9 rows - fine stripes, not a fade.  For a true
        //         top-to-bottom gradient use
        //           rainbowTable[(shift + y * 360 / h + rainbowOffset) % 360]
        //         It "runs far too long" because it draws 320 rows x 24
        //         shifts x 2 runs, with 3 sin() calls per row.
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
  tft.setTextDatum(prevDatum);   // Whatever it was!
  tft.fillScreen(TFT_BLACK);     // Hide the bodies!  I was never here!!
}
/*===========================================================================
  H. VERSION, SERIAL COMMANDS AND SAVED SETTINGS
  ===========================================================================*/
/***************************************************************************/
void printVers()
/***************************************************************************/
// Works out the version number from the sketch folder name
// (".../Total_Gradient_Clock_PSRAM_v1.9/Utilities.ino" -> "1.9") and
// prints it with the compile date/time.
// REVIEW: This looks for "\" - Windows path separators.  On a Mac or Linux
//         the separator is "/", so sVer becomes "1.9/Utilities".  It also
//         breaks if a later folder name contains a "v".  A plain
//           #define SKETCH_VERSION "1.9"
//         in Definitions.h is simpler and can't go wrong.
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
// Reads one command from the serial monitor and acts on it.
// Format: first character = command, anything after it = argument.
//   R        reboot
//   H or ?   help
//   I        info (time, version, mode)
//   1        mode 1 - everything shimmers
//   2        mode 2 - only the rainbow line moves, white digits
//   3[x]     mode 3 - solid-colour digits; x = B C G R W Y (default Green)
//   4        mode 4 - colour by digit position
// Modes and the mode-3 colour are saved to flash and restored at boot.
{
  static char cUserReq;  //, prevCmd;
  static char inStr[chrBufSize];

  if (!Serial.available()) return;

  cUserReq = toupper(Serial.read());  // Read command character
  // REVIEW: If the serial monitor sends CR+LF, a lone '\r' gets here as a
  //         command and prints "not implemented".  Also skip '\r'.
  if (cUserReq == '\n' || cUserReq == '\r') return;  // was 0x0a
  Serial.print("Command char read: "); Serial.println(cUserReq);

  // Clear buffer and reset pointer.
  memset(inStr, 0, chrBufSize); inStrCt = 0;

  // Read the rest of the line.  The delay(20) gives the next character time
  // to arrive (at 115200 baud a character takes < 0.1 ms, so this is very
  // generous).
  // REVIEW: Relying on timing works, but reading until '\n' is the usual,
  //         more reliable way.
  while (Serial.available()) {
    inStr[inStrCt++] = Serial.read(); delay(20);
    if (inStrCt == chrBufSize) {
      Serial.println("Serial Input line too long. Ignored.");
      return;
    }
  }

  // if (inStrCt > 0) {
  //   Serial.printf(" %i characters read from input steam\r\n", inStrCt);
  //   Serial.print("Input: / "); Serial.print(inStr); Serial.println(" / ");
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
      Serial.println("Running from: "); Serial.println(__FILE__);
      Serial.print("Compiled on: "); Serial.print(__DATE__);
      // REVIEW (BUG): prints __DATE__ again - should be __TIME__.
      Serial.print(" at "); Serial.println(__TIME__);
      //      Serial.println("Help text to be added later.  Try typing I");
      // REVIEW: ShowHelpText() already exists - call it here.
      //      for (int temp = 0; temp < helpLines; temp++) {
      //        Serial.println(helpString[temp]);
      //      }
      ShowHelpText();
      break;

    // Show selected operational information.
    case 'I':
      Serial.println(); Serial.print(ctime(&workTime_t));
      // REVIEW (BUG): printf "%s" needs a C string, but sVer is an Arduino
      //         String object.  This prints garbage or can crash.  Use
      //         sVer.c_str().
      Serial.printf("Gradient Clock vers %s\r\n", sVer.c_str());
      Serial.printf("Display mode is %d\r\n", displayMode);
      Serial.println("Help text to be added later.");
      break;

    // All field shimmer.
    case '1':
      Serial.println("\r\n1 - Display mode set to 1 - Segment Shimmering");
      displayMode = DM_FULL_SHIMMER;
      saveColorMode(); break;
    // Only the rainbow line moves.
    case '2':
      Serial.println("\r\n2 - Display mode set to 2 - RL Only Shimmering");
      displayMode = DM_RL_SHIMMER;
      saveColorMode(); break;
    // Only the rainbow line moves.
    // (Comment above is a copy/paste slip - mode 4 is "whole digit".)
    case '4':
      Serial.println("\r\n4 - Display mode set to 4 - Wholedigit Shimmer");
      displayMode = DM_WHOLE_DIGIT;
      saveColorMode(); break;
    // Solid colour digits.  Optional second character picks the colour.
    case '3':
      Serial.println("\r\n3 - Display mode set to 3 - Single Color per digit");
      // Serial.println("Display mode 3 not implemented, defaulting to mode 1.");
      displayMode = DM_STATIC_DIGITS;
      mode3Color = TFT_GREEN; // default bright green

      if (inStrCt > 0) {
        char arg = toupper(inStr[0]);  // first extra char
        switch (arg) {
          case 'B':
            Serial.println("Mode 3 color set to TFT_BLUE");
            mode3Color = TFT_BLUE; break;
          case 'C':
            Serial.println("Mode 3 color set to TFT_CYAN");
            mode3Color = TFT_CYAN; break;
          case 'G':
            Serial.println("Mode 3 color set to TFT_GREEN");
            mode3Color = TFT_GREEN; break;
          case 'R':
            Serial.println("Mode 3 color set TFT_RED");
            mode3Color = TFT_RED; break;
          case 'W':
            Serial.println("Mode 3 color set to TFT_WHITE");
            mode3Color = TFT_WHITE; break;
          case 'Y':
            Serial.println("Mode 3 color set to TFT_YELLOW");
            mode3Color = TFT_YELLOW; break;
          // REVIEW: Plain "3" + Enter puts "\r" or "\n" in inStr[0], so it
          //         lands here and gives WHITE, not the green the help text
          //         promises.  Check for '\r' / '\n' / ' ' too.
          case '\r': break;
          case '\n': break;
          default:  // fallback
            Serial.println("Mode 3 color defaulted to TFT_WHITE");
            mode3Color = TFT_WHITE; break;
        }
      } else {
        Serial.println("Mode 3 default color set to White");
      }
      saveColorMode(); break;

    default:
      Serial.println("\r\nCommand / Mode not implemented.");
      ShowHelpText();
      break;
  }
  //  prevCmd = cUserReq;  // Remember the previous command in case of X
  // REVIEW: there is no X command; prevCmd is unused.
}

/*****************************************************************************/
void loadColorMode()
/*****************************************************************************/
// Reads the saved display mode and mode-3 colour from flash.
// "blc" is just the storage namespace name (a folder for our settings).
{
  prefs.begin("blc", true);    // namespace "blc", read-only
  // (Comment below is out of date - the default is 1, not 0.)
  displayMode = prefs.getInt("displayMode", 1);  // default to 0 if not set
  mode3Color = prefs.getUShort("mode3Color", TFT_GREEN);
  prefs.end();
  Serial.printf("Display mode set to %d\n", displayMode);
  Serial.printf("Mode 3 color set to 0x %04X\n", mode3Color);
}

/*****************************************************************************/
void saveColorMode()
/*****************************************************************************/
// Saves the current display mode and mode-3 colour to flash.
{
  Serial.printf("Saving Display mode %d\n", displayMode);
  Serial.printf("Saving Mode 3 color 0x %04X\n", mode3Color);
  prefs.begin("blc", false);   // namespace "blc", RW mode
  prefs.putInt("displayMode", displayMode);
  prefs.putUShort("mode3Color", mode3Color);
  prefs.end();
}

/*****************************************************************************/
void ShowHelpText()
/*****************************************************************************/
// Lists the serial commands.
// REVIEW: Missing "\r\n" after "Solid Green color per digit." (the next
//         line runs on), and modes 4, I, H and R are not listed.
{
  Serial.print("Enter one of the following mode settings: \r\n"
               "1  - All Shimmer\r\n"
               "2  - Only Rainbow line moves.\r\n"
               "3  - Solid Green color per digit."
               "3x - Solid color per digit, where x is: \r\n"
               "     B - Blue\r\n"
               "     C - Cyan\r\n"
               "     G - Green\r\n"
               "     R - Red\r\n"
               "     W - White\r\n"
               "     Y - Yellow\r\n");
  Serial.printf("Current display mode: %d\n", displayMode);
}

/*  ---- Your test notes on each HourDance (kept as-is) ----
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
