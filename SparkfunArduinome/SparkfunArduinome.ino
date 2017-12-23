#define NUM_LED_COLUMNS (8)
#define NUM_LED_ROWS    (8)
#define NUM_BTN_COLUMNS (8)
#define NUM_BTN_ROWS    (8)
#define NUM_KEYS        (64)
#define MAX_DEBOUNCE    (3)

static bool   ledBuffer[NUM_LED_COLUMNS][NUM_LED_ROWS];
static int8_t debounceCount[NUM_BTN_COLUMNS][NUM_BTN_ROWS];

static const uint8_t BTN_COLUMN_PIN[NUM_BTN_COLUMNS] = {53,51,49,47,45,43,41,39};
static const uint8_t BTN_ROW_PIN[NUM_BTN_ROWS]       = {37,35,33,31,29,27,25,23};
static const uint8_t LED_COLUMN_PIN[NUM_LED_COLUMNS] = {52,50,48,46,44,42,40,38};
static const uint8_t LED_ROW_PIN[NUM_LED_ROWS]       = {36,34,32,30,28,26,24,22};

const uint8_t gridNumber = 0x01;
const uint8_t gridX      = 8;
const uint8_t gridY      = 8;
const uint8_t numGrids   = 16;
uint8_t       current    = 0;

// FIXME: deviceID is broken somehow
String deviceID  = "monome                          ";  // 32
String serialNum = "m1000009"; // 8

static void setupPins() {
  uint8_t i;

  for (i = 0; i < NUM_LED_COLUMNS; i++) {
    pinMode(LED_COLUMN_PIN[i], OUTPUT);

    // with nothing selected by default
    digitalWrite(LED_COLUMN_PIN[i], HIGH);
  }

  // button columns
  for (i = 0; i < NUM_BTN_COLUMNS; i++) {
    pinMode(BTN_COLUMN_PIN[i], OUTPUT);

    // with nothing selected by default
    digitalWrite(BTN_COLUMN_PIN[i], HIGH);
  }

  // button row input lines
  for (i = 0; i < NUM_BTN_ROWS; i++) {
    pinMode(BTN_ROW_PIN[i], INPUT_PULLUP);
  }

  // LED drive lines
  for (i = 0; i < NUM_LED_ROWS; i++) {
    pinMode(LED_ROW_PIN[i], OUTPUT);
    digitalWrite(LED_ROW_PIN[i], LOW);
  }

  // Initialize the debounce counter array
  for (uint8_t i = 0; i < NUM_BTN_COLUMNS; i++) {
    for (uint8_t j = 0; j < NUM_BTN_ROWS; j++) {
      debounceCount[i][j] = 0;
    }
  }
}

void setLED(int x, int y, int v) {
  if (x >= 0 && x < 8 && y >= 0 && y < 8) {
    ledBuffer[x][y] = v;
  }
}

void setAllLEDs(int value) {
  for (uint8_t i = 0; i < NUM_LED_COLUMNS; i++) {
    for (uint8_t j = 0; j < NUM_LED_ROWS; j++) {
      ledBuffer[i][j] = value;
    }
  }
}

void clearLEDs() {
  setAllLEDs(0);
}

void turnOnLEDs() {
  setAllLEDs(1);
}

void setup() {
  Serial.begin(115200);

  setupPins();

  clearLEDs();
}

uint8_t readInt() {
  return Serial.read();
}

void writeInt(uint8_t value) {
  Serial.write(value);
}

void serialEvent() {
  do {
    processSerial();
  } while (Serial.available() > 16);
}

