#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// ================= TFT CONFIG =================
#define TFT_CS   15
#define TFT_RST  17
#define TFT_DC   16
#define TFT_MOSI 7
#define TFT_SCLK 6

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ================= KEYPAD CONFIG =================
const int ROWS = 7;
const int COLS = 3;
const int rowPins[ROWS] = {0, 1, 8, 10, 11, 12, 13}; 
const int colPins[COLS] = {3, 4, 5}; 

const unsigned long DEBOUNCE_MS = 20;

const char* keyLabel[ROWS][COLS] = {
  {"Enter_key",     "Up_key",       "Backspace_key"},
  {"Left_key",      "Ok_key",       "Right_key"   },
  {"Send_key",      "Down_key",     "Clear_key"   },
  {"1",             "2",            "3"           },
  {"4",             "5",            "6"           },
  {"7",             "8",            "9"           },
  {"#",             "0",            "."           }
};

bool pressed[ROWS][COLS];
bool lastReading[ROWS][COLS];
unsigned long lastChangeMs[ROWS][COLS];

// ================= STATE MACHINE =================
enum UIState { HOME, INPUT_DATA, SENDING };
UIState currentState = HOME;

// ================= INPUT HANDLING =================
String inputBuffer = "";
int cursorX = 0;
int cursorY = 0;

// ================= HELPER FUNCTIONS =================
void showLogo() {
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5, 20);
  tft.println("Welcome to");
  tft.setCursor(5, 45);
  tft.println("Wycript Lab");
  delay(4000);

  tft.fillScreen(ST77XX_WHITE);
  tft.fillCircle(64, 100, 20, ST77XX_BLUE);
  tft.fillCircle(64, 100, 5, ST77XX_WHITE);
  tft.drawCircle(64, 60, 10, ST77XX_YELLOW);
  tft.drawCircle(64, 60, 15, ST77XX_YELLOW);
  tft.drawCircle(64, 60, 20, ST77XX_YELLOW);
}

void showLogo2() {
  tft.fillScreen(ST77XX_WHITE);
  tft.fillCircle(64, 100, 20, ST77XX_BLUE);
  tft.fillCircle(64, 100, 5, ST77XX_WHITE);
  tft.drawCircle(64, 60, 10, ST77XX_YELLOW);
  tft.drawCircle(64, 60, 15, ST77XX_YELLOW);
  tft.drawCircle(64, 60, 20, ST77XX_YELLOW);
}

void showHome() {
  currentState = HOME;
  showLogo();
}

void showHome2() {
  currentState = HOME;
  showLogo2();
}

void showInputData() {
  currentState = INPUT_DATA;
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 10);
  tft.println("Input Mode:");
  tft.setCursor(5, 40);
  tft.print(inputBuffer);
}

void showSending() {
  currentState = SENDING;
  tft.fillScreen(ST77XX_GREEN);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(30, 80);
  tft.println("Sending...");
  delay(2000);
  showHome();
}

void handleKeyPress(const char* key) {
  if (currentState == HOME) {
    // Only allow data keys in HOME state
    if (!(String(key).endsWith("_key"))) {
      inputBuffer = key;
      showInputData();
    }
  } else if (currentState == INPUT_DATA) {
    if (String(key) == "Left_key") {
      if (cursorX > 0) cursorX--;
    } else if (String(key) == "Right_key") {
      if (cursorX < inputBuffer.length()) cursorX++;
    } else if (String(key) == "Up_key") {
      if (cursorY > 0) cursorY--;
    } else if (String(key) == "Down_key") {
      cursorY++;
    } else if (String(key) == "Clear_key") {
      inputBuffer = "";
      showHome2();
      return;
    } else if (String(key) == "Backspace_key") {
      if (inputBuffer.length() <= 1) {
        inputBuffer = "";
        showHome2();
        return;
      } else {
        inputBuffer.remove(inputBuffer.length() - 1);
      }
    } else if (String(key) == "Send_key") {
      showSending();
      return;
    } else if (!(String(key).endsWith("_key"))) { // only data keys add text
      inputBuffer += key;
    }
    showInputData();
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // TFT setup
  tft.initR(INITR_144GREENTAB);
  tft.setSPISpeed(27000000); // boost SPI speed for faster refresh
  showLogo();

  // Setup keypad
  for (int r = 0; r < ROWS; r++) pinMode(rowPins[r], INPUT_PULLUP);
  for (int c = 0; c < COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], HIGH);
  }

  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      pressed[r][c] = false;
      lastReading[r][c] = false;
      lastChangeMs[r][c] = 0;
    }
  }
}

// ================= LOOP =================
void loop() {
  for (int c = 0; c < COLS; c++) {
    digitalWrite(colPins[c], LOW);

    for (int r = 0; r < ROWS; r++) {
      int val = digitalRead(rowPins[r]);
      bool isPressedNow = (val == LOW);

      if (isPressedNow != lastReading[r][c]) {
        lastReading[r][c] = isPressedNow;
        lastChangeMs[r][c] = millis();
      } else {
        if ((millis() - lastChangeMs[r][c]) >= DEBOUNCE_MS) {
          if (pressed[r][c] != isPressedNow) {
            pressed[r][c] = isPressedNow;
            if (pressed[r][c]) {
              handleKeyPress(keyLabel[r][c]);
            }
          }
        }
      }
    }

    digitalWrite(colPins[c], HIGH);
  }
}
