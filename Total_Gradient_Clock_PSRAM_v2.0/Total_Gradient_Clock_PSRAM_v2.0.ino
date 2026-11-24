/*============================================================================
  Total Gradient Clock (PSRAM) v1.9  -  "Multicolor Segments"
  ============================================================================
  WHAT THIS IS
    A WiFi-synced 12-hour clock for an ESP32 driving a 480x320 ILI9488 SPI
    TFT (landscape).  The screen is laid out top-to-bottom as:

      y =   3  : Big 7-segment time "HH:MM" (each segment drawn separately so
                 every segment can have its own rainbow colour)
      y = 147  : 1-pixel-tall animated rainbow line (a divider)
      y = 148  : Day of week ("Wednesday"), the yellow text is used as a
                 stencil and a scrolling photo (PinkBlossoms) shows through it
      y = 148+H: Month + date ("Nov 23rd"), filled with a scrolling rainbow

    At the top of every hour an "HourDance" animation plays (16 kinds,
    picked at random) and the backlight is set from an hour-by-hour table.

  THE 7-SEGMENT TRICK (important to remember)
    The font DSEG7_Classic_Bold140 (in My_Fonts.h) is a custom TFT_eSPI
    "smooth font" with only 8 glyphs: '0'..'6' and ':'.  Glyph '0' is NOT
    the number zero - it is segment 0 (top bar) alone, '1' is segment 1
    alone, and so on.  Drawing glyphs '0','1','2'... on top of each other at
    the same x,y builds up a full digit, one coloured segment at a time.

             --0--
            |     |
            5     1
            |     |
             --6--
            |     |
            4     2
            |     |
             --3--

  FILE MAP
    Total_Gradient_Clock_PSRAM_v1.9.ino  setup() and loop() (this file)
    Definitions.h   libraries, globals, constants, pin notes
    AllScroll.ino   the per-frame drawing: time digits, date fields
    Utilities.ino   helpers: colours, punch-through effects, WiFi, time
                    callbacks, HourDance animations, serial commands,
                    saving settings to flash
    My_Fonts.h      font data (TTF files and the segment font)
    TwoJPGs.h       two 434x80 pictures (Fireworks, PinkBlossoms)
    Free_Fonts.h    TFT_eSPI's standard Free Fonts helper (not really used)
    FS_Fireworks.h  480x320 full-screen picture - NOT included anywhere

  SERIAL COMMANDS (115200 baud) - see handleUserReq() in Utilities.ino
    1 / 2 / 3[x] / 4 = display mode,  I = info,  H or ? = help,  R = reboot

  Comments marked "REVIEW:" are review notes (bugs, risks, suggestions).
  See REVIEW.md for the full write-up.
  ==========================================================================*/

#include "Definitions.h"