void processSerial() {
  uint8_t identifierSent;  // Command byte sent from controller to matrix
  uint8_t deviceAddress;   // device address sent from controller
  uint8_t dummy;           // for reading in data not used by the matrix
  uint8_t intensity = 255; // LED intensity.  Since UNTZ is on or off, 0=off, >0=on, initially on
  uint8_t readX, readY;    // X and Y values read from driver
  uint8_t i;               // loop variable
  uint8_t index;           // Linear index of the LED being worked with (0-63 or 0-127)

  // get command identifier: first byte of packet is identifier in the form: [(a << 4) + b]
  // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
  // b = command (ie. query, enable, led, key, frame)
  identifierSent = Serial.read();

  switch (identifierSent) {
    // 0x00-0x0f are system commands

    case 0x00:
      writeInt((uint8_t)0x00); // system/query response 0x00 -> 0x00
      writeInt((uint8_t)0x01); // grids
      writeInt((uint8_t)0x01); // one grid
      break;

    case 0x01:
      writeInt((uint8_t)0x01);
      Serial.print(deviceID);
      break;

    case 0x02:
      for(i=0; i<32; i++) {
        deviceID[i] = Serial.read();
      }
      break;

    case 0x03:
      writeInt((uint8_t)0x02);  // system / request grid offsets
      // writeInt(0);              // n grid
      writeInt((uint8_t)0x00);  // x offset
      writeInt((uint8_t)0x00);  // y offset
      break;

    case 0x04:
      dummy = readInt();        // system / set grid offset
      readX = readInt();        // An offset of 8 is valid only for HellaUNTZ
      readY = readInt();        // An offset is invalid for Y as it's only 8
      break;

    case 0x05:
      writeInt((uint8_t)0x03);   // system / request grid size  /sys/size
      writeInt((uint8_t)gridX);
      writeInt((uint8_t)gridY);
      break;

    case 0x06:
      readX = readInt();         // system / set grid size
      readY = readInt();         // currently this does nothing.  Get more info?
      break;

    case 0x07:
      break;                     // system / get ADDR (scan) (we don't send I2C via serial) "h"

    case 0x08:
      deviceAddress = readInt(); // system / set ADDR (scan)
      dummy = readInt();         // address value
      break;

    case 0x0F:
      writeInt((uint8_t)0x0F);   // Send Serial Number
      Serial.print(serialNum);   // monome SN m####### or m64-#### m128-### m256-####
      break;

    // 0x10-0x1F are LED stuff

    case 0x10:
      readX = readInt();
      readY = readInt();
      ledBuffer[readX][readY] = 0;
      break;

    case 0x11:
      readX = readInt();
      readY = readInt();
      ledBuffer[readX][readY] = 1;
      break;

    case 0x12:
      clearLEDs();
      break;

    case 0x13:
      turnOnLEDs();
      break;

    case 0x14:
      readX = readInt();
      readY = readInt();
      readX >> 3; readX << 3; // floor to multiple of 8
      readY >> 3; readY << 3;
      if(readX == 8 && NUM_KEYS > 64) break; // trying to set an 8x16 grid on a pad with only 64 keys
      if(readY != 0) break;               // since we only have 8 LEDs in a column, no offset
      for (uint8_t y = 0; y < 8; y++) {   // each i will be a row
        intensity = readInt();            // read one byte of 8 bits on/off
        for (int8_t x = 0; x < 8; x++) {  // for 8 LEDs on a row
          if ((intensity>>x) & 0x01) {    // set LED if the intensity bit is set
            setLED(readX + x, y, 1);
          }
          else {
            setLED(readX + x, y, 0);
          }
        }
      }
      break;

    case 0x15:
      readX = readInt();              // led-grid / set row (debug "v")
      readX >> 3; readX << 3;
      readY = readInt();              // may be any value
      intensity = readInt();          // read one byte of 8 bits on/off

      for (i = 0; i < 8; i++) {        // for the next 8 lights in row
        if ((intensity >> i) & 0x01) { // if intensity bit set, light
          setLED(readX + i, readY, 1);
        }
        else {
          setLED(readX + i, readY, 0);
        }
      }
      break;

    case 0x16:
      readX = readInt();                 // led-grid / column set (debug "w")
      readY = readInt();
      readY >> 3 ; readY << 3;           // floor to multiple of 8
      intensity = readInt();             // read one byte of 8 bits on/off
      if (readY != 0) break;             // we only have 8 lights in a column
      for (i = 0; i < 8; i++) {          // for the next 8 lights in column
        if ((intensity>>i) & 0x01) {     // if intensity bit set, light
          setLED(readX, i, 1);
        }
        else {
          setLED(readX, i, 0);
        }
      }
      break;

    case 0x17:
      intensity = readInt();
      // intensity stuff - ignored
      break;

    case 0x18:
      readX = readInt();                   // led-grid / set LED intensity (debug "y")
      readY = readInt();                   // read the x and y coordinates
      intensity = readInt();               // read the intensity value (0-255, 0x00-0xFF)
      if (intensity) {                     // if intensity > 0
        setLED(readX, readY, 1);
      }
      else {
        setLED(readX, readY, 0);
      }
      break;

    case 0x19: // set all leds
      intensity = readInt();
      if (intensity > 0) {
        turnOnLEDs();
      }
      else {
        clearLEDs();
      }

    case 0x1A: // set 8x8 block
      readX = readInt();
      readX << 3; readX >> 3;
      readY = readInt();
      readY << 3; readY >> 3;
      for (uint8_t y=0; y<8; y++) {
        for (uint8_t x=0; x<8; x++) {
          if (index > NUM_KEYS) break;

          if ((x + y) % 2 == 0) {       // even bytes, read intensity, use upper nybble
            intensity = readInt();

            if (intensity >> 4 & 0x0F)  {
              setLED(readX + x, y, 1);
            }
            else {
              setLED(readX + x, y, 0);
            }
          }
          else {                       // odd bytes, use lower nybble
            if (intensity & 0x0F) {
              setLED(readX + x, y, 1);
            }
            else {
              setLED(readX + x, y, 0);
            }
          }
        }
      }
      break;

    case 0x1B:  // set 8x1 row of intensities
      readX = readInt();
      readX << 3; readX >> 3;
      readY = readInt();
      for (uint8_t x = 0; x < 8; x++) {
        intensity = readInt();

        if (intensity) {
          setLED(readX + x, readY, 1);
        }
      }
      break;

    case 0x1C:  // set 1x8 column by intensity; we can't, so any nonzero one is displayed
      readX = readInt();
      readY = readInt();
      readY << 3; readY >> 3;
      for (uint8_t y = 0; y < 8; y++) {
        intensity = readInt();

        if (intensity) {
          setLED(readX, readY + y, 1);
        }
      }
      break;

    default:
      break;
  }

  return;
}

