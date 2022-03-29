/*void printLineChime(const int x, const int y, const int row) {
    if(row < 0 || row > numFileTitles-1) return;
    display.setCursor(x, y);
    display.print(fileTitles[i]);
}

bool rowActionChime(const int row) {
    if(val < 0 || val > numFileTitles-1) return false;
      Serial.print("New chime is: ");
      Serial.println(fileTitles[val]);
      currentAlarm.ap.track = val;
      screen = alarm_setting_scr;
      stopTrack();
      return true;
}

void hoverChime(const int row) {
    playTrack(row+1);
}

void displayChimeList() {
    listLoop(numFileTitles, 0, &printLineChime, &rowActionChime, &hoverChime);
}

void listLoop(const int length, int top, void (*printLine)(int,int,int), bool (*clickAction)(int), void (*onHover)(int) == NULL) {

  int prevEncoderPostition = 0;
  const int numLines = 4;
  
  timeSinceLastAction = millis();
  rotaryEncoder.setBoundaries(0, length, false);
  rotaryEncoder.setEncoderValue(top);
  displayList(false, top, "Chime", printLine);
  displayMenuSelectionIndicator(0);

  while(true) {

    if(checkScreenTimeout()) return;
    manageLoop();

    if(rotaryEncoder.encoderChanged()) {
        timeSinceLastAction = millis();
        int val = rotaryEncoder.readEncoder();
        onHover(val);
        if(val == top && top != 0 && val < prevEncoderPostition) {
            prevEncoderPostition = val;
            // TODO: should increade by possibly more then once because the library (but make sure it doesn't overstep)
            top--;
            displayList(true, top, "Chime", printLine);
            displayMenuSelectionIndicator(0);
        } else
        if(val == top+4 && top != length && val > prevEncoderPostition) {
            prevEncoderPostition = val;
            top++;
            displayList(true, top, "Chime", printLine);
            displayMenuSelectionIndicator(3);
        } else {
            displayMenuSelectionIndicator(val - top);
        }
    }
    
    if(rotaryEncoder.isEncoderButtonClicked()) {
      if(clickAction(rotaryEncoder.readEncoder())) return;
    }

  }
}

void displayList(bool partial, int top, const char* title, void (*printLine)(int,int,int)) {

  if(partial) display.setPartialWindow(0, 0, display.width(), display.height());
  else display.setFullWindow();

  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    if(!partial) displayTitle(title);

    const int lineHeight = 22;
    const int horizontalOffset = 30;
    const int numLines = 4;

    int row = 0;
    for(int i = top; i < top+numLines; i++) {
      int verticalOffset = lineHeight*(row+2);
      printLine(horizontalOffset, verticalOffset, i);
      row++;
    }

  }
  while (display.nextPage());
}

void displayTitle(const char* title) {
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(title, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  display.fillRect(0, 0, display.width(), 24, GxEPD_BLACK);
  display.setCursor(x, 16);
  display.setTextColor(GxEPD_WHITE);
  display.print(title);
  display.setTextColor(GxEPD_BLACK);
}*/