/*------------------------------------------------------------------------
  itsagrid varibright 64/128

  Based on code for Trellinome by szymonkaliski  https://github.com/szymonkaliski/diy-monome 
  and the Adafruit UNTZtrument - a Trellis button controller.
  https://github.com/adafruit/Adafruit_UNTZtrument
  
  Changing addresses of HOLTEK switch driver chips with jumpers on each pcb
  
  see void setup() below for order of i2c addresses for HOLTEK

  A0 sets the lowest bit with a value of 1 
  A1 sets the middle bit with a value of 2
  A2 sets the high bit with a value of 4
  The final address is 0x70 + A2 + A1 + A0. 

  1 -           0x70
  2 A0          0x71
  3 A1          0x72
  4 A0+A1       0x73
  5 A2          0x74
  6 A0+A2       0x75
  7 A1+A2       0x76
  8 A0+A1+A2    0x77
  
  4 or 8 board matrices can be used.  #define NUM_TRELLIS & NUM_BOARDS to the number in use.

  Similar in function to Adafruit UNTZ and HellaUNTZ
  ----> https://www.adafruit.com/products/1919
  ----> https://www.adafruit.com/products/1999

  Trellis code written by Limor Fried/Ladyada for Adafruit Industries.  
  UNTZ key code by Phil Burgess for Adafruit Industries
  Monome emulation written by Mike Barela for Adafruit Industries
  MIT license, all text above must be included in any redistribution
  
  Version 0.6  2018-11-04  

  This version uses the i2c_t3 Teensy library so the Trellis and TLC59116 libraries 
  need to be modified to use this as well. I copied these to a new version and added 
  "_i2c_t3" to the names. 
  ------------------------------------------------------------------------*/
// #define DEBUG 1

#include <i2c_t3.h>
#include <TLC59116_i2c_t3.h>

#include "Adafruit_Trellis_i2c_t3.h"
 
#define NUM_TRELLIS (8)    // either 4 = 64, 8 = 128
#define NUM_KEYS    (NUM_TRELLIS * 16)

#define NUM_BOARDS 8
TLC59116 led_boards[NUM_BOARDS] = {
  TLC59116(0x65), TLC59116(0x66), TLC59116(0x62), TLC59116(0x60),
  TLC59116(0x67), TLC59116(0x6E), TLC59116(0x64), TLC59116(0x63)
};

Adafruit_Trellis matrixes[NUM_TRELLIS] = {    // Instance matrix using number of Trellises set above
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis()
#if NUM_TRELLIS > 4
  ,Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis(),
  Adafruit_Trellis()
#endif
};

Adafruit_TrellisSet trellis = Adafruit_TrellisSet(
          &matrixes[0], &matrixes[1], &matrixes[2], &matrixes[3]
  #if NUM_TRELLIS > 4
          ,&matrixes[4], &matrixes[5], &matrixes[6], &matrixes[7]
  #endif
);

String deviceID  = "monome";
String serialNum = "m1000010"; // this does not get used - teensy serial number is picked up instead

const uint8_t gridNumber = 0x01;            // This code is for Grid #2 (change for > 1 grid) ?????
const uint8_t numGrids = (NUM_TRELLIS / 4); // ????

const uint8_t gridX    = (NUM_TRELLIS*2);   // Will be either 8 or 16
const uint8_t gridY    = 8;                 // standard

uint8_t       offsetX  = 0;                 // offset for 128 only (8x8 can't offset)
uint8_t       variMonoThresh  = 0;                 //  intensity at which led will get set to off
                                                   // set to 0 for varibright
unsigned long prevReadTime  = 0;
unsigned long prevWriteTime = 0;

static uint8_t ledBuffer[gridX][gridY];
uint8_t led_array[128] = { 0 };

bool dirtyquad = 0;

// -------

