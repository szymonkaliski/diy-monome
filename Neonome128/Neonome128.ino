#include "Adafruit_NeoTrellis.h"

#define DIM_X 16
#define DIM_Y 8
#define INT_PIN 5
#define BRIGHTNESS 50

Adafruit_NeoTrellis trellis_parts[DIM_Y / 4][DIM_X / 4] = {
    {Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x30),
     Adafruit_NeoTrellis(0x2F), Adafruit_NeoTrellis(0x2E)},
    {Adafruit_NeoTrellis(0x35), Adafruit_NeoTrellis(0x34),
     Adafruit_NeoTrellis(0x33), Adafruit_NeoTrellis(0x32)}};

Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_parts, DIM_Y / 4,
                              DIM_X / 4);

String deviceID = "neonome";
String serialNum = "m1000000";

uint32_t ledBuffer[DIM_X][DIM_Y];

uint32_t prevReadTime = 0;
uint32_t prevWriteTime = 0;
uint8_t currentWriteX = 0;

void setLED(uint8_t x, uint8_t y, uint32_t value) { ledBuffer[x][y] = value; }

void setAllLEDs(uint32_t value) {
  uint8_t x, y;

  for (x = 0; x < DIM_X; x++) {
    for (y = 0; y < DIM_Y; y++) {
      ledBuffer[x][y] = value;
    }
  }
}

void turnOffLEDs() { setAllLEDs(0x000000); }

void turnOnLEDs() { setAllLEDs(0xFFFFFF); }

void setup() {
  Serial.begin(115200);

  if (!trellis.begin()) {
    Serial.println("Failed to begin trellis");
    while (1) {
    }
  }

  // full brightness bricks arduino, probably a current thing
  uint8_t x, y;
  for (x = 0; x < DIM_X / 4; x++) {
    for (y = 0; y < DIM_Y / 4; y++) {
      trellis_parts[y][x].pixels.setBrightness(BRIGHTNESS);
    }
  }

  for (x = 0; x < DIM_X; x++) {
    for (y = 0; y < DIM_Y; y++) {
      trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
      trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
    }
  }

  pinMode(INT_PIN, INPUT);

  turnOffLEDs();
}

uint8_t readInt() { return Serial.read(); }

void writeInt(uint8_t value) { Serial.write(value); }

