#include "Definitions.h"
/*****************************************************************************/
void setup()
/*****************************************************************************/
{
  struct tm timeinfo = {0};
  int ofrFontHeight;

  Serial.begin(115200); delay(2000);
  //  Serial.println("Running from:"); Serial.println(__FILE__);
  //  Serial.printf("Compiled on %s at %s\r\n", __DATE__, __TIME__);
  printVers();
  Serial.print("Board detected: ");
  Serial.println(ARDUINO_BOARD);

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.invertDisplay(false);

  dispWidth = tft.width();
  dispHeight = tft.height();
  xCenter = dispWidth  / 2;
  yCenter = dispHeight / 2;

#if defined TFT_BL
  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  ledcWrite(TFT_BL, 200);
#endif

  ofr.setDrawer(tft); // Link renderer to base display for init messages..

  //------------------

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

  myBuildString = "Multicolor Segments";
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf(myBuildString.c_str());
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf("Initializing");

  //  tft.fillRect(0, dispLine3, dispWidth, ofrFontHeight, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("WiFiManager");

  startWiFiManager();
  IPAddress ip = WiFi.localIP();
  sprintf(workDate, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  //  Serial.print("Connected on "); Serial.println(workDate);
  tft.fillScreen(SkyBlue);
  // Foreground color, Background color
  tft.setTextColor(TFT_BLACK, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf("WiFi");
  ofr.setCursor(dispWidth / 2, dispLine2); ofr.cprintf("connected on");
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf(workDate);
  ofr.setCursor(dispWidth / 2, dispLine4); ofr.cprintf("to");
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf(WiFi.SSID().c_str());
  // Wait 3 seconds.  This is non-blocking for events.
  for (loopWait = 0; loopWait < 3000; loopWait++) delay(1);

  tft.fillRect(0, yPos + 160, dispWidth, ofrFontHeight, SkyBlue);
  tft.fillScreen(SkyBlue);
  myBuildString = "Multicolor Segments";
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf(myBuildString.c_str());
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf("Initializing");
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("Time");
  Serial.println("Getting time epoch");

  int looper = 12;  // A full minute
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  sntp_set_sync_interval(86407000);  // 1 day + 7 secs in ms.
  configTzTime("PHT-8", "pool.ntp.org", "time.nist.gov");
  delay(2000);  // Give it a little chance if it is fast enough.
  while (!getLocalTime(&timeinfo, 5000)) {  // Times out in 5 seconds.
    if (looper-- < 1) ESP.restart();  // Quit trying after a minute.
    Serial.printf("Waiting for initial time (%d)\r\n", looper);
    tft.fillRect(0, dispLine5, tft.width(), 80, SkyBlue);
    ofr.setCursor(dispWidth / 2, dispLine5);
    ofr.cprintf("Time (%d)", looper);
    // vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  // Now, wait for the correct year, meaning probably the right time, too.
  strftime(workTime, sizeof(workTime), "%Y", localtime(&workTime_t));
  iYear = atoi(workTime);
  //  Serial.printf("Initial year retrieved is %d\r\n", iYear);
  int iLooper = 0;
  while (iYear < 2026) {
    time(&workTime_t);
    delay(2000);
    strftime (workTime, 100, "%Y", localtime(&workTime_t));
    iYear = atoi(workTime);
    Serial.println(localtime(&workTime_t), "Local time: %a %m-%d-%Y %T");
    if (iLooper++ > 30) {
      Serial.println("Cannot get time set after a minute. Rebooting.");
      ESP.restart();
    }
  }
  tft.fillScreen(SkyBlue);
  myBuildString = "Multicolor Segments";
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf(myBuildString.c_str());
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf("Initializing");
  ofr.setCursor(dispWidth / 2, dispLine4); ofr.cprintf("Sprites");

  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("rainbow1LineSprite");
  delay(1000);
  int *a;  // rainbow1LineSprite (the small one liner)
  a = (int*)rainbow1LineSprite.createSprite(tft.width(), 1);  // one line tall
  if (a == 0) {
    Serial.println("rainbow1LineSprite creation failed.  Cannot continue.");
    while (1);
  }
  buildRainbowTable();

  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF),
                   __LINE__, __func__);
  ofr.setFontSize(DATE_FONT_SIZE);  // Probably useless.
  TextMetrics tm = getTextMetrics(testDay, DATE_FONT_SIZE);
  spriteDateW = tm.width + 42;
  spriteDateH = tm.height;
  Serial.printf("Return from getTextWidth %dx%d\r\n",
                spriteDateW, spriteDateH);


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

  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF), __LINE__, __func__);
  // print FreeType version
  ofr.setFontSize(50);  // Reset for the displayed text
  tft.fillRect(0, dispLine5, tft.width(), 80, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("spritePic");
  delay(1000);
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

  tft.unloadFont();
  tft.loadFont(DSEG7_Classic_Bold140);  // must match the array name exactly

  tft.fillScreen(TFT_BLACK);

  //  spritePic.pushImage(0, 0, 434, 80, PinkBlossoms);
  spritePic.pushImage(0, 0, 434, 80, Fireworks);
  /* Testing */
  //  spritePic.pushSprite(0, 0);
  //  delay(1);
  //  tft.fillScreen(TFT_BLACK);
  /* End Testing */

  ofr.setDrawer(spriteDate); // Link renderer sprite for date display.

  // Replace, later with Preferences, default of 1.
  displayMode = DM_FULL_SHIMMER;
  Serial.printf("Display mode is %d\r\n", displayMode);

  Serial.println(F("setup() done. To work, we go..."));
}
/*****************************************************************************/
void loop()
/*****************************************************************************/
{
  time(&workTime_t);
  tm = localtime(&workTime_t);

  //  The minimal way
  //  sensorValue = analogRead(sensorPin);
  //  Serial.println(sensorValue);

  // The way to change to gamma to better match how eyes work.
  //  int raw = analogRead(34);
  //  float L = raw / 4095.0;
  //  float gamma = 0.5;  // tweak for taste
  //  int brightness = int(pow(L, gamma) * 255);

  // Weekly reboot test and implementation.
  if (tm->tm_wday == 0 && tm->tm_hour == 2 && tm->tm_min == 2 &&
      tm->tm_sec == 2) {  // Sunday at 02:02:02? Reboot!
    Serial.println("Time to restart.");
    tft.fillScreen(SkyBlue);
    myBuildString = "Multicolor Segments";
    ofr.setCursor(dispWidth / 2, dispLine1);
    ofr.cprintf(myBuildString.c_str());
    ofr.setCursor(dispWidth / 2, dispLine3);
    ofr.cprintf("Rebooting in");
    ofr.setCursor(dispWidth / 2, dispLine5);
    ofr.cprintf("5 seconds.");
    delay(5000); ESP.restart();
  }

  if (Serial.available()) {
    handleUserReq();
    Serial.printf("Current display mode: %d\n", displayMode);
  }
//  startMillis = millis();
  switch (displayMode) {
    case DM_FULL_SHIMMER:
    case DM_RL_SHIMMER:
    case DM_WHOLE_DIGIT:
      UpdateDisplayAS();
      break;

    case DM_STATIC_DIGITS:
      // Serial.println("Case not implemented yet.");
      break;

    default:
      Serial.println("Invalid display mode.");
      break;
  }
//  Serial.printf("Time to update display: %lu\r\n",
//                millis() - startMillis);
}