/*****************************************************************************/
void setup()
/*****************************************************************************/
{
  struct tm timeinfo = {0};   // Filled in by getLocalTime() below.
  int ofrFontHeight;          // REVIEW: only used by a dead line (see below).

  //--------------------------------------------------------------------------
  // 1. Serial port + version banner
  //--------------------------------------------------------------------------
  Serial.begin(115200); delay(2000);  // 2 s lets the USB serial monitor attach.
  //  Serial.println("Running from:"); Serial.println(__FILE__);
  //  Serial.printf("Compiled on %s at %s\r\n", __DATE__, __TIME__);
  printVers();                        // Prints folder, version, build date.
  Serial.print("Board detected: ");
  Serial.println(ARDUINO_BOARD);

  //--------------------------------------------------------------------------
  // 2. Display hardware
  //    Rotation 1 = landscape (480 wide x 320 tall).
  //    setSwapBytes(true) = pictures stored in PROGMEM arrays are in the
  //    "normal" byte order and must be swapped for the SPI display.
  //--------------------------------------------------------------------------
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.invertDisplay(false);

  dispWidth = tft.width();      // 480 in landscape
  dispHeight = tft.height();    // 320 in landscape
//  xCenter = dispWidth  / 2;
//  yCenter = dispHeight / 2;

  // Backlight: PWM at 5 kHz, 8-bit (0..255).  200 = fairly bright during
  // start-up; the hourly table in Definitions.h takes over later.
#if defined TFT_BL
  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  ledcWrite(TFT_BL, 200);
#endif

  // OpenFontRender (ofr) draws TrueType fonts.  "setDrawer" tells it WHERE
  // to draw: the real screen (tft) now, the date sprite later.
  ofr.setDrawer(tft); // Link renderer to base display for init messages..

  //--------------------------------------------------------------------------
  // 3. "Initializing" splash screen
  //--------------------------------------------------------------------------
  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF), __LINE__, __func__);
  // print FreeType version
  Serial.print("Freetype version: "); ofr.showFreeTypeVersion();
  ofr.setFontSize(50);

  tft.fillScreen(SkyBlue);  // Foreground color, Background color
  ofr.setFontColor(TFT_BLACK, SkyBlue);
  // Include caps and descenders to get the full height
  myBuildString = "ABCabcjyWM";
  //  ofrFontHeight = ofr.getTextHeight(myBuildString.c_str());
  ofrFontHeight = ofr.getTextHeight(sizingChars);
  ofrFontHeight += 8;

  // cprintf = "centre print": text is centred on the cursor's x position.
  myBuildString = "Multicolor Segments";
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf(myBuildString.c_str());
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf("Initializing");

  //  tft.fillRect(0, dispLine3, dispWidth, ofrFontHeight, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("WiFiManager");

  //--------------------------------------------------------------------------
  // 4. WiFi
  //    Connects using saved credentials, or opens the "ColorTest" access
  //    point / web portal so you can enter them.  Reboots on failure.
  //--------------------------------------------------------------------------
  startWiFiManager();
  IPAddress ip = WiFi.localIP();
  sprintf(workDate, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  //  Serial.print("Connected on "); Serial.println(workDate);

  // Show "WiFi connected on <IP> to <SSID>" for 3 seconds.
  tft.fillScreen(SkyBlue);
  // Foreground color, Background color
  // REVIEW: tft.setTextColor() does not affect ofr text; ofr.setFontColor()
  //         is what matters (it is still set from above, so it works).
  tft.setTextColor(TFT_BLACK, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf("WiFi");
  ofr.setCursor(dispWidth / 2, dispLine2); ofr.cprintf("connected on");
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf(workDate);
  ofr.setCursor(dispWidth / 2, dispLine4); ofr.cprintf("to");
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf(WiFi.SSID().c_str());
  // Wait 3 seconds.  This is non-blocking for events.
  // REVIEW: This IS blocking - it is exactly the same as delay(3000).
  //         WiFi/system tasks keep running either way because delay()
  //         yields to FreeRTOS.  Replace with a plain delay(3000).
  for (loopWait = 0; loopWait < 3000; loopWait++) delay(1);

  // REVIEW: dead line - yPos is never set (always 0) and the very next
  //         line clears the whole screen anyway.  Safe to delete, along
  //         with ofrFontHeight above.
//  tft.fillRect(0, yPos + 160, dispWidth, ofrFontHeight, SkyBlue);
  tft.fillScreen(SkyBlue);
  myBuildString = "Multicolor Segments";
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf(myBuildString.c_str());
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf("Initializing");
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("Time");
  Serial.println("Getting time epoch");

  //--------------------------------------------------------------------------
  // 5. Network time (SNTP)
  //    "PHT-8" is the POSIX time-zone string for Philippine Time (UTC+8, no
  //    daylight saving).  Note POSIX signs are reversed: -8 means "8 hours
  //    AHEAD of UTC".  Three NTP servers are given as fall-backs.
  //    The ESP32 then re-syncs on its own every ~24 h (interval below).
  //--------------------------------------------------------------------------
  // REVIEW: The comment says "a full minute" but each pass of the while
  //         loop can wait up to 5 s inside getLocalTime(), so 25 passes is
  //         up to ~2 minutes before ESP.restart().
  int looper = 25;  // Up to ~2 minutes
  sntp_set_time_sync_notification_cb(timeSyncCallback);  // Log each sync.
  // REVIEW: sntp_set_sync_interval / sntp_restart are the old names; newer
  //         ESP32 cores prefer esp_sntp_set_sync_interval / esp_sntp_restart.
  esp_sntp_set_sync_interval(86407000);  // 1 day + 7 secs in ms.
  configTzTime("PHT-8",
               "pool.ntp.org", "time.nist.gov", "time.google.com");
  delay(5000);  // Give it a little chance if it is fast enough.

  // getLocalTime() returns false until the clock holds a real time
  // (internally: until the year is later than 2016).
  while (!getLocalTime(&timeinfo, 5000)) {  // Times out in 5 seconds.
    if (looper % 5 == 0) {           // Every 5th try, kick SNTP again.
      Serial.println("Restarting sNTP to issue new request.");
      esp_sntp_restart();
    }
    if (looper-- < 1) ESP.restart();  // Quit trying after a minute.
    Serial.printf("Waiting for initial time (%d)\r\n", looper);
    tft.fillRect(0, dispLine5, tft.width(), 80, SkyBlue);
    ofr.setCursor(dispWidth / 2, dispLine5);
    ofr.cprintf("Time (%d)", looper);
    // vTaskDelay(2000 / portTICK_PERIOD_MS);
  }

  // Now, wait for the correct year, meaning probably the right time, too.
  // REVIEW: workTime_t has not been read yet (it is still 0 = 1 Jan 1970),
  //         so this first strftime always produces "1970" and the loop
  //         below always runs at least once.  timeinfo.tm_year + 1900 already
  //         holds the real year from getLocalTime().
  // REVIEW: The hard-coded 2026 means a build compiled for an older year
  //         would work, but think about it if you reuse this code - a
  //         constant like MIN_VALID_YEAR in Definitions.h is clearer.
  time(&workTime_t);    
  strftime(workTime, sizeof(workTime), "%Y", localtime(&workTime_t));
  iYear = atoi(workTime);
  //  Serial.printf("Initial year retrieved is %d\r\n", iYear);
  int iLooper = 0;
  while (iYear < 2026) {
    time(&workTime_t);                         // Read the system clock.
    delay(2000);
    strftime (workTime, 100, "%Y", localtime(&workTime_t));
    iYear = atoi(workTime);
    Serial.println(localtime(&workTime_t), "Local time: %a %m-%d-%Y %T");
    if (iLooper++ > 30) {                      // 30 x 2 s = 1 minute
      Serial.println("Cannot get time set after a minute. Rebooting.");
      ESP.restart();
    }
  }

  //--------------------------------------------------------------------------
  // 6. Sprites (off-screen drawing buffers)
  //    A sprite is drawn in RAM and then "pushed" to the screen in one go,
  //    which avoids flicker.  Big sprites go into PSRAM if it is enabled.
  //--------------------------------------------------------------------------
  tft.fillScreen(SkyBlue);
  myBuildString = "Multicolor Segments";
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf(myBuildString.c_str());
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf("Initializing");
  ofr.setCursor(dispWidth / 2, dispLine4); ofr.cprintf("Sprites");

  // 6a. rainbow1LineSprite: 480 x 1 pixel strip for the rainbow divider.
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("rainbow1LineSprite");
  delay(1000);
  // REVIEW: createSprite() returns void*; "void *a" (and comparing with
  //         nullptr) is cleaner than casting to int*.
  int *a;  // rainbow1LineSprite (the small one liner)
  a = (int*)rainbow1LineSprite.createSprite(tft.width(), 1);  // one line tall
  if (a == 0) {
    // REVIEW: while(1) hangs forever with a frozen screen.  Showing a
    //         message on the TFT, waiting, then ESP.restart() is friendlier.
    Serial.println("rainbow1LineSprite creation failed.  Cannot continue.");
    while (1);
  }
  buildRainbowTable();  // Pre-compute the 360 rainbow colours once.

  // 6b. Measure the widest day name ("Wednesday") at the date font size,
  //     so the date sprite is big enough for any day.
  //     (The font is already loaded, so this call is skipped internally.)
  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF),
                   __LINE__, __func__);
  ofr.setFontSize(DATE_FONT_SIZE);  // Probably useless.
  // REVIEW: The local name "tm" hides the global "tm" pointer used
  //         everywhere else.  It works, but it is confusing - rename to
  //         something like "dayMetrics".
  TextMetrics dayMetrics = getTextMetrics(testDay, DATE_FONT_SIZE);
  // REVIEW: The +42 is a fudge.  The text is later centred at
  //         tft.width()/2 = 240 INSIDE this sprite, which only lines up
  //         because the sprite is pushed at x = 0.  If the sprite is
  //         narrower than ~2 x half the text width the right side gets
  //         clipped.  Simplest robust fix: make the sprite the full screen
  //         width (dispWidth).  See REVIEW.md.
  spriteDateW = tft.width();
  spriteDateH = dayMetrics.height;
  Serial.printf("Return from getTextWidth %dx%d\r\n",
                spriteDateW, spriteDateH);

  // 6c. spriteDate: where the day/date text is drawn before effects.
  //     (These repeated safeLoadOFR_Font calls are no-ops - same font.)
  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF), __LINE__, __func__);
  // print FreeType version
  ofr.setFontSize(50);  // Reset for the displayed text
  tft.fillRect(0, dispLine5, tft.width(), 80, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("spriteDate");
  delay(1000);
  a = (int*)spriteDate.createSprite(spriteDateW, spriteDateH );
  if (a == 0) {
    Serial.println("spriteDate creation failed.  Cannot continue.");
    while (1);
  }

  // 6d. spritePic: holds the picture that shows through the day-name text.
  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF), __LINE__, __func__);
  // print FreeType version
  ofr.setFontSize(50);  // Reset for the displayed text
  tft.fillRect(0, dispLine5, tft.width(), 80, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("spritePic");
  delay(1000);
  // REVIEW: The picture is 434x80 but this sprite is spriteDateW x
  //         spriteDateH.  If those differ, the extra area stays black and
  //         shows as black stripes when the picture wraps around.  Size
  //         this sprite 434x80 (or read straight from the PROGMEM array and
  //         skip this sprite entirely - saves ~70 KB of RAM).
  a = (int*)spritePic.createSprite(spriteDateW, spriteDateH);
  if (a == 0) {
    Serial.println("spritePic creation failed.  Cannot continue.");
    while (1);
  }
  spritePic.setSwapBytes(true);  // Set for correct endianess

  tft.fillScreen(SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine2); ofr.cprintf("Initialization");
  ofr.setCursor(dispWidth / 2, dispLine4); ofr.cprintf("Complete");
  delay(1000);

  //--------------------------------------------------------------------------
  // 7. Final setup for normal running
  //--------------------------------------------------------------------------
  // Load the special 7-segment font into TFT_eSPI (not ofr).  From here on
  // tft.drawString("3", ...) draws SEGMENT 3, not the number 3.
  tft.unloadFont();
  tft.loadFont(DSEG7_Classic_Bold140);  // must match the array name exactly

  tft.fillScreen(TFT_BLACK);

  // Copy the picture from flash into spritePic.
  spritePic.pushImage(0, 0, 434, 80, PinkBlossoms);
  //  spritePic.pushImage(0, 0, 434, 80, Fireworks);
  /* Testing */
  //  spritePic.pushSprite(0, 0);
  //  delay(1);
  //  tft.fillScreen(TFT_BLACK);
  /* End Testing */

  // From now on ofr draws into spriteDate, not the screen.
  ofr.setDrawer(spriteDate); // Link renderer sprite for date display.

  // Restore the last display mode / mode-3 colour saved in flash.
  // Replace, later with Preferences, default of 1.
  //  displayMode = DM_FULL_SHIMMER;
  loadColorMode();
  Serial.printf("Display mode is %d\r\n", displayMode);

  Serial.println(F("setup() done. To work, we go..."));
}

