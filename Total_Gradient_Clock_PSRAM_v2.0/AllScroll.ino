/*============================================================================
  AllScroll.ino  -  everything that is drawn on every frame
  ----------------------------------------------------------------------------
  UpdateDisplayAS()    one full frame: time, rainbow line, date, hourly jobs
  drawMyStringAS()     walks through "HH:MM" and draws each character
  drawDigitCached()    draws one digit segment-by-segment (current method)
  drawDigitAS()        older version of the above - no longer called
  shimmerDateFields()  re-colours the day line or the date line (alternating)
  ==========================================================================*/

/*****************************************************************************/
void UpdateDisplayAS()  // mode = 1 shimmer, mode = 2 stable updates
/*****************************************************************************/
// Called from loop() once per frame for all four display modes.
// "static" variables keep their value between calls (like globals that
// only this function can see).
{
  static int offset = 0, rlOffset = 0;   // Hue offsets: digits / line.
  static int prevMinute = -1;            // REVIEW: written, never read.
  // REVIEW: This hides the global prevHour in Definitions.h - see note
  //         there.  Only this copy is ever updated.
  //  static int prevHour   = -1;
  static unsigned long prevMillis = 0;   // Timer for the slow rainbow line.
  static int prevHDmin = -1;             // For the HourDance test block.
  static int testHD = 0;

  //--------------------------------------------------------------------------
  // 1. Time digits
  //--------------------------------------------------------------------------
  // Convert 24h → 12h
  int tempHour = tm->tm_hour;
  if (tempHour == 0) tempHour = 12;
  if (tempHour > 12) tempHour -= 12;

  sprintf(workTime, "%02d:%02d", tempHour, tm->tm_min);   // e.g. "09:33"
  // Rainbow line shimmer always
  // Move the digit colours 5 degrees round the colour wheel every frame.
  offset = (offset + 5) % 360;

  // Digits
  if (displayMode == DM_FULL_SHIMMER ||
      displayMode == DM_RL_SHIMMER ||
      displayMode == DM_WHOLE_DIGIT  ||
      tm->tm_min != prevMinute) {
    // Two-digit hours (10-12) need to start further left so "12:00" fits.
    // (-30 means the first digit box starts off the left edge; the glyphs
    //  have blank space on their left, and TFT_eSPI clips anything
    //  off-screen.)
    int startX = (tempHour > 9) ? -30 : 50;
    // New hour: wipe the screen, because the digit x positions may change
    // (e.g. 12:59 -> 1:00) and old segments would otherwise be left behind.
    if (prevHour != tm->tm_hour) tft.fillScreen(TFT_BLACK);
    drawMyStringAS(workTime, startX, TIME_LOC, offset);
    prevMinute = tm->tm_min;
  }
  // REVIEW: In modes 2 and 3 the digits do not change colour, yet all
  //         segments are redrawn every frame.  Redrawing only when the
  //         minute changes (what the commented "if" above was for) would
  //         free a lot of time for the other animations.

  //--------------------------------------------------------------------------
  // 2. Rainbow divider line
  //--------------------------------------------------------------------------
  // REVIEW: In mode 1 the line is drawn here AND again just below, so it
  //         is drawn twice per frame.  In mode 4 rlOffset never changes, so
  //         the line is drawn every frame but never moves.  If mode 4
  //         should have a moving line, add it to the second "if".
  if (displayMode == DM_FULL_SHIMMER ||
      displayMode == DM_WHOLE_DIGIT) {
    rlOffset = (rlOffset - 5 + 360) % 360;
    drawRainbowLine(RL_LOC, rlOffset);
  }

  // Mode 1: line scrolls 5 degrees every frame (the opposite direction to
  // the digits, because we subtract).
  //  if (displayMode == DM_FULL_SHIMMER) {
  //    rlOffset = (rlOffset - 5 + 360) % 360;
  //    drawRainbowLine(RL_LOC, rlOffset);
  //  }

  // Mode 2, have to slow down the line
  // Modes 2 and 3: line scrolls at most once every 200 ms.
  // REVIEW: "millis() > prevMillis" with prevMillis = millis() + 200 breaks
  //         when millis() wraps after ~49.7 days (the weekly reboot hides
  //         this).  The wrap-safe pattern is:
  //           if (millis() - lastLine >= 200) { lastLine = millis(); ... }
  //  if ((displayMode == DM_RL_SHIMMER ||
  //       displayMode == DM_STATIC_DIGITS) && millis() - prevMillis >= 200) {
  //    rlOffset = (rlOffset - 5 + 360) % 360;
  //    drawRainbowLine(RL_LOC, rlOffset);
  //    prevMillis = millis();
  //  }
  if ((displayMode == DM_RL_SHIMMER ||
       displayMode == DM_STATIC_DIGITS) &&
      millis() - prevMillis >= 200)
  {
    rlOffset = (rlOffset - 5 + 360) % 360;
    drawRainbowLine(RL_LOC, rlOffset);
    prevMillis += 200;
  }
  //--------------------------------------------------------------------------
  // 3. Animated date fields (modes 1 and 2 only)
  //--------------------------------------------------------------------------
  // Date fields
  if (displayMode == DM_FULL_SHIMMER ||
      displayMode == DM_RL_SHIMMER) {
    shimmerDateFields();  // your existing shimmer routine
    //  } else if (tm->tm_hour != prevHour) {
    //    //    redrawDateFields();   // only once per hour
    //    shimmerDateFields();
    //    prevHour = tm->tm_hour;
  }

  /* Testing -- Rotate through all HourDances, 1 per minute. */
  //  if (prevHDmin != tm->tm_min) {
  //    prevHDmin = tm->tm_min;
  //    HourDance(testHD % 16); testHD = (testHD + 1) % 16;
  //    HourDance(9);  // ...or just do this one repeatedly.
  //  }
  /* End Testing */

  //--------------------------------------------------------------------------
  // 4. Once-per-hour jobs (also runs on the very first frame after boot,
  //    because prevHour starts at -1)
  //    - rebuild the day and date strings
  //    - play a random HourDance
  //    - set the backlight for this hour
  //--------------------------------------------------------------------------
  // REVIEW (BUG): The date is drawn FIRST and then HourDance() and the
  //         fillScreen() below wipe the whole screen.  In modes 1 and 2
  //         shimmerDateFields() redraws it on the next frame, so you never
  //         notice.  In modes 3 and 4 nothing redraws it, so THE DAY AND
  //         DATE NEVER APPEAR in those modes.  Fix: build the strings, do
  //         the HourDance + clear, and only then draw the two date sprites.
  // On hour change, recreate the bottom two text fields.
  if (prevHour != tm->tm_hour) {
    safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                     sizeof(BritanicBoldTTF),
                     __LINE__, __func__);

    // --- Upper line: day name with the picture showing through it ---
    spriteDate.fillSprite(TFT_BLACK);
    upperString = dayStr(tm->tm_wday + 1);   // TimeLib: 1 = Sunday
    ofr.setFontColor(TFT_YELLOW, TFT_BLACK);
    ofr.setFontSize(DATE_FONT_SIZE);  // Redundant, really.
    // REVIEW: Centred on the SCREEN centre (240) inside the sprite.  See the
    //         spriteDateW note in setup(); spriteDate.width()/2 would be
    //         the sprite's own centre.
    ofr.setCursor(tft.width() / 2, 0);
    ofr.cprintf(upperString.c_str());

    // punchXOffset1 += 2;  // Moves the image to the left
    // Correct increment past end of sprite.
    // if (punchXOffset1 > spritePic.width()) punchXOffset1 = 0;

    punchXOffset1 -= 2;  // Move the image to the right
    // Correct underflow
    if (punchXOffset1 < 0) punchXOffset1 = spritePic.width();
    punchYOffset1 ++; if (punchYOffset1 > spritePic.height())
      punchYOffset1 = 0;
    // Replace every yellow text pixel with the matching picture pixel.
    punchImageMoving(spriteDate, spritePic,
                     punchXOffset1, punchYOffset1);

    spriteDate.pushSprite(0, DATE_LOC);

    // --- Lower line: "Nov 23rd" filled with a rainbow ---
    spriteDate.fillSprite(TFT_BLACK);
    lowerString =  monthShortStr(uint8_t(tm->tm_mon + 1));  // "Nov"
    lowerString += " ";
    lowerString += String(tm->tm_mday);                     // "23"
    lowerString += th(tm->tm_mday);                         // "rd"
    ofr.setCursor(tft.width() / 2, 0);
    ofr.cprintf(lowerString.c_str());
    punchRainbowShift(spriteDate, rainbowTable, RAINBOW_SIZE, 0);
    spriteDate.pushSprite(0, DATE_LOC + spriteDateH);

    // --- The hourly show ---
    HourDance(random(16));          // 0..15
    // HourDance(tm->tm_hour);
    // for (whichDance = 0; whichDance < 16; whichDance++) {
    //   doThis(2) {
    //     HourDance(whichDance);
    //     delay(2000);
    //   }
    // }
    // REVIEW: HourDance() already ends with fillScreen(TFT_BLACK); this
    //         second one is redundant.
    //    tft.fillScreen(TFT_BLACK);

    // --- Backlight for this hour ---
    Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                  tm->tm_hour, ihourlyBrilliance[tm->tm_hour]);
    // REVIEW: setup() guards the backlight with "#if defined TFT_BL" but
    //         this line does not, so the sketch will not compile on a
    //         board setup without TFT_BL.  Wrap it the same way.