static void scanMatrix(int current) {
  uint8_t val;
  uint8_t i, j;

  // select current columns
  digitalWrite(BTN_COLUMN_PIN[current], LOW);
  digitalWrite(LED_COLUMN_PIN[current], LOW);

  // output LED row values
  for (i = 0; i < NUM_LED_ROWS; i++) {
    if (ledBuffer[current][i] > 0) {
      digitalWrite(LED_ROW_PIN[i], HIGH);
    }
  }

  delay(1);

  // read the button inputs
   for (j = 0; j < NUM_BTN_ROWS; j++) {
     val = digitalRead(BTN_ROW_PIN[j]);

     if (val == LOW) {
       // button pressed
       if (debounceCount[current][j] < MAX_DEBOUNCE) {
         debounceCount[current][j]++;

         if (debounceCount[current][j] == MAX_DEBOUNCE) {
           Serial.write(0x21);
           Serial.write(current);
           Serial.write(j);
         }
       }
     }
     else {
       // button released
       if (debounceCount[current][j] > 0) {
         debounceCount[current][j]--;

         if (debounceCount[current][j] == 0) {
           Serial.write(0x20);
           Serial.write(current);
           Serial.write(j);
         }
       }
     }
   }

  delay(1);

  digitalWrite(BTN_COLUMN_PIN[current], HIGH);
  digitalWrite(LED_COLUMN_PIN[current], HIGH);

  for (i = 0; i < NUM_LED_ROWS; i++) {
    digitalWrite(LED_ROW_PIN[i], LOW);
  }
}

void loop() {
  scanMatrix(current);

  current++;
  if (current >= NUM_LED_COLUMNS) {
    current = 0;
  }
}
