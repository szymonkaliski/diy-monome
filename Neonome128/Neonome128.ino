#include <Adafruit_NeoTrellis.h>

#define DIM_X 16
#define DIM_Y 8

#define INT_PIN 4
#define LED_PIN 13 // internal LED used to show boot info

#define BRIGHTNESS 100
#define R 255
#define G 255
#define B 255

Adafruit_NeoTrellis trellis_parts[DIM_Y / 4][DIM_X / 4] = {
    {Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x30),
     Adafruit_NeoTrellis(0x2F), Adafruit_NeoTrellis(0x2E)},
    {Adafruit_NeoTrellis(0x35), Adafruit_NeoTrellis(0x34),
     Adafruit_NeoTrellis(0x33), Adafruit_NeoTrellis(0x32)}};

Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_parts, DIM_Y / 4,
                              DIM_X / 4);

String deviceID = "neonome";
String serialNum = "m4676000";

uint8_t ledBuffer[DIM_X * DIM_Y];
uint8_t prevLedBuffer[DIM_X * DIM_Y];
bool isInited = false;

elapsedMillis refreshTime;

void setLED(uint8_t x, uint8_t y, uint8_t value) {
  // sanity check, but it was killing teensy looks like norns by default sends
  // offsets for 256, so we're ignoring values outside of available dimensions
  if (x >= DIM_X || y >= DIM_Y) {
    return;
  }

  int index = x + (y << 4);
  ledBuffer[index] = value;
}

void setAllLEDs(uint8_t value) {
  uint8_t x, y;

  for (x = 0; x < DIM_X; x++) {
    for (y = 0; y < DIM_Y; y++) {
      setLED(x, y, value);
    }
  }
}

void turnOffLEDs() { setAllLEDs(0); }

void turnOnLEDs() { setAllLEDs(15); }

TrellisCallback keyCallback(keyEvent evt) {
  uint8_t x = evt.bit.NUM % DIM_X;
  uint8_t y = evt.bit.NUM / DIM_X;

  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
    Serial.write(0x20);
  } else {
    Serial.write(0x21);
  }

  Serial.write(x);
  Serial.write(y);

  return 0;
}

