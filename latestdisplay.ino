// This Arduino sketch for ESP32-C6 demonstrates a flicker-free UI for a matrix keypad
// on a ST7735 TFT display using double buffering.

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// ================= TFT CONFIG =================
// The display is a 1.44" ST7735
#define TFT_CS    10
#define TFT_RST   8
#define TFT_DC    9
#define TFT_MOSI  11
#define TFT_SCLK  12

// The TFT library object, using the hardware SPI pins and screen size
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// The size of your display in pixels
#define TFT_WIDTH  tft.width()
#define TFT_HEIGHT tft.height()

// Create a graphics buffer in RAM for double buffering.
// A GFXcanvas16 creates a 16-bit color buffer to match the ST7735.
GFXcanvas16 canvas(TFT_WIDTH, TFT_HEIGHT);

// ================= KEYPAD CONFIG =================
const int ROWS = 7;
const int COLS = 3;
const int rowPins[ROWS] = {0, 1, 8, 10, 11, 12, 13};
const int colPins[COLS] = {3, 4, 5};

const unsigned long DEBOUNCE_MS = 20;

const char* keyLabel[ROWS][COLS] = {
  {"Enter_key",     "Up_key",        "Backspace_key"},
  {"Left_key",      "Ok_key",        "Right_key"    },
  {"Send_key",      "Down_key",      "Clear_key"    },
  {"1",             "2",             "3"            },
  {"4",             "5",             "6"            },
  {"7",             "8",             "9"            },
  {"#",             "0",             "."            }
};

bool pressed[ROWS][COLS];
bool lastReading[ROWS][COLS];
unsigned long lastChangeMs[ROWS][COLS];

// ================= STATE MACHINE =================
enum UIState { HOME, INPUT_DATA, SENDING, LOGO_PAGE };
UIState currentState = LOGO_PAGE; // Start in the logo state
unsigned long stateChangeTime = 0;

// ================= INPUT HANDLING =================
String inputBuffer = "";

// ================= HELPER FUNCTIONS =================

// Draws the contents of the buffer to the screen.
void updateScreen() {
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), TFT_WIDTH, TFT_HEIGHT);
}

// Draws the initial logo screen to the buffer.
void showLogo() {
  canvas.fillScreen(ST7735_WHITE);
  canvas.setTextColor(ST7735_BLACK);
  canvas.setTextSize(2);
  canvas.setCursor(5, 20);
  canvas.println("Welcome to");
  canvas.setCursor(5, 45);
  canvas.println("Wycript Lab");
}

// Draws the main input screen to the buffer.
void showInputData() {
  canvas.fillScreen(ST7735_BLACK);
  canvas.setTextColor(ST7735_WHITE);
  canvas.setTextSize(1);
  canvas.setCursor(5, 10);
  canvas.println("Input Mode:");
  canvas.setCursor(5, 40);
  canvas.print(inputBuffer);
}

// Draws the "Sending..." screen to the buffer.
void showSending() {
  canvas.fillScreen(ST7735_GREEN);
  canvas.setTextColor(ST7735_BLACK);
  canvas.setTextSize(2);
  canvas.setCursor(20, 50);
  canvas.println("Sending...");
  canvas.setTextSize(1);
  canvas.setCursor(5, 80);
  canvas.println(inputBuffer);
}

void handleKeyPress(const char* key) {
  // We only update the state and the buffer here, not the screen
  if (currentState == LOGO_PAGE) {
    currentState = HOME;
  }
  
  if (currentState == HOME) {
    if (String(key) != "Enter_key") { // Prevent immediate input buffer clearing with Enter key
      if (!(String(key).endsWith("_key"))) {
        inputBuffer = key;
        currentState = INPUT_DATA;
        showInputData();
        updateScreen();
      }
    }
  } else if (currentState == INPUT_DATA) {
    if (String(key) == "Clear_key") {
      inputBuffer = "";
      currentState = HOME;
      showLogo();
      updateScreen();
      return;
    } else if (String(key) == "Backspace_key") {
      if (inputBuffer.length() > 0) {
        inputBuffer.remove(inputBuffer.length() - 1);
      }
      showInputData();
      updateScreen();
    } else if (String(key) == "Send_key") {
      currentState = SENDING;
      stateChangeTime = millis();
      showSending();
      updateScreen();
    } else if (!(String(key).endsWith("_key"))) {
      inputBuffer += key;
      showInputData();
      updateScreen();
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // TFT setup
  tft.initR(INITR_144GREENTAB);
  tft.setSPISpeed(27000000); // boost SPI speed
  tft.setRotation(1); // Set landscape mode
  
  // Show the initial logo screen
  showLogo();
  updateScreen();

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
  // Handle state transitions without blocking
  if (currentState == LOGO_PAGE && millis() - stateChangeTime > 4000) {
    currentState = HOME;
    showLogo();
    updateScreen();
  }
  if (currentState == SENDING && millis() - stateChangeTime > 2000) {
    currentState = HOME;
    showLogo();
    updateScreen();
    inputBuffer = ""; // Clear buffer after sending
  }

  // Keypad scanning loop
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
