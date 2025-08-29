// ====== Pin lists ======
const int ROWS = 7;                                 // Number of row lines
const int COLS = 3;                                 // Number of column lines

// Pin assignments for ESP32-C6 (safe pins)
const int rowPins[ROWS] = {6, 7, 10, 11, 21, 22, 23};   // Row GPIOs (inputs with pull-ups)
const int colPins[COLS] = {5, 20, 3};                   // Column GPIOs (outputs)

// Debounce settings
const unsigned long DEBOUNCE_MS = 20;                // Time a state must be stable

// Key type map (Command vs Data)
const char* keyType[ROWS][COLS] = {
  {"Command", "Command", "Command"},
  {"Command", "Command", "Command"},
  {"Command", "Command", "Command"},
  {"Data",    "Data",    "Data"},
  {"Data",    "Data",    "Data"},
  {"Data",    "Data",    "Data"},
  {"Data",    "Data",    "Data"}
};

// Key label map
const char* keyLabel[ROWS][COLS] = {
  {"Enter_key",     "Up_key",       "Backspace_key"},
  {"Left_key",      "Ok_key",       "Right_key"   },
  {"Send_key",      "Down_key",     "Clear_key"   },
  {"1",             "2",            "3"           },
  {"4",             "5",            "6"           },
  {"7",             "8",            "9"           },
  {"#",             "0",            "."           }
};

// State storage for debouncing
bool pressed[ROWS][COLS];
bool lastReading[ROWS][COLS];
unsigned long lastChangeMs[ROWS][COLS];

void setup() {
  Serial.begin(115200);
  
  // Setup rows as inputs with pull-ups
  for (int r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }
  
  // Setup columns as outputs, idle HIGH
  for (int c = 0; c < COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], HIGH);
  }

  // Initialize debounce arrays
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      pressed[r][c] = false;
      lastReading[r][c] = false;
      lastChangeMs[r][c] = 0;
    }
  }

  Serial.println("3x7 Matrix keyboard ready.");
}

void loop() {
  for (int c = 0; c < COLS; c++) {
    digitalWrite(colPins[c], LOW);     // Activate column
    delayMicroseconds(5);

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
              // Print only when key is pressed
              Serial.print(keyType[r][c]);
              Serial.print(" ");
              Serial.print(keyLabel[r][c]);
              Serial.println(" press");
            }
          }
        }
      }
    }

    digitalWrite(colPins[c], HIGH);   // Deactivate column
  }
}
