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

#ifndef _TRELLIS_H_
#define _TRELLIS_H_

// Set _BV if not already set (eg. Arudiono DUE, Arduino Zero Pro)
#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#ifdef __AVR_ATtiny85__
  #include <TinyWireM.h>
#else
//  #include <Wire.h>
  #include <i2c_t3.h>
#endif

#define LED_ON  1
#define LED_OFF 0

#define HT16K33_BLINK_OFF    0
#define HT16K33_BLINK_2HZ    1
#define HT16K33_BLINK_1HZ    2
#define HT16K33_BLINK_HALFHZ 3


// this is the raw HT16K33 controller
class Adafruit_Trellis {
 public:
  Adafruit_Trellis(void);
  void begin(uint8_t _addr);
  void setBrightness(uint8_t b);
  void blinkRate(uint8_t b);
  void writeDisplay(void);
  void clear(void);
  bool isKeyPressed(uint8_t k);
  bool wasKeyPressed(uint8_t k);
  boolean isLED(uint8_t x);
  void setLED(uint8_t x);
  void clrLED(uint8_t x);
  boolean readSwitches(void);
  boolean justPressed(uint8_t k);
  boolean justReleased(uint8_t k);

  uint16_t displaybuffer[8];

  void init(uint8_t a);
 private:
  uint8_t keys[6], lastkeys[6];

  uint8_t i2c_addr;
};


// control a large # at a time!
class Adafruit_TrellisSet {
 public:
  Adafruit_TrellisSet(Adafruit_Trellis *matrix0, 
		      Adafruit_Trellis *matrix1=0,
		      Adafruit_Trellis *matrix2=0,
		      Adafruit_Trellis *matrix3=0,
		      Adafruit_Trellis *matrix4=0,
		      Adafruit_Trellis *matrix5=0,
		      Adafruit_Trellis *matrix6=0,
		      Adafruit_Trellis *matrix7=0);
  void begin(uint8_t _addr0 = 0x70, uint8_t _addr1 = 0x71,
	     uint8_t _addr2 = 0x72, uint8_t _addr3 = 0x73,
	     uint8_t _addr4 = 0x74, uint8_t _addr5 = 0x75,
	     uint8_t _addr6 = 0x76, uint8_t _addr7 = 0x77);

  void setBrightness(uint8_t b);
  void blinkRate(uint8_t b);
  void writeDisplay(void);
  void clear(void);
  bool isKeyPressed(uint8_t k);
  bool wasKeyPressed(uint8_t k);
  boolean isLED(uint8_t x);
  void setLED(uint8_t x);
  void clrLED(uint8_t x);
  boolean readSwitches(void);
  boolean justPressed(uint8_t k);
  boolean justReleased(uint8_t k);

 private:
  Adafruit_Trellis *matrices[8];
  uint8_t _nummatrix;
};

#endif // _TRELLIS_H_