void processSerial() {
  uint8_t identifierSent;  // command byte sent from controller to matrix
  uint8_t deviceAddress;   // device address sent from controller
  uint8_t dummy;           // for reading in data not used by the matrix
  uint8_t intensity = 255; // led intensity
  uint8_t readX, readY;    // x and y values read from driver
  uint8_t i, x, y;

  // get command identifier: first byte of packet is identifier in the form:
  // [(a << 4) + b]
  // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
  // b = command (ie. query, enable, led, key, frame)
  identifierSent = Serial.read();

  switch (identifierSent) {
  case 0x00:
    writeInt((uint8_t)0x00); // system/query response 0x00 -> 0x00
    writeInt((uint8_t)0x01); // grids
    writeInt((uint8_t)0x01); // one grid
    break;

  case 0x01:
    writeInt((uint8_t)0x01);
    for (i = 0; i < 32; i++) { // has to be 32
      if (i < deviceID.length()) {
        Serial.print(deviceID[i]);
      } else {
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
    writeInt((uint8_t)0x02); // system / request grid offsets
    writeInt((uint8_t)0);    // x offset
    writeInt((uint8_t)0);    // y offset
    break;

  case 0x04:
    dummy = readInt(); // system / set grid offset
    readX = readInt(); // an offset of 8 is valid only for 16 x 8 monome
    readY = readInt(); // an offset is invalid for y as it's only 8
    break;

  case 0x05:
    writeInt((uint8_t)0x03); // system / request grid size
    writeInt((uint8_t)DIM_X);
    writeInt((uint8_t)DIM_Y);
    break;

  case 0x06:
    readX = readInt(); // system / set grid size - ignored
    readY = readInt();
    break;

  case 0x07:
    break; // I2C stuff - ignored

  case 0x08:
    deviceAddress = readInt(); // set addr - ignored
    dummy = readInt();
    break;

  case 0x0F:
    writeInt((uint8_t)0x0F); // send serial number
    Serial.print(serialNum);
    break;

  case 0x10:
    readX = readInt();
    readY = readInt();
    setLED(readX, readY, 0x000000);
    break;

  case 0x11:
    readX = readInt();
    readY = readInt();
    setLED(readX, readY, 0xFFFFFF);
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

    for (y = 0; y < 8; y++) { // each i will be a row
      intensity = readInt();  // read one byte of 8 bits on/off

      for (x = 0; x < 8; x++) { // for 8 LEDs on a row
        setLED(readX + x, y, intensity * 0x10101);
      }
    }
    break;

  case 0x15:
    readX = readInt(); // led-grid / set row
    readY = readInt(); // may be any value

    intensity = readInt(); // read one byte of 8 bits on/off

    for (x = 0; x < 8; x++) { // for the next 8 lights in row
      setLED(readX + x, readY, intensity * 0x10101);
    }
    break;

  case 0x16:
    readX = readInt(); // led-grid / column set
    readY = readInt();

    intensity = readInt(); // read one byte of 8 bits on/off

    for (y = 0; y < 8; y++) { // for the next 8 lights in column
      setLED(readX, readY + y, intensity * 0x10101);
    }
    break;

  case 0x17:
    // ignoring intensity for entire grid
    intensity = readInt();
    break;

  case 0x18:
    readX = readInt();     // led-grid / set LED intensity
    readY = readInt();     // read the x and y coordinates
    intensity = readInt(); // read the intensity value (0-255)

    setLED(readX, readY, intensity * 0x10101);
    break;

  case 0x19: // set all leds
    intensity = readInt();
    setAllLEDs(intensity * 0x10101);
    break;

  case 0x1A: // set 8x8 block
    readX = readInt();
    readY = readInt();

    for (y = 0; y < 8; y++) {
      for (x = 0; x < 8; x++) {
        intensity = readInt();
        setLED(readX + x, readY + y, intensity * 0x10101);
      }
    }
    break;

  case 0x1B: // set 8x1 row by intensity
    readX = readInt();
    readY = readInt();

    for (x = 0; x < 8; x++) {
      intensity = readInt();
      setLED(readX + x, readY, intensity * 0x10101);
    }
    break;

  case 0x1C: // set 1x8 column by intensity
    readX = readInt();
    readY = readInt();

    for (y = 0; y < 8; y++) {
      intensity = readInt();
      setLED(readX, readY + y, intensity * 0x10101);
    }
    break;

  default:
    break;
  }

  return;
}

void loop() {
  unsigned long now = millis();
  uint8_t x, y, i, keypad_count;

  if (!digitalRead(INT_PIN)) {
    for (x = 0; x < DIM_X / 4; x++) {
      for (y = 0; y < DIM_Y / 4; y++) {
        keypad_count = trellis_parts[y][x].getKeypadCount();

        keyEventRaw e[keypad_count];
        trellis_parts[y][x].readKeypad(e, keypad_count);

        for (i = 0; i < keypad_count; i++) {
          uint8_t xx = e[i].bit.NUM % 4;
          uint8_t yy = e[i].bit.NUM / 8;

          uint8_t fx = x * 4 + xx;
          uint8_t fy = y * 4 + yy;

          if (e[i].bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
            writeInt(0x21);
          } else {
            writeInt(0x20);
          }

          writeInt(fx);
          writeInt(fy);
        }
      }
    }
  }

  if (Serial.available() > 0) {
    do {
      processSerial();
    } while (Serial.available() > 16);
  }

  for (y = 0; y < DIM_Y; y++) {
    trellis.setPixelColor(currentWriteX, y, ledBuffer[currentWriteX][y]);
  }

  for (y = 0; y < DIM_Y / 4; y++) {
    trellis_parts[y][currentWriteX / 4].pixels.show();
  }

  currentWriteX = (currentWriteX + 1) % DIM_X;

  // unoptimised:

  // if (now - prevWriteTime >= 10) {
  //   // set trellis internal matrix from ledBuffer
  //   for (uint8_t x = 0; x < DIM_X; x++) {
  //     for (uint8_t y = 0; y < DIM_Y; y++) {
  //       trellis.setPixelColor(x, y, ledBuffer[x][y]);
  //     }
  //   }

  //   trellis.show();
  //   prevWriteTime = now;
  // }

  // if (now - prevReadTime >= 30) {
  //   trellis.read();
  //   prevReadTime = now;
  // }
}