void setup() {
  Serial.begin(115200);

  pinMode(INT_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}

void processSerial() {
  uint8_t identifierSent; // command byte sent from controller to matrix
  uint8_t intensity = 15; // led intensity
  uint8_t readX, readY;   // x and y values read from driver
  uint8_t i, x, y, z;

  // get command identifier: first byte of packet is identifier in the form:
  // [(a << 4) + b]
  // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
  // b = command (ie. query, enable, led, key, frame)
  identifierSent = Serial.read();

  switch (identifierSent) {
  case 0x00:
    Serial.write(0x00); // system/query response 0x00 -> 0x00
    Serial.write(0x01); // grids
    Serial.write(0x01); // one grid
    break;

  case 0x01:
    Serial.write(0x01);
    for (i = 0; i < 32; i++) { // has to be 32
      if (i < deviceID.length()) {
        Serial.write(deviceID[i]);
      } else {
        Serial.write(0x00);
      }
    }
    break;

  case 0x02:
    for (i = 0; i < 32; i++) {
      deviceID[i] = Serial.read();
    }
    break;

  case 0x03:
    Serial.write(0x02); // system / request grid offsets
    Serial.write(0x01); // n number
    Serial.write(0);    // x offset
    Serial.write(0);    // y offset
    break;

  case 0x04:
    Serial.read(); // system / set grid offset
    Serial.read(); // grid number
    Serial.read(); // x offset
    Serial.read(); // y offset
    break;

  case 0x05:
    Serial.write(0x03); // system / request grid size
    Serial.write(DIM_X);
    Serial.write(DIM_Y);
    break;

  case 0x06:
    // set grid size
    Serial.read(); // size x
    Serial.read(); // size y
    break;

  case 0x07:
    // get ADDR (I2C)
    break;

  case 0x08:
    // set ADDR (I2C)
    Serial.read(); // addr to set
    Serial.read(); // new addr value
    break;

  case 0x0F:
    Serial.write(0x0F);       // send serial number
    for (i = 0; i < 8; i++) { // has to be 8
      if (i < serialNum.length()) {
        Serial.write(serialNum[i]);
      } else {
        Serial.write(0x00);
      }
    }
    break;

  case 0x10:
    readX = Serial.read();
    readY = Serial.read();
    setLED(readX, readY, 0);
    break;

  case 0x11:
    readX = Serial.read();
    readY = Serial.read();
    setLED(readX, readY, 15);
    break;

  case 0x12:
    turnOffLEDs();
    break;

  case 0x13:
    turnOnLEDs();
    break;

  case 0x14:
    readX = Serial.read();
    readY = Serial.read();

    for (y = 0; y < 8; y++) {    // each i will be a row
      intensity = Serial.read(); // read one byte of 8 bits on/off

      for (x = 0; x < 8; x++) { // for 8 LEDs on a row
        setLED(readX + x, y, intensity);
      }
    }
    break;

  case 0x15:
    readX = Serial.read(); // led-grid / set row
    readY = Serial.read(); // may be any value

    intensity = Serial.read(); // read one byte of 8 bits on/off

    for (x = 0; x < 8; x++) {        // for the next 8 lights in row
      if ((intensity >> x) & 0x01) { // if intensity bit set, light led
        setLED(readX + x, readY, intensity);
      } else {
        setLED(readX + x, readY, 0);
      }
    }
    break;

  case 0x16:
    readX = Serial.read(); // led-grid / column set
    readY = Serial.read();

    intensity = Serial.read(); // read one byte of 8 bits on/off

    for (y = 0; y < 8; y++) {        // for the next 8 lights in column
      if ((intensity >> y) & 0x01) { // if intensity bit set, light led
        setLED(readX, readY + y, intensity);
      } else {
        setLED(readX, readY + y, 0);
      }
    }
    break;

  case 0x17:
    // intensity for entire grid - ignored
    Serial.read();
    break;

  case 0x18:
    readX = Serial.read();     // led-grid / set LED intensity
    readY = Serial.read();     // read the x and y coordinates
    intensity = Serial.read(); // read the intensity value (0-255)

    setLED(readX, readY, intensity);
    break;

  case 0x19: // set all leds
    intensity = Serial.read();
    setAllLEDs(intensity);
    break;

  case 0x1A: // set 8x8 block
    readX = Serial.read();
    readY = Serial.read();

    z = 0;

    for (y = 0; y < 8; y++) {
      for (x = 0; x < 8; x++) {
        if (z % 2 == 0) {
          // even bytes, read value and use upper nybble
          intensity = Serial.read();

          setLED(readX + x, readY + y, (intensity >> 4 & 0x0F));
        } else {
          // odd bytes, use lower nybble
          setLED(readX + x, readY + y, (intensity & 0x0F));
        }

        z++;
      }
    }
    break;

  case 0x1B: // set 8x1 row by intensity
    readX = Serial.read();
    readY = Serial.read();

    for (x = 0; x < 8; x++) {
      if (x % 2 == 0) {
        // even bytes, read value and use upper nybble
        intensity = Serial.read();

        setLED(readX + x, readY, (intensity >> 4 & 0x0F));
      } else {
        // odd bytes, use lower nybble
        setLED(readX + x, readY, (intensity & 0x0F));
      }
    }
    break;

  case 0x1C: // set 1x8 column by intensity
    readX = Serial.read();
    readY = Serial.read();

    for (y = 0; y < 8; y++) {
      if (y % 2 == 0) {
        // even bytes, read value and use upper nybble
        intensity = Serial.read();

        setLED(readX, readY + y, (intensity >> 4 & 0x0F));
      } else {
        // odd bytes, use lower nybble
        setLED(readX, readY + y, (intensity & 0x0F));
      }
    }
    break;

  default:
    Serial.read(); // clean up until we find a frame?
    break;
  }

  return;
}

void processLEDs() {
  uint8_t value, prevValue = 0;
  uint32_t hexColor;

  bool isDirty = false;

  // set trellis internal matrix from ledBuffer, only for changed value
  for (int i = 0; i < DIM_X * DIM_Y; i++) {
    value = ledBuffer[i];
    prevValue = prevLedBuffer[i];

    if (value != prevValue) {
      hexColor = (((R * value) >> 4) << 16) + (((G * value) >> 4) << 8) +
                 ((B * value) >> 4);

      trellis.setPixelColor(i, hexColor);

      prevLedBuffer[i] = value;
      isDirty = true;
    }
  }

  if (isDirty) {
    trellis.show();
  }
}

void loop() {
  if (Serial.available()) {
    processSerial();
  }

  // refresh every 16ms or so
  if (isInited && refreshTime > 16) {
    trellis.read();
    processLEDs();

    refreshTime = 0;
  }

  // delayed init since trellis.begin() and trellis.activateKey() take some
  // time, and norns starts sending serial messages right away
  if (!isInited && refreshTime > 500) {
    trellis.begin();

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
        trellis.registerCallback(x, y, keyCallback);
      }
    }

    refreshTime = 0;
    isInited = true;
    digitalWrite(LED_PIN, LOW);
  }
}