#if defined TFT_BL
    ledcWrite(TFT_BL, ihourlyBrilliance[tm->tm_hour]);
#endif
    prevHour = tm->tm_hour;
  }
}

/*****************************************************************************/
void drawMyStringAS(const char* str, int digitX, int digitY, int baseOffset)
/*****************************************************************************/
// Draws "HH:MM" starting at digitX, digitY.
//   - Each digit is 120 px wide.
//   - The colon blinks: lit on odd seconds, black on even seconds.
//   - A leading zero in the hour is skipped ("9:33", not "09:33").
//   - Each character position gets its own colour offset (+60 degrees),
//     so the four digits start at different points on the colour wheel.
{
  // REVIEW: These do not need to be "static" - plain local variables are
  //         clearer and avoid surprises.
  static int digit;
  static int tempHour;

  // Convert 24 hour time to 12 hour time.
  // REVIEW: Same conversion as in UpdateDisplayAS(); pass tempHour in as a
  //         parameter instead of working it out twice.
  tempHour = tm->tm_hour;
  if (tempHour == 0) tempHour = 12;  // Midnight
  if (tempHour > 12) tempHour -= 12;  // 1:00 pm through 11:59 pm

  for (int position = 0; str[position] != '\0'; position++) {
    // Each digit gets its own offset based on position
    int localOffset = (baseOffset + position * 60) % 360;

    if (str[position] == ':') {
      // Colon flashes on/off
      colonOn = tm->tm_sec % 2;
      if (colonOn) {
        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      } else {
        tft.setTextColor(TFT_BLACK);
      }
      // The colon glyph is narrow, so nudge it left and then advance x by
      // less than a full digit.  Different amounts for 1- and 2-digit hours.
      if (tempHour > 9)
        tft.drawString(":", digitX - 11, digitY);
      else
        tft.drawString(":", digitX - 3, digitY);

      digitX += (tempHour > 9) ? 10 : 30;

    } else {  // It's a digit
      digit = str[position] - '0';     // '7' -> 7
      if (digit < 0 || digit > 9) {
        Serial.printf("⚠️ Skipping non-digit char '%c' at position %d\n",
                      str[position], position);
        continue;
      }
      if (position == 0 && digit == 0) continue; // suppress leading zero

      // Pass a fresh offset for this digit
      // drawDigitAS(digit, digitX, digitY, localOffset);
      // NOTE: "position" is the character index 0,1,3,4 - see the
      //       currentMask REVIEW note in Definitions.h.
      drawDigitCached(position, digit, digitX, digitY, localOffset);
      digitX += 120;
    }
  }
}

