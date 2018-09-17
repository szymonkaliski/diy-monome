/*************************************************** 
  This is a library for the Adafruit Trellis w/HT16K33

  Designed specifically to work with the Adafruit Trellis 
  ----> https://www.adafruit.com/products/1616
  ----> https://www.adafruit.com/products/1611

  These displays use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#ifdef __AVR_ATtiny85__
  #include <TinyWireM.h>
  #define Wire TinyWireM
#else
//  #include <Wire.h>
  #include <i2c_t3.h>
#endif

#include "Adafruit_Trellis_i2c_t3.h"

#define HT16K33_BLINK_CMD       0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_CMD_BRIGHTNESS  0xE0

/*
These are the lookup tables that convert the LED/button #
to the memory address in the HT16K33 - don't mess with them :)
*/

static const uint8_t PROGMEM
  ledLUT[16] =
    { 0x3A, 0x37, 0x35, 0x34,
      0x28, 0x29, 0x23, 0x24,
      0x16, 0x1B, 0x11, 0x10,
      0x0E, 0x0D, 0x0C, 0x02 },
  buttonLUT[16] =
    { 0x07, 0x04, 0x02, 0x22,
      0x05, 0x06, 0x00, 0x01,
      0x03, 0x10, 0x30, 0x21,
      0x13, 0x12, 0x11, 0x31 };

Adafruit_Trellis::Adafruit_Trellis(void) {
}

void Adafruit_Trellis::begin(uint8_t _addr = 0x70) {
  i2c_addr = _addr;

  Wire.begin();
	// HT16K33 supports 400 KHz - but this may break compatibility with other I2C devices?
	// added for teensy
    Wire.setClock(400000);

  Wire.beginTransmission(i2c_addr);
  Wire.write(0x21);  // turn on oscillator
  Wire.endTransmission();
  blinkRate(HT16K33_BLINK_OFF);
  
  setBrightness(15); // max brightness

  Wire.beginTransmission(i2c_addr);
  Wire.write(0xA1);  // turn on interrupt, active low
  Wire.endTransmission();

}

/* 
Helper button functions, the data is updated every readSwitches() call!
*/

bool Adafruit_Trellis::isKeyPressed(uint8_t k) {
  if (k > 15) return false;
  k = pgm_read_byte(&buttonLUT[k]);
  return (keys[k>>4] & _BV(k & 0x0F));
}
bool Adafruit_Trellis::wasKeyPressed(uint8_t k) {
  if (k > 15) return false;
  k = pgm_read_byte(&buttonLUT[k]);
  return (lastkeys[k>>4] & _BV(k & 0x0F));
}

boolean Adafruit_Trellis::justPressed(uint8_t k) {
  return (isKeyPressed(k) & !wasKeyPressed(k));
}
boolean Adafruit_Trellis::justReleased(uint8_t k) {
  return (!isKeyPressed(k) & wasKeyPressed(k));
}

/* 
Helper LED functions, the data is written on writeDisplay()
*/


boolean Adafruit_Trellis::isLED(uint8_t x) {
  if (x > 15) return false;
  x = pgm_read_byte(&ledLUT[x]);
  return ((displaybuffer[x >> 4] & _BV(x & 0x0F)) > 0);
}
void Adafruit_Trellis::setLED(uint8_t x) {
  if (x > 15) return;
  x = pgm_read_byte(&ledLUT[x]);
  displaybuffer[x >> 4] |= _BV(x & 0x0F);
}
void Adafruit_Trellis::clrLED(uint8_t x) {
  if (x > 15) return;
  x = pgm_read_byte(&ledLUT[x]);
  displaybuffer[x >> 4] &= ~_BV(x & 0x0F);
}


/* 
   Gets the switch memory data and updates the last/current read
*/

boolean Adafruit_Trellis::readSwitches(void) {
  memcpy(lastkeys, keys, sizeof(keys));

  Wire.beginTransmission((byte)i2c_addr);
  Wire.write(0x40);
  Wire.endTransmission();
  Wire.requestFrom((byte)i2c_addr, (byte)6);
  for (uint8_t i=0; i<6; i++) 
    keys[i] = Wire.read();

  for (uint8_t i=0; i<6; i++) {
    if (lastkeys[i] != keys[i]) {
       for (uint8_t j=0; j<6; j++) {
	 //Serial.print(keys[j], HEX); Serial.print(" ");
       }
       //Serial.println();
      return true;
    }
  }
  return false;
}

void Adafruit_Trellis::setBrightness(uint8_t b) {
  if (b > 15) b = 15;
  Wire.beginTransmission(i2c_addr);
  Wire.write(HT16K33_CMD_BRIGHTNESS | b);
  Wire.endTransmission();  
}

void Adafruit_Trellis::blinkRate(uint8_t b) {
  Wire.beginTransmission(i2c_addr);
  if (b > 3) b = 0; // turn off if not sure
  
  Wire.write(HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1)); 
  Wire.endTransmission();
}


void Adafruit_Trellis::writeDisplay(void) {
  Wire.beginTransmission(i2c_addr);
  Wire.write((uint8_t)0x00); // start at address $00

  for (uint8_t i=0; i<8; i++) {
    Wire.write(displaybuffer[i] & 0xFF);    
    Wire.write(displaybuffer[i] >> 8);    
  }
  Wire.endTransmission();  
}

