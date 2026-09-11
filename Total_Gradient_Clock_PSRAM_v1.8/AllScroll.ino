/*****************************************************************************/
void drawMyStringAS(const char* str, int digitX, int digitY, int baseOffset)
/*****************************************************************************/
{
  static int digit;
  static int tempHour;

  // Convert 24 hour time to 12 hour time.
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
      if (tempHour > 9)
        tft.drawString(":", digitX - 11, digitY);
      else
        tft.drawString(":", digitX - 3, digitY);

      digitX += (tempHour > 9) ? 10 : 30;

    } else {  // It's a digit
      digit = str[position] - '0';
      if (digit < 0 || digit > 9) {
        Serial.printf("⚠️ Skipping non-digit char '%c' at position %d\n",
                      str[position], position);
        continue;
      }
      if (position == 0 && digit == 0) continue; // suppress leading zero

      // Pass a fresh offset for this digit
      //      drawDigitAS(digit, digitX, digitY, localOffset);
      drawDigitCached(position, digit, digitX, digitY, localOffset);
      digitX += 120;
    }
  }
}
/*****************************************************************************/
// Draw a single digit at (x, y) with rainbow shimmer for active segments,
// and black for inactive segments
void drawDigitAS(int digit, int x, int y, int offset)
/*****************************************************************************/
{
  // Mark active segments for this digit
  bool active[7] = {false};
  for (int i = 0; i < digitCount[digit]; i++) {
    int seg = digitSegments[digit][i];
    if (seg >= 0 && seg < 7) {
      active[seg] = true;
    } else {
      Serial.printf("⚠️ Bad seg index: %d for digit %d\r\n", seg, digit);
    }
  }

  int localOffset = offset;  // start offset for this digit

  // Draw all 7 segments
  for (int seg = 0; seg < 7; seg++) {
    if (active[seg]) {
      // Only step offset when drawing an active segment
      if (displayMode == DM_FULL_SHIMMER)
        tft.setTextColor(rainbowColorMode1(seg, localOffset), TFT_BLACK);
      if (displayMode == DM_WHOLE_DIGIT)
        tft.setTextColor(rainbowColorMode4(x, y, localOffset), TFT_BLACK);
      tft.drawString(String(seg), x, y);  // active → rainbow
      localOffset = (localOffset + 15) % 360;  // step offset only here
    } else {
      // inactive → blank, no offset step
      tft.setTextColor(TFT_BLACK, TFT_BLACK);
      tft.drawString(String(seg), x, y);
    }
  }
}
/*****************************************************************************/
void UpdateDisplayAS()  // mode = 1 shimmer, mode = 2 stable updates
/*****************************************************************************/
{
  static int offset = 0, rlOffset = 0;
  static int prevMinute = -1;
  static int prevHour   = -1;
  static unsigned long prevMillis = 0;
  static int prevHDmin = -1;
  static int testHD = 0;

  // Convert 24h → 12h
  int tempHour = tm->tm_hour;
  if (tempHour == 0) tempHour = 12;
  if (tempHour > 12) tempHour -= 12;

  sprintf(workTime, "%02d:%02d", tempHour, tm->tm_min);
  // Rainbow line shimmer always
  offset = (offset + 5) % 360;

  // Digits
  if (displayMode == DM_FULL_SHIMMER ||
      displayMode == DM_RL_SHIMMER ||
      displayMode == DM_WHOLE_DIGIT  ||
      tm->tm_min != prevMinute) {
    int startX = (tempHour > 9) ? -30 : 50;
    if (prevHour != tm->tm_hour) tft.fillScreen(TFT_BLACK);
    drawMyStringAS(workTime, startX, TIME_LOC, offset);
    prevMinute = tm->tm_min;
  }

  if (displayMode == DM_FULL_SHIMMER ||
      displayMode == DM_WHOLE_DIGIT)
    drawRainbowLine(RL_LOC, rlOffset);

  if (displayMode == DM_FULL_SHIMMER) {
    rlOffset = (rlOffset - 5 + 360) % 360;
    drawRainbowLine(RL_LOC, rlOffset);
  }

  // Mode 2, have to slow down the line
  if (displayMode == DM_RL_SHIMMER &&
      (millis() > prevMillis)) {
    rlOffset = (rlOffset - 5 + 360) % 360;
    drawRainbowLine(RL_LOC, rlOffset);
    prevMillis = millis() + 200;
  }

  // Date fields
  if (displayMode == DM_FULL_SHIMMER) {
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

  // On hour change, recreate the bottom two text fields.
  if (prevHour != tm->tm_hour) {
    safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                     sizeof(BritanicBoldTTF),
                     __LINE__, __func__);
    spriteDate.fillSprite(TFT_BLACK);
    upperString = dayStr(tm->tm_wday + 1);
    ofr.setFontColor(TFT_YELLOW, TFT_BLACK);
    ofr.setFontSize(DATE_FONT_SIZE);  // Redundant, really.
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
    punchImageMoving (spriteDate, spritePic,
                      punchXOffset1, punchYOffset1);

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
    tft.fillScreen(TFT_BLACK);
    Serial.printf("Setting display brightness for hour %02i to %i\r\n",
                  tm->tm_hour, ihourlyBrilliance[tm->tm_hour]);
    ledcWrite(TFT_BL, ihourlyBrilliance[tm->tm_hour]);
    prevHour = tm->tm_hour;
  }
}
/*****************************************************************************/
void shimmerDateFields()
/*****************************************************************************/
{
  static bool   punchDateSwitch = true;
  static int    upperShift, lowerShift;

  //  Serial.println("Entering shimmerDateFields.");

  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF),
                   __LINE__, __func__);

  // Every time through, move the colors either left or right.
  ofr.setFontSize(DATE_FONT_SIZE);
  ofr.setFontColor(TFT_YELLOW, TFT_BLACK);
  punchDateSwitch = !punchDateSwitch;
  if (punchDateSwitch) {
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

    punchXOffset1 -= 2;  // Move the image to the right
    // Correct underflow
    if (punchXOffset1 < 0) punchXOffset1 = spritePic.width();

    punchYOffset1++; if (punchYOffset1 > spritePic.height())
      punchYOffset1 = 0;
    punchImageMoving(spriteDate, spritePic, punchXOffset1, punchYOffset1);

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
}
/*****************************************************************************/
void drawDigitCached(int pos, int digit, int x, int y, int offset)
/*****************************************************************************/
{
  uint8_t newMask = digitMasks[digit];
  uint8_t oldMask = currentMask[pos];
  int localOffset = offset;

  for (int seg = 0; seg < 7; seg++) {
    bool wasActive = oldMask & (1 << seg);
    bool isActive  = newMask & (1 << seg);

    if (isActive) {
      // Always draw active segments (first pass or shimmer)
      tft.setTextColor(rainbowColorMode1(seg, localOffset), TFT_BLACK);
      tft.drawString(String(seg), x, y);
      localOffset = (localOffset + 15) % 360;
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