/*****************************************************************************/
void drawDigitCached(int pos, int digit, int x, int y, int offset)
/*****************************************************************************/
// Draws one digit at (x, y) one segment at a time, using the segment font.
//   pos    - which slot on the clock (used to look up what was there before)
//   digit  - 0..9
//   offset - starting hue (0..359) for this digit's colours
// Segments that are ON are drawn in the mode's colour.
// Segments that WERE on last time but are now OFF are painted black.
// Segments that are off and were already off are skipped (saves time).
{
  uint8_t newMask = digitMasks[digit];
  uint8_t oldMask = currentMask[pos];
  int localOffset = offset;

  for (int seg = 0; seg < 7; seg++) {
    bool wasActive = oldMask & (1 << seg);
    bool isActive  = newMask & (1 << seg);

    if (isActive) {
      // Always draw active segments (first pass or shimmer)
      // tft.setTextColor(rainbowColorMode1(seg, localOffset), TFT_BLACK);

      // Pick this segment's colour according to the display mode.
      switch (displayMode) {
        case DM_FULL_SHIMMER:       // every segment a different colour
          tft.setTextColor(rainbowColorMode1(seg, localOffset), TFT_BLACK);
          break;
        case DM_WHOLE_DIGIT:        // colour based on digit position
          tft.setTextColor(rainbowColorMode4(x, y, offset), TFT_BLACK);
          break;
        case DM_STATIC_DIGITS:      // one fixed colour from flash
          tft.setTextColor(mode3Color, TFT_BLACK);
          break;
        default:  // Mode 2, RL only falls through to here.
          tft.setTextColor(TFT_WHITE, TFT_BLACK);
      }

      // The segment font trick: glyph "0".."6" = segment 0..6 alone.
      // REVIEW: String(seg) builds a heap String 7 times per digit per
      //         frame.  A tiny char array is cheaper:
      char s[2] = {char('0' + seg), 0};
      tft.drawString(s, x, y);
      // tft.drawString(String(seg), x, y);
      localOffset = (localOffset + 15) % 360;   // Next segment: +15 degrees.
    } else if (wasActive && !isActive) {
      // Cleanup: segment used to be lit, now must be blanked
      tft.setTextColor(TFT_BLACK, TFT_BLACK);
      tft.drawString(String(seg), x, y);
    }
    // If inactive both before and after, skip entirely
  }

  // Update cache
  currentMask[pos] = newMask;
}