// These functions are from Adafruit_UNTZtrument.h - transplanted here to keep code size low
// If your code uses encoders, go ahead and use the UNTZtrument library
// Lookup tables take some PROGMEM size but they make for fast constant-time lookup.
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
    {  3,  2,  1,  0, 19, 18, 17, 16 },
    {  7,  6,  5,  4, 23, 22, 21, 20 },
    { 11, 10,  9,  8, 27, 26, 25, 24 },
    { 15, 14, 13, 12, 31, 30, 29, 28 },
    { 35, 34, 33, 32, 51, 50, 49, 48 },
    { 39, 38, 37, 36, 55, 54, 53, 52 },
    { 43, 42, 41, 40, 59, 58, 57, 56 },
    { 47, 46, 45, 44, 63, 62, 61, 60 }},
    
  xy2i128[8][16] = {
    {  3,  2,  1,  0, 19, 18, 17, 16,  35, 34, 33, 32, 51, 50, 49, 48 },
    {  7,  6,  5,  4, 23, 22, 21, 20,  39, 38, 37, 36, 55, 54, 53, 52 },
    { 11, 10,  9,  8, 27, 26, 25, 24,  43, 42, 41, 40, 59, 58, 57, 56 },
    { 15, 14, 13, 12, 31, 30, 29, 28,  47, 46, 45, 44, 63, 62, 61, 60 },
    
    { 67, 66, 65, 64,  83, 82, 81, 80,  99, 98, 97, 96, 115,114,113,112 },
    { 71, 70, 69, 68,  87, 86, 85, 84, 103,102,101,100, 119,118,117,116 },
    { 75, 74, 73, 72,  91, 90, 89, 88, 107,106,105,104, 123,122,121,120 },
    { 79, 78, 77, 76,  95, 94, 93, 92, 111,110,109,108, 127,126,125,124 } 
};  


uint8_t xy2i(uint8_t x, uint8_t y) {
  if(y > 7) return 255;
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

void setup() {

  Serial.begin(115200);
  //Serial2.begin(115200); // send to serial 2 pins for debug
  
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 2400000);
  //Wire.setDefaultTimeout(10000); // 10ms
  
   // Initialize HOLTEK - change addresses as needed
    trellis.begin(              // addr is the I2C address of the upper left, upper right, lower left and lower right matrices, respectively
      0x74, 0x75, 0x71, 0x70   // 2x2 arrangement - 64
    #if NUM_TRELLIS > 4
      ,0x76, 0x77, 0x73, 0x72   // 4x2 arrangement - 128
    #endif 
    );
    // trellis.begin(0x70, 0x71, 0x72, 0x73);
    
    // initialize pwm
    for (int i = 0; i < NUM_BOARDS; i++)
      led_boards[i].begin();
    
    //setAllLEDs(1); // blink leds on at start
    //delay(20);
}

uint8_t readInt() {
  uint8_t val = Serial.read();
  //Serial2.write(val); // send to serial 2 pins for debug
  return val; 
}

void writeInt(uint8_t value) {
#if DEBUG
   //Serial.print(value,HEX);       // For debug, values are written to the serial monitor in Hexidecimal
   Serial.print(value);       
   Serial.println(" ");
#else
   Serial.write(value);           // standard is to write out the 8 bit value on serial
#endif
}

