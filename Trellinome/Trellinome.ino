/*------------------------------------------------------------------------
  Based on code for  Adafruit UNTZtrument - a Trellis button controller.
  https://github.com/adafruit/Adafruit_UNTZtrument
  
  ------------------------------------------------------------------------*/
  
#include "Adafruit_Trellis.h"

#define NUM_TRELLIS (4)
#define NUM_KEYS    (NUM_TRELLIS * 16)
#define NUM_COLS    (NUM_TRELLIS * 2)
#define NUM_ROWS    (NUM_TRELLIS * 2)

Adafruit_Trellis matrixes[] = {
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis()
};

Adafruit_TrellisSet trellis = Adafruit_TrellisSet(&matrixes[0], &matrixes[1], &matrixes[2], &matrixes[3],
													&matrixes[4], &matrixes[5], &matrixes[6], &matrixes[7]);

String deviceID  = "monome";
String serialNum = "m1000000";

unsigned long prevReadTime  = 0;
unsigned long prevWriteTime = 0;

static bool ledBuffer[8][8];

// these functions are from Adafruit_UNTZtrument.h
static const uint8_t PROGMEM
	i2xy64[] = { // Remap 8x8 TrellisSet button index to column/row
		0x00, 0x10, 0x20, 0x30, 0x01, 0x11, 0x21, 0x31,
		0x02, 0x12, 0x22, 0x32, 0x03, 0x13, 0x23, 0x33,
		0x40, 0x50, 0x60, 0x70, 0x41, 0x51, 0x61, 0x71,
		0x42, 0x52, 0x62, 0x72, 0x43, 0x53, 0x63, 0x73,
		0x04, 0x14, 0x24, 0x34, 0x05, 0x15, 0x25, 0x35,
		0x06, 0x16, 0x26, 0x36, 0x07, 0x17, 0x27, 0x37,
		0x44, 0x54, 0x64, 0x74, 0x45, 0x55, 0x65, 0x75,
		0x46, 0x56, 0x66, 0x76, 0x47, 0x57, 0x67, 0x77 },
	i2xy128[] = { // Remap 16x8 TrellisSet button index to column/row
		0x00, 0x10, 0x20, 0x30, 0x01, 0x11, 0x21, 0x31,
		0x02, 0x12, 0x22, 0x32, 0x03, 0x13, 0x23, 0x33,
		0x40, 0x50, 0x60, 0x70, 0x41, 0x51, 0x61, 0x71,
		0x42, 0x52, 0x62, 0x72, 0x43, 0x53, 0x63, 0x73,
		0x80, 0x90, 0xA0, 0xB0, 0x81, 0x91, 0xA1, 0xB1,
		0x82, 0x92, 0xA2, 0xB2, 0x83, 0x93, 0xA3, 0xB3,
		0xC0, 0xD0, 0xE0, 0xF0, 0xC1, 0xD1, 0xE1, 0xF1,
		0xC2, 0xD2, 0xE2, 0xF2, 0xC3, 0xD3, 0xE3, 0xF3,
		0x04, 0x14, 0x24, 0x34, 0x05, 0x15, 0x25, 0x35,
		0x06, 0x16, 0x26, 0x36, 0x07, 0x17, 0x27, 0x37,
		0x44, 0x54, 0x64, 0x74, 0x45, 0x55, 0x65, 0x75,
		0x46, 0x56, 0x66, 0x76, 0x47, 0x57, 0x67, 0x77,
		0x84, 0x94, 0xA4, 0xB4, 0x85, 0x95, 0xA5, 0xB5,
		0x86, 0x96, 0xA6, 0xB6, 0x87, 0x97, 0xA7, 0xB7,
		0xC4, 0xD4, 0xE4, 0xF4, 0xC5, 0xD5, 0xE5, 0xF5,
		0xC6, 0xD6, 0xE6, 0xF6, 0xC7, 0xD7, 0xE7, 0xF7 },
	xy2i64[8][8] = { // Remap [row][col] to Trellis button/LED index
		{  0,  1,  2,  3, 16, 17, 18, 19 },
		{  4,  5,  6,  7, 20, 21, 22, 23 },
		{  8,  9, 10, 11, 24, 25, 26, 27 },
		{ 12, 13, 14, 15, 28, 29, 30, 31 },
		{ 32, 33, 34, 35, 48, 49, 50, 51 },
		{ 36, 37, 38, 39, 52, 53, 54, 55 },
		{ 40, 41, 42, 43, 56, 57, 58, 59 },
		{ 44, 45, 46, 47, 60, 61, 62, 63 }},
	xy2i128[8][16] = {
		{  0,  1,  2,  3, 16, 17, 18, 19, 32, 33, 34, 35, 48, 49, 50, 51 },
		{  4,  5,  6,  7, 20, 21, 22, 23, 36, 37, 38, 39, 52, 53, 54, 55 },
		{  8,  9, 10, 11, 24, 25, 26, 27, 40, 41, 42, 43, 56, 57, 58, 59 },
		{ 12, 13, 14, 15, 28, 29, 30, 31, 44, 45, 46, 47, 60, 61, 62, 63 },
		{ 64, 65, 66, 67, 80, 81, 82, 83, 96, 97, 98, 99,112,113,114,115 },
		{ 68, 69, 70, 71, 84, 85, 86, 87,100,101,102,103,116,117,118,119 },
		{ 72, 73, 74, 75, 88, 89, 90, 91,104,105,106,107,120,121,122,123 },
		{ 76, 77, 78, 79, 92, 93, 94, 95,108,109,110,111,124,125,126,127 } 
};  


