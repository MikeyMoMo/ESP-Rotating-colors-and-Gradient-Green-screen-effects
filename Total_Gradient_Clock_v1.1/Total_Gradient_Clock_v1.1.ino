#include "Definitions.h"
/*****************************************************************************/
void setup()
/*****************************************************************************/
{
  struct tm timeinfo = {0};
  int ofrFontHeight;

  Serial.begin(115200); delay(2000);
  Serial.println("Running from:"); Serial.println(__FILE__);
  Serial.printf("Compiled on %s at %s\r\n", __DATE__, __TIME__);
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
  Serial.print("Connected on "); Serial.println(workDate);
  tft.fillScreen(SkyBlue);
  // Foreground color, Background color
  tft.setTextColor(TFT_BLACK, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf("WiFi");
  ofr.setCursor(dispWidth / 2, dispLine2); ofr.cprintf("connected on");
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf(workDate);
  ofr.setCursor(dispWidth / 2, dispLine4); ofr.cprintf(WiFi.SSID().c_str());
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
  configTzTime("PHT-8", "pool.ntp.org"); delay(2000);
  while (!getLocalTime(&timeinfo, 5000)) {
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
      Serial.println("Cannot get time set after a minute waiting. Rebooting.");
      ESP.restart();
    }
  }
  tft.fillScreen(SkyBlue);
  myBuildString = "Multicolor Segments";
  ofr.setCursor(dispWidth / 2, dispLine1); ofr.cprintf(myBuildString.c_str());
  ofr.setCursor(dispWidth / 2, dispLine3); ofr.cprintf("Initializing");
  ofr.setCursor(dispWidth / 2, dispLine4); ofr.cprintf("Sprites"); 

  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("rainbow1LineSprite");
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
  ofr.setFontSize(DATE_FONT_SIZE);
  TextMetrics tm = getTextMetrics(testDay, DATE_FONT_SIZE);
  spriteDateW = tm.width + 5;
  spriteDateH = tm.height;
  Serial.printf("Return from getTextWidth %dx%d\r\n",
                spriteDateW, spriteDateH);

  safeLoadOFR_Font("BritanicBoldTTF", BritanicBoldTTF,
                   sizeof(BritanicBoldTTF), __LINE__, __func__);
  // print FreeType version
  ofr.setFontSize(50);
  tft.fillRect(0, dispLine5, tft.width(), 80, SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine5); ofr.cprintf("spriteDate");
  delay(1000);
  a = (int*)spriteDate.createSprite(spriteDateW, spriteDateH );
  if (a == 0) {
    Serial.println("spriteDate creation failed.  Cannot continue.");
    while (1);
  }

  tft.fillScreen(SkyBlue);
  ofr.setCursor(dispWidth / 2, dispLine2); ofr.cprintf("Initialization");
  ofr.setCursor(dispWidth / 2, dispLine4); ofr.cprintf("Complete");
  delay(2000);

  tft.unloadFont();
  tft.loadFont(DSEG7_Classic_Bold140);  // must match the array name exactly

  tft.fillScreen(TFT_BLACK);
  ofr.setDrawer(spriteDate); // Link renderer sprite for date display.

  Serial.println(F("setup() done. To work, we go..."));
}
/*****************************************************************************/
void loop()
/*****************************************************************************/
{
  time(&workTime_t);
  tm = localtime(&workTime_t);
  //  startMillis = millis();
//  tft.unloadFont();
//  tft.loadFont(DSEG7_Classic_Bold140);   // must match the array name exactly
  UpdateDisplay();
  //  Serial.printf("Time to fully update display: %d ms.\r\n",
  //                millis() - startMillis);
}