void processSerial() {
  uint8_t identifierSent;                     // command byte sent from controller to matrix
  uint8_t deviceAddress;                      // device address sent from controller
  uint8_t dummy;                              // for reading in data not used by the matrix
  uint8_t intensity = 15;                     // default full led intensity
  uint8_t readX, readY;                       // x and y values read from driver
  uint8_t i, x, y, z;


  identifierSent = Serial.read();             // get command identifier: first byte of packet is identifier in the form: [(a << 4) + b]
                                              // a = section (ie. system, key-grid, digital, encoder, led grid, tilt)
                                              // b = command (ie. query, enable, led, key, frame)
  
  //Serial2.write(identifierSent);    // send to serial 2 pins for debug                  
  
  switch (identifierSent) {
    case 0x00:                  // device information
      writeInt((uint8_t)0x00);                // system/query response 0x00 -> 0x00
      writeInt((uint8_t)0x01);                // grids
      writeInt((uint8_t)0x01);                // one grid
      break;

    case 0x01:                  // device ID string
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
      writeInt((uint8_t)0x00);                // x offset - could be 0 or 8 
      writeInt((uint8_t)0x00);                // y offset
      break;

    case 0x04:
      dummy = readInt();                      // system / set grid offset
      readX = readInt();                      // an offset of 8 is valid only for 16 x 8 monome
      readY = readInt();                      // an offset is invalid for y as it's only 8
      //if(NUM_KEYS > 64 && readX == 8) offsetX = 8; 
      break;

    case 0x05:
      writeInt((uint8_t)0x03);                // system / request grid size
      writeInt(gridX);
      writeInt(gridY);
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
      
  // 0x10-0x1F are for an LED Grid Control.  All bytes incoming, no responses back
  
    case 0x10:            // /prefix/led/set x y [0/1]
      readX = readInt();
      readY = readInt();
      setLED(readX, readY, 0);
      break;

    case 0x11:            // /prefix/led/set x y [0/1]
      readX = readInt();
      readY = readInt();
      setLED(readX, readY, 15);   // probably should have a brightness global or something
      break;

    case 0x12:            //  /prefix/led/all [0/1]  
      turnOffLEDs();
      sendBufferedLeds();  // send commands
      
      break;

    case 0x13:                      //  /prefix/led/all [0/1]
      turnOnLEDs();
      sendBufferedLeds();  // send commands
      
      break;

    case 0x14:                  // /prefix/led/map x y d[8]
      readX = readInt();
      readY = readInt();
      //readX >> 3; readX << 3;                 // floor to multiple of 8
      //readY >> 3; readY << 3;
      //if(readX == 8 && NUM_KEYS > 64) break;  // trying to set an 8x16 grid on a pad with only 64 keys
      if (readY != 0) break;                  // since we only have 8 LEDs in a column, no offset

      for (y = 0; y < gridY; y++) {           // each i will be a row
        intensity = readInt();                // read one byte of 8 bits on/off
    
        for (x = 0; x < gridX; x++) {          // for 8 LEDs on a row
          if ((intensity >> x) & 0x01) {      // set LED if the intensity bit is set
            writeBufferedLed(readX + x, y, 15);
          }
          else {
            writeBufferedLed(readX + x, y, 0);
          }
        }
      }
      sendBufferedLeds();
      break;

    case 0x15:                                //  /prefix/led/row x y d
      readX = readInt();                      // led-grid / set row
      readY = readInt();                      // may be any value
      intensity = readInt();                  // read one byte of 8 bits on/off
      //readX >> 3; readX << 3;

      for (i = 0; i < 8; i++) {               // for the next 8 lights in row
        if ((intensity >> i) & 0x01) {        // if intensity bit set, light led
          setLED(readX + i, readY, intensity);
        }
        else {
          setLED(readX + i, readY, 0);
        }
      }

      break;

    case 0x16:                                //  /prefix/led/col x y d
      readX = readInt();                      // led-grid / column set
      readY = readInt();
      //readY >> 3 ; readY << 3;                // floor to multiple of 8
      intensity = readInt();                  // read one byte of 8 bits on/off
      if (readY != 0) break;                  // we only have 8 lights in a column

      for (i = 0; i < gridY; i++) {           // for the next 8 lights in column
        if ((intensity >> i) & 0x01) {        // if intensity bit set, light led
          setLED(readX, i, intensity);
        }
        else {
          setLED(readX, i, 0);
        }
      }

      break;

    case 0x17:                                     //  /prefix/led/intensity i
      intensity = readInt();                      // set brightness for entire grid
      // this is probably not right
      setAllLEDs(intensity);
      sendBufferedLeds();  // send commands
      
      break;

    case 0x18:                                //  /prefix/led/level/set x y i
      readX = readInt();                      // led-grid / set LED intensity
      readY = readInt();                      // read the x and y coordinates
      intensity = readInt();                  // read the intensity

      if (intensity > variMonoThresh) {       // because monobright, if intensity > variMonoThresh
        writeBufferedLed(readX, readY, intensity);
        //setLED(readX, readY, intensity);      //   set the pixel
      }
      else {
        writeBufferedLed(readX, readY, 0);
        //setLED(readX, readY, 0);              //   otherwise clear the pixel
      }
      sendBufferedLeds();
      break;

    case 0x19:                               //  /prefix/led/level/all s
      intensity = readInt();                 // set all leds
      
      if (intensity > variMonoThresh) {
        setAllLEDs(intensity);
      }
      else {
        setAllLEDs(0);
        //turnOffLEDs();                       // turn off if intensity = 0
      }
      sendBufferedLeds();  // send commands
      break;

    case 0x1A:                               //   /prefix/led/level/map x y d[64]
     readX = readInt();                      // set 8x8 block
     //readX << 3; readX >> 3;
     readY = readInt();
     //readY << 3; readY >> 3;

      if (readY == 0){  // only loop if y = 0 since we only have 1 or 2 quads with 64/128 buttons
        z = 0;
        for (y = 0; y < 8; y++) {
          for (x = 0; x < 8; x++) {
            if (z % 2 == 0) {                    
              intensity = readInt();
              if ( ((intensity >> 4) & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
                writeBufferedLed(readX + x, y, (intensity >> 4) & 0x0F);
                //setLED(readX + x, y, intensity >> 4);
              } else {
                writeBufferedLed(readX + x, y, 0);
                //setLED(readX + x, y, 0);
              }
            } else {                        
              if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
                writeBufferedLed(readX + x, y, intensity & 0x0F);
                //setLED(readX + x, y, intensity);
              } else {
                writeBufferedLed(readX + x, y, 0);
                //setLED(readX + x, y, 0);
              }
            }
            z++;
          }
        }
        sendBufferedLeds();
      } else {
        for (int q = 0; q<32; q++){
          readInt();
        }
      }

      
      break;

    case 0x1B:                                // /prefix/led/level/row x y d[8]
      readX = readInt();                      // set 8x1 block of led levels, with offset
      // readX << 3; readX >> 3;                 // x = x offset, will be floored to multiple of 8 by firmware
      readY = readInt();                      // y = y offset
      for (x = 0; x < 8; x++) {
          if (x % 2 == 0) {                    
            intensity = readInt();
            
            if ( (intensity >> 4 & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
              writeBufferedLed(readX + x, readY, intensity);
              //setLED(readX + x, readY, intensity);
            }
            else {
              writeBufferedLed(readX + x, readY, 0);
              //setLED(readX + x, readY, 0);
            }
          } else {                              
            if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
              writeBufferedLed(readX + x, readY, intensity);
              //setLED(readX + x, readY, intensity);
            }
            else {
              writeBufferedLed(readX + x, readY, 0);
              //setLED(readX + x, readY, 0);
            }
          }
      }
      sendBufferedLeds();
      break;

    case 0x1C:                                // /prefix/led/level/col x y d[8]
      readX = readInt();                      // set 1x8 block of led levels, with offset
      readY = readInt();                      // x = x offset
      // readY << 3; readY >> 3;                 // y = y offset, will be floored to multiple of 8 by firmware
      for (y = 0; y < gridY; y++) {
          if (y % 2 == 0) {                    
            intensity = readInt();
              
            if ( (intensity >> 4 & 0x0F) > variMonoThresh) {  // even bytes, use upper nybble
              writeBufferedLed(readX, y, intensity);
              //setLED(readX, y, intensity);
            }
            else {
              writeBufferedLed(readX, y, 0);
              //setLED(readX, y, 0);
            }
          } else {                              
            if ((intensity & 0x0F) > variMonoThresh ) {      // odd bytes, use lower nybble
              writeBufferedLed(readX, y, intensity);
              //setLED(readX, y, intensity);
            }
            else {
              writeBufferedLed(readX, y, 0);
              //setLED(readX, y, 0);
            }
          }
      }
      sendBufferedLeds();
      break;
 
    default:
      break;
  }

  return;
}