/*****************************************************************************/
void loop()
/*****************************************************************************/
// Runs over and over as fast as it can.  Each pass = one animation frame.
// REVIEW: There is no frame-rate limit, so the shimmer speed depends on how
//         long a frame takes to draw (and will change if you change the
//         SPI speed, sprite size, or board).  Consider only drawing a new
//         frame every N ms using millis().
{
  // Read the clock once per frame.  "tm" (global) is then used by all the
  // drawing code.  localtime() applies the PHT-8 time zone.
  time(&workTime_t);
  tm = localtime(&workTime_t);

  //  The minimal way
  //  sensorValue = analogRead(sensorPin);
  //  Serial.println(sensorValue);

  // Future idea: light sensor on GPIO 34 for automatic brightness.
  // The way to change to gamma to better match how eyes work.
  //  int raw = analogRead(34);
  //  float L = raw / 4095.0;
  //  float gamma = 0.5;  // tweak for taste
  //  int brightness = int(pow(L, gamma) * 255);

  // Weekly reboot: Sunday (tm_wday 0) at 02:02:02.  Clears any slow memory
  // fragmentation and gets a fresh WiFi/NTP start.
  // REVIEW: This only fires if a frame happens to start during that exact
  //         second.  Frames are short, so it normally works, but a slow
  //         frame could skip it for a week.  Safer: check hour==2 &&
  //         min==2 and use a flag, or reboot when millis() passes 7 days.
  //  if (tm->tm_wday == 0 && tm->tm_hour == 2 && tm->tm_min == 2 &&
  //      tm->tm_sec == 2) {  // Sunday at 02:02:02? Reboot!
  if (millis() > 604800000 &&
      tm->tm_wday == 0 && tm->tm_hour == 2 && tm->tm_min == 2) {
    Serial.println("Time to restart.");
    // REVIEW: ofr is pointed at spriteDate at this point, so this text is
    //         drawn into the sprite, not on screen - you will only see the
    //         blue screen.  Call ofr.setDrawer(tft) first.
    ofr.setDrawer(tft);
    tft.fillScreen(SkyBlue);
    myBuildString = "Multicolor Segments";
    ofr.setCursor(dispWidth / 2, dispLine1);
    ofr.cprintf(myBuildString.c_str());
    ofr.setCursor(dispWidth / 2, dispLine3);
    ofr.cprintf("Rebooting in");
    ofr.setCursor(dispWidth / 2, dispLine5);
    ofr.cprintf("10 seconds.");
    delay(10000); ESP.restart();
  }

  // Any typed command on the serial monitor?
  if (Serial.available()) handleUserReq();

  //  startMillis = millis();
  // All four modes are drawn by the same routine; it checks displayMode
  // internally to decide colours and what moves.
  switch (displayMode) {
    case DM_FULL_SHIMMER:
    case DM_RL_SHIMMER:
    case DM_WHOLE_DIGIT:
    case DM_STATIC_DIGITS:
      UpdateDisplayAS();
      break;
    default:
      Serial.println("Invalid display mode. Resetting to Full Shimmer");
      displayMode = DM_FULL_SHIMMER;
      break;
  }
  //  Serial.printf("Time to update display: %lu\r\n",
  //                millis() - startMillis);
}
