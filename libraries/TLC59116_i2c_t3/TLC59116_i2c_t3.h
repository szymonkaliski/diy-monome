/*
 * Copyright (c) 2014, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 *  1. Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 * 
 *  3. Neither the name of Majenko Technologies nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without 
 *     specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef _TLC59116_H
#define _TLC59116_H

#if (ARDUINO >= 100) 
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

#include <i2c_t3.h>

#define TLC59116_BASEADDR   0b1100000
#define TLC59116_ALLCALL    0b1101000
#define TLC59116_SUB1       0b1101001
#define TLC59116_SUB2       0b1101010
#define TLC59116_SUB3       0b1101100
#define TLC59116_RESET      0b1101011

#define TLC59116_MODE1      0x00
#define TLC59116_MODE2      0x01
#define TLC59116_PWM0       0x02
#define TLC59116_PWM1       0x03
#define TLC59116_PWM2       0x04
#define TLC59116_PWM3       0x05
#define TLC59116_PWM4       0x06
#define TLC59116_PWM5       0x07
#define TLC59116_PWM6       0x08
#define TLC59116_PWM7       0x09
#define TLC59116_PWM8       0x0A
#define TLC59116_PWM9       0x0B
#define TLC59116_PWM10      0x0C
#define TLC59116_PWM11      0x0D
#define TLC59116_PWM12      0x0E
#define TLC59116_PWM13      0x0F
#define TLC59116_PWM14      0x10
#define TLC59116_PWM15      0x11
#define TLC59116_GRPPWM     0x12
#define TLC59116_GRPFREQ    0x13
#define TLC59116_LEDOUT0    0x14
#define TLC59116_LEDOUT1    0x15
#define TLC59116_LEDOUT2    0x16
#define TLC59116_LEDOUT3    0x17
#define TLC59116_SUBADR1    0x18
#define TLC59116_SUBADR2    0x19
#define TLC59116_SUBADR3    0x1A
#define TLC59116_ALLCALLADR 0x1B
#define TLC59116_IREF       0x1C
#define TLC59116_EFLAG1     0x1D
#define TLC59116_EFLAG2     0x1E

/*! The TLC59116 library drives a TLC59116 PWM LED Driver chip over I2C.  It allows the control of each output via
 *  functions that work the same as the normal Arduinop analogWrite() function.  There is also a pin mapping facility
 *  for displaying numeric values on a 7 segment display.
 */

class TLC59116 {
    private:
        uint8_t _addr;
         uint8_t _begun;
        static const uint8_t pinmap[8];
        static const uint8_t numbers[10];

        const uint8_t *_currentPinMapping;

    public:
        /*! Create a new TLC59116 board with the default address of 0 */
        TLC59116();
        /*! Create a new TLC59116 board with the default provided address */
        TLC59116(uint8_t addr);
        /*! Initialize the board, set all channels to 0, and apply the default pin mapping */
        void begin();
        /*! Set the channel (0-15) to the given brightness value (0-255) */
        void analogWrite(uint8_t channel, uint8_t brightness);
        
        void analogWriteBatch(uint8_t start_channel, uint8_t *brightness, uint8_t length);

       void writeRegister(uint8_t reg, uint8_t val);

        /*! Display the given number at the specified brightness using the current pin mapping */
        void displayNumber(uint8_t number, uint8_t brightness);
        /*! Load a new pin mapping.
         *
         * Pin mappings are in the form of an 8-byte array with each byte representing the mapping for segments A to G and DP in that order.
         * The upper nibble (4 bits) of each byte is the analog channel number for the left-hand digit. The lower nibble is the channel number for the right-hand digit.
         */
        void setPinMapping(const uint8_t *mapping);
};
#endif