uint8_t xy2i(uint8_t x, uint8_t y) {
	if (x > 7 || y > 7) {
		return 255;
	}
	if(NUM_KEYS == 64) {
		if(x > 7) return 255;
		return pgm_read_byte(&xy2i64[y][x]);
	} else {
		if(x > 15) return 255;
		return pgm_read_byte(&xy2i128[y][x]);
	}

//  return pgm_read_byte(&xy2i64[y][x]);
}

void i2xy(uint8_t i, uint8_t *x, uint8_t *y) {
	if (i > NUM_KEYS) {
		*x = *y = 255;
		return;
	}
	// uint8_t xy = pgm_read_byte(&i2xy64[i]);
	uint8_t xy = pgm_read_byte((NUM_KEYS == 64) ? &i2xy64[i] : &i2xy128[i]);
	*x = xy >> 4;
	*y = xy & 15;
}

// --------

void setLED(int x, int y, int v) {
  if (x >= 0 && x < 8 && y >= 0 && y < 8) {
    ledBuffer[x][y] = v;
  }
}

void setAllLEDs(int value) {
  uint8_t i, j;

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      ledBuffer[i][j] = value;
    }
  }
}

void turnOffLEDs() {
  setAllLEDs(0);
}

void turnOnLEDs() {
  setAllLEDs(1);
}

void setup() {
	Serial.begin(115200);
	// adjust this if x/y makes no sense on your grid
	
	if (NUM_KEYS == 64 ){
		// 2x2 arrangement - addr is the I2C address of the upper left, upper right, lower left and lower right matrices, respectively
		trellis.begin(0x71, 0x70, 0x73, 0x72);
	} else if (NUM_KEYS == 128 ){
		// 4x2 arrangement - 128
		trellis.begin(0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77);
	}

	setAllLEDs(0);
}

uint8_t readInt() {
  return Serial.read();
}

void writeInt(uint8_t value) {
  Serial.write(value);
}