void Adafruit_Trellis::clear(void) {
  memset(displaybuffer, 0, sizeof(displaybuffer));
}


/*************************************************************************/

// Maximum 8 matrices (3 address pins)
Adafruit_TrellisSet::Adafruit_TrellisSet(Adafruit_Trellis *matrix0, 
				      Adafruit_Trellis *matrix1,
				      Adafruit_Trellis *matrix2,
				      Adafruit_Trellis *matrix3,
				      Adafruit_Trellis *matrix4,
				      Adafruit_Trellis *matrix5,
				      Adafruit_Trellis *matrix6,
				      Adafruit_Trellis *matrix7) {
  matrices[0] = matrix0;
  matrices[1] = matrix1;
  matrices[2] = matrix2;
  matrices[3] = matrix3;
  matrices[4] = matrix4;
  matrices[5] = matrix5;
  matrices[6] = matrix6;
  matrices[7] = matrix7;

  _nummatrix = 0;

  for (uint8_t i=0; i<8; i++) {
    if (matrices[i] != 0)
      _nummatrix= i+1;
    else break;
  }
}



void Adafruit_TrellisSet::begin(uint8_t addr0, uint8_t addr1,
				uint8_t addr2, uint8_t addr3,
				uint8_t addr4, uint8_t addr5,
				uint8_t addr6, uint8_t addr7) {
  uint8_t addrs[8] = {addr0, addr1, addr2, addr3, addr4, addr5, addr6, addr7};

  for (uint8_t i=0; i<_nummatrix; i++) {
    if (matrices[i] != 0)
      matrices[i]->begin(addrs[i]);
  }
}

/* 
Helper button functions, the data is updated every readSwitches() call!
*/

bool Adafruit_TrellisSet::isKeyPressed(uint8_t k) {
  if (k > 127) return false;
  uint8_t matrix, key;

  // determine submatrix #
  matrix = k / 16;
  key = k % 16;

  // not that many matrices!
  if (matrix >= _nummatrix) return false;

  return  matrices[matrix]->isKeyPressed(key);
}

bool Adafruit_TrellisSet::wasKeyPressed(uint8_t k) {
  if (k > 127) return false;
  uint8_t matrix, key;
  
  // determine submatrix #
  matrix = k / 16;
  key = k % 16;

  // not that many matrices!
  if (matrix >= _nummatrix) return false;

  return  matrices[matrix]->wasKeyPressed(key);
}

boolean Adafruit_TrellisSet::justPressed(uint8_t k) {
  return (isKeyPressed(k) & !wasKeyPressed(k));
}
boolean Adafruit_TrellisSet::justReleased(uint8_t k) {
  return (!isKeyPressed(k) & wasKeyPressed(k));
}

/* 
Helper LED functions, the data is written on writeDisplay()
*/


boolean Adafruit_TrellisSet::isLED(uint8_t x) {
  if (x > 127) return false;
  uint8_t matrix, led;
  
  // determine submatrix #
  matrix = x / 16;
  led = x % 16;

  // not that many matrices!
  if (matrix >= _nummatrix) return false;

  return  matrices[matrix]->isLED(led);
}

void Adafruit_TrellisSet::setLED(uint8_t x) {
  if (x > 127) return ;
  uint8_t matrix, led;
  
  // determine submatrix #
  matrix = x / 16;
  led = x % 16;

  // not that many matrices!
  if (matrix >= _nummatrix) return ;

  matrices[matrix]->setLED(led);
}

void Adafruit_TrellisSet::clrLED(uint8_t x) {
  if (x > 127) return ;
  uint8_t matrix, led;
  
  // determine submatrix #
  matrix = x / 16;
  led = x % 16;

  // not that many matrices!
  if (matrix >= _nummatrix) return ;

  matrices[matrix]->clrLED(led);
}


/* 
   Gets the switch memory data and updates the last/current read
*/

boolean Adafruit_TrellisSet::readSwitches(void) {
  boolean changed = false;
  for (uint8_t i=0; i<_nummatrix; i++) {
    if (matrices[i] != 0)
      changed = changed || matrices[i]->readSwitches();
  }
  return changed;
}

void Adafruit_TrellisSet::setBrightness(uint8_t b) {
 for (uint8_t i=0; i<_nummatrix; i++) {
   if (matrices[i] != 0)
     matrices[i]->setBrightness(b);
 }
}

void Adafruit_TrellisSet::blinkRate(uint8_t b) {
 for (uint8_t i=0; i<_nummatrix; i++) {
   if (matrices[i] != 0)
     matrices[i]->blinkRate(b);
 }
}


void Adafruit_TrellisSet::writeDisplay(void) {
 for (uint8_t i=0; i<_nummatrix; i++) {
   if (matrices[i] != 0)
     matrices[i]->writeDisplay();
 } 
}

void Adafruit_TrellisSet::clear(void) {
 for (uint8_t i=0; i<_nummatrix; i++) {
   if (matrices[i] != 0)
     matrices[i]->clear();
 }
}