void setLED(int x, int y, int value) { // just go ahead and light each pixel individually
  uint8_t idx = xy2i(x, y);
  int board_num = idx / 16; // idx >> 4
  int led_num   = idx % 16; // idx & 0xF or idx & 15

  if (x >= 0 && x < gridX && y >= 0 && y < gridY) {
    led_boards[board_num].analogWrite(led_num, value * 16); // val << 4
  //  ledBuffer[x][y] = value;
  }
}

void setAllLEDs(int value) {
  uint8_t i, j;
  for (i = 0; i < gridX; i++) {
    for (j = 0; j < gridY; j++) {
      //setLED(i, j, value);
      led_array[xy2i(i,j)] = value*16;  // update led array
    }
  }
  
}

void turnOffLEDs() {
  setAllLEDs(0);
}

void turnOnLEDs() {
  setAllLEDs(15);
}

void writeBufferedLed(uint8_t x, uint8_t y, uint8_t bright){  // update led array
  led_array[xy2i(x,y)] = bright*16;
}

void sendBufferedLeds(){  // faster routine to buffer the send to each PWM chip
  for (uint8_t i = 0; i < NUM_BOARDS; i++){
    led_boards[i].analogWriteBatch(0, &led_array[i*16], 16);
  }
}


void writeLEDbuffer() {   //not using this anymore
   //uint8_t i = 0;
   for (uint8_t x = 0; x < gridX; x++) {
      for (uint8_t y = 0; y < gridY; y++) {

        // intensity value is stored in ledBuffer[x][y]
       led_boards[0].analogWrite(xy2i(x, y), ledBuffer[x][y]*8);
        
       /* 
        if (ledBuffer[x][y] > 0) {
          led_boards[0].analogWrite(xy2i(x, y), ledBuffer[x][y]*8);
          //trellis.setLED(xy2i(x, y));
        }
        else {
          //led_board1.analogWrite(xy2i(x, y), 0);
          led_boards[0].analogWrite(xy2i(x, y), ledBuffer[x][y]*8);
          //trellis.clrLED(xy2i(x, y));
        }
        */
      }
    }     
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
    do { processSerial();  } 
    while (Serial.available() > 16);
  }
  
  if (now - prevReadTime >= 30) {
    // read switches not more often than every ~30ms - hardware requirement (???)
    if (trellis.readSwitches() > 0) {
      readKeys();
     }
     
    prevReadTime = now;
    //delay(1);
  }
//    if (now - prevWriteTime >= 20) {     
//      // update display every ~10ms   
//      prevWriteTime = now;
//    }
 
}