void processSerial() {
  uint8_t identifierSent;                     // command byte sent from controller to matrix
  uint8_t deviceAddress;                      // device address sent from controller
  uint8_t dummy;                              // for reading in data not used by the matrix
  uint8_t intensity = 255;                    // led intensity, ignored
  uint8_t readX, readY;                       // x and y values read from driver
  uint8_t i, x, y;

  identifierSent = Serial.read();             // get command identifier: first byte of packet is identifier in the form: [(a << 4) + b]
                                              // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
                                              // b = command (ie. query, enable, led, key, frame)

  switch (identifierSent) {
    case 0x00:
      writeInt((uint8_t)0x00);                // system/query response 0x00 -> 0x00
      writeInt((uint8_t)0x01);                // grids
      writeInt((uint8_t)0x01);                // one grid
      break;

    case 0x01:
      writeInt((uint8_t)0x01);
      for (i = 0; i < 32; i++) {              // has to be 32
        if (i < deviceID.length()) {
          Serial.print(deviceID[i]);
        }
        else {
          Serial.print('\0');
        }
      }
      break;

    case 0x02:
      for (i = 0; i < 32; i++) {
        deviceID[i] = Serial.read();
      }
      break;

    case 0x03:
      writeInt((uint8_t)0x02);                // system / request grid offsets
      // writeInt(0);                         // n grid?
      writeInt((uint8_t)0x00);                // x offset
      writeInt((uint8_t)0x00);                // y offset
      break;

    case 0x04:
      dummy = readInt();                      // system / set grid offset
      readX = readInt();                      // an offset of 8 is valid only for 16 x 8 monome
      readY = readInt();                      // an offset is invalid for y as it's only 8
      break;

    case 0x05:
      writeInt((uint8_t)0x03);                // system / request grid size
      writeInt((uint8_t)8);
      writeInt((uint8_t)8);
      break;

    case 0x06:
      readX = readInt();                      // system / set grid size - ignored
      readY = readInt();
      break;

    case 0x07:
      break;                                  // I2C stuff - ignored

    case 0x08:
      deviceAddress = readInt();              // set addr - ignored
      dummy = readInt();
      break;

    case 0x0F:
      writeInt((uint8_t)0x0F);                // send serial number
      Serial.print(serialNum);
      break;

    case 0x10:
      readX = readInt();
      readY = readInt();
      setLED(readX, readY, 0);
      break;

    case 0x11:
      readX = readInt();
      readY = readInt();
      setLED(readX, readY, 1);
      break;

    case 0x12:
      turnOffLEDs();
      break;

    case 0x13:
      turnOnLEDs();
      break;

    case 0x14:
      readX = readInt();
      readY = readInt();

      if (readY != 0) break;                  // since we only have 8 LEDs in a column, no offset

      for (y = 0; y < NUM_COLS; y++) {               // each i will be a row
        intensity = readInt();                // read one byte of 8 bits on/off

        for (x = 0; x < NUM_ROWS; x++) {             // for 8 LEDs on a row
          if ((intensity >> x) & 0x01) {      // set LED if the intensity bit is set
            setLED(readX + x, y, 1);
          }
          else {
            setLED(readX + x, y, 0);
          }
        }
      }
      break;

    case 0x15:
      readX = readInt();                      // led-grid / set row
      readY = readInt();                      // may be any value
      intensity = readInt();                  // read one byte of 8 bits on/off

      for (i = 0; i < 8; i++) {               // for the next 8 lights in row
        if ((intensity >> i) & 0x01) {        // if intensity bit set, light
          setLED(readX + i, readY, 1);
        }
        else {
          setLED(readX + i, readY, 0);
        }
      }
      break;

    case 0x16:
      readX = readInt();                      // led-grid / column set
      readY = readInt();
      intensity = readInt();                  // read one byte of 8 bits on/off

      if (readY != 0) break;                  // we only have 8 lights in a column

      for (i = 0; i < NUM_COLS; i++) {               // for the next 8 lights in column
        if ((intensity >> i) & 0x01) {        // if intensity bit set, light
          setLED(readX, i, 1);
        }
        else {
          setLED(readX, i, 0);
        }
      }
      break;

    case 0x17:
      intensity = readInt();                  // intensity stuff - ignored
      break;

    case 0x18:
      readX = readInt();                      // led-grid / set LED intensity
      readY = readInt();                      // read the x and y coordinates
      intensity = readInt();                  // read the intensity value (0-255, 0x00-0xFF)

      if (intensity > 0) {
        setLED(readX, readY, 1);
      }
      else {
        setLED(readX, readY, 0);
      }
      break;

    case 0x19:                                // set all leds
      intensity = readInt();

      if (intensity > 0) {
        turnOnLEDs();
      }
      else {
        turnOffLEDs();
      }

    case 0x1A:                                // set 8x8 block
      readX = readInt();
      readY = readInt();

      for (y = 0; y < NUM_COLS; y++) {
        for (x = 0; x < NUM_ROWS; x++) {
          if ((x + y) % 2 == 0) {             // even bytes, use upper nybble
            intensity = readInt();

            if (intensity >> 4 & 0x0F)  {
              setLED(readX + x, y, 1);
            }
            else {
              setLED(readX + x, y, 0);
            }
          }
          else {                              // odd bytes, use lower nybble
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

    case 0x1B:                                // set 8x1 row by intensity
      readX = readInt();
      readY = readInt();

      for (x = 0; x < NUM_ROWS; x++) {
        intensity = readInt();

        if (intensity) {
          setLED(readX + x, readY, 1);
        }
        else {
          setLED(readX + x, readY, 0);
        }
      }
      break;

    case 0x1C:                                // set 1x8 column by intensity
      readX = readInt();
      readY = readInt();

      for (y = 0; y < NUM_COLS; y++) {
        intensity = readInt();

        if (intensity) {
          setLED(readX, readY + y, 1);
        }
        else {
          setLED(readX, readY + y, 0);
        }
      }
      break;

    default:
      break;
  }

  return;
}

void readKeys() {
  uint8_t x, y;

  for (uint8_t i = 0; i < NUM_KEYS; i++) {
    if (trellis.justPressed(i)) {
      i2xy(i, &x, &y);

      writeInt(0x21);
      writeInt(x);
      writeInt(y);
    }
    else if (trellis.justReleased(i)) {
      i2xy(i, &x, &y);

      writeInt(0x20);
      writeInt(x);
      writeInt(y);
    }
  }
}

void loop() {
  unsigned long now = millis();

  if (Serial.available() > 0) {
    do {
      processSerial();
    } while (Serial.available() > 16);
  }
  else if (now - prevWriteTime >= 10) {
    // set trellis internal matrix from ledBuffer
    for (uint8_t x = 0; x < NUM_ROWS; x++) {
      for (uint8_t y = 0; y < NUM_COLS; y++) {
        if (ledBuffer[x][y]) {
          trellis.setLED(xy2i(x, y));
        }
        else {
          trellis.clrLED(xy2i(x, y));
        }
      }
    }

    // update display every ~10ms
    trellis.writeDisplay();

    prevWriteTime = now;
  }
  else if (now - prevReadTime >= 30) {
    // read switches not more often than every ~30ms - hardware requirement
    if (trellis.readSwitches()) {
      readKeys();
    }

    prevReadTime = now;
  }
}