///*****************************************************************************/
//// Draw a single digit at (x, y) with rainbow shimmer for active segments,
//// and black for inactive segments
//// OLD VERSION - not called any more (drawDigitCached replaced it).  It
//// always redraws all 7 segments, so it is slower.  Kept for reference.
//void drawDigitAS(int digit, int x, int y, int offset)
///*****************************************************************************/
//{
//  // Mark active segments for this digit
//  bool active[7] = {false};
//  for (int i = 0; i < digitCount[digit]; i++) {
//    int seg = digitSegments[digit][i];
//    if (seg >= 0 && seg < 7) {
//      active[seg] = true;
//    } else {
//      Serial.printf("⚠️ Bad seg index: %d for digit %d\r\n", seg, digit);
//    }
//  }
//
//  int localOffset = offset;  // start offset for this digit
//
//  // Draw all 7 segments
//  for (int seg = 0; seg < 7; seg++) {
//    if (active[seg]) {
//      // Only step offset when drawing an active segment
//      switch (displayMode) {
//        case DM_FULL_SHIMMER:
//          tft.setTextColor(rainbowColorMode1(seg, localOffset), TFT_BLACK);
//          break;
//        case DM_WHOLE_DIGIT:
//          tft.setTextColor(rainbowColorMode4(x, y, localOffset), TFT_BLACK);
//          break;
//        case DM_STATIC_DIGITS:
//          tft.setTextColor(mode3Color, TFT_BLACK);
//          break;
//        default:
//          tft.setTextColor(TFT_WHITE, TFT_BLACK);
//      }
//      tft.drawString(String(seg), x, y);  // active → rainbow
//      localOffset = (localOffset + 15) % 360;  // step offset only here
//    } else {
//      // inactive → blank, no offset step
//      tft.setTextColor(TFT_BLACK, TFT_BLACK);
//      tft.drawString(String(seg), x, y);
//    }
//  }
//}
/*****************************************************************************/
void shimmerDateFields()
/*****************************************************************************/
// Animates the two date lines.  To halve the work, it alternates:
//   odd calls  -> redraw the upper line (day name, picture scrolls)
//   even calls -> redraw the lower line (date, rainbow scrolls)
// Each redraw: clear sprite -> draw yellow text -> replace the yellow
// pixels with picture/rainbow pixels -> push sprite to the screen.
{
  static bool punchDateSwitch = true;
  static int  upperShift, lowerShift;
    
  //  Serial.println("Entering shimmerDateFields.");

  // Cheap: returns immediately because the font is already loaded.
  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF),
                   __LINE__, __func__);

  // Every time through, move the colors either left or right.
  ofr.setFontSize(DATE_FONT_SIZE);
  ofr.setFontColor(TFT_YELLOW, TFT_BLACK);
  punchDateSwitch = !punchDateSwitch;
  // REVIEW: Rendering the TrueType text again every frame is the slowest
  //         part of this function, and the text only changes once a day.
  //         Faster: render each line ONCE into its own "mask" sprite (on
  //         the hour), then each frame just copy picture/rainbow pixels
  //         through the mask.  Costs one more sprite of RAM (PSRAM is fine).
  if (punchDateSwitch) {
    // ---- Upper line: day name with moving picture ----
    upperShift = (upperShift + 5) % spriteDate.width();
    spriteDate.fillSprite(TFT_BLACK);
    ofr.setCursor(tft.width() / 2, 0);
    ofr.cprintf(upperString.c_str());
    // Do the rainbow initial punchthrough.
    // punchRainbowShift(spriteDate, rainbowTable, RAINBOW_SIZE,
    //                   upperShift);

    // Punch through the picture contained in spritePic.
    // punchImage(spriteDate, spritePic);

    // punchXOffset1 += 2;  // Moves the image to the left
    // Correct increment past end of sprite.
    // if (punchXOffset1 > spritePic.width()) punchXOffset1 = 0;

    // Picture drifts 2 px right and 1 px up each time = diagonal scroll.
    punchXOffset1 -= 2;  // Move the image to the right
    // Correct underflow
    if (punchXOffset1 < 0) punchXOffset1 = spritePic.width();

    punchYOffset1++; if (punchYOffset1 > spritePic.height())
      punchYOffset1 = 0;
    punchImageMoving(spriteDate, spritePic, punchXOffset1, punchYOffset1);

    spriteDate.pushSprite(0, DATE_LOC);

  } else {
    // ---- Lower line: date with moving rainbow ----
    lowerShift = (lowerShift - 5);
    if (lowerShift < 0) lowerShift = spriteDate.width();
    spriteDate.fillSprite(TFT_BLACK);
    ofr.setCursor(tft.width() / 2, 0);
    ofr.cprintf(lowerString.c_str());
    punchRainbowShift(spriteDate, rainbowTable, RAINBOW_SIZE,
                      lowerShift);
    spriteDate.pushSprite(0, DATE_LOC + spriteDateH);
  }
}
