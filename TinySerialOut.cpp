/*
 * TinySerialOut.cpp
 *
 *  Copyright (C) 2015  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *
 *  This file is part of TinySerialOut https://github.com/ArminJo/AttinySendSerial_1Mhz_38400Bd.
 *
 *  TinySerialOut is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 *
 * For transmitting debug data over bit bang serial with 38400 baud for a TINY running on 1 MHz
 * 1 Start, 8 Data, 1 Stop, No Parity
 * 26 cycles per bit => 260 cycles per byte needed for 38400 baud
 *
 * For transmitting debug data over bit bang serial with 230400 baud for a TINY running on 8 MHz
 * 34.722 cycles per bit => 350 cycles per byte needed for 230400 baud
 *
 * In order to guarantee the correct timing, compile with Arduino standard settings or:
 * avr-g++ -I"C:\arduino\hardware\arduino\avr\cores\arduino" -I"C:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
 *
 */

#include "TinySerialOut.h"

#include <avr/io.h>         // for PORTB - is also included by <avr/interrupt.h>
#include <stdlib.h>         // for utoa() etc.

#ifndef _NOP
#define _NOP()  __asm__ volatile ("nop")
#endif

#ifndef PORTB
#define PORTB (*(volatile uint8_t *)((0x18) + 0x20))
#endif

/*
 * Used for writeStringSkipLeadingSpaces() and therefore all write<type>()
 */
bool sUseCliSeiForStrings = false;
void useCliSeiForStrings(bool aUseCliSeiForStrings) {
    sUseCliSeiForStrings = aUseCliSeiForStrings;
}

/*
 * Only multiple of 4 cycles are possible. Last loop is only 3 cycles.
 * 3 -> 11 cycles
 * 4 -> 15 cycles
 * 5 -> 19 cycles
 * 6 -> 23 cycles
 */
inline void delay4CyclesInlineExact(uint16_t a4Microseconds) {
    // the following loop takes 4 cycles (4 microseconds  at 1MHz) per iteration
    __asm__ __volatile__ (
            "1: sbiw %0,1" "\n\t"    // 2 cycles
            "brne .-4" : "=w" (a4Microseconds) : "0" (a4Microseconds)// 2 cycles
    );
}

#if (F_CPU == 1000000)
/*
 * 26 cycles per bit, 260 per byte for 38400 baud
 * 24 cycles between each cbi (Clear Bit in Io-register) or sbi command
 */
void write1Start8Data1StopNoParity(uint8_t aChar) {
    // start bit
    PORTB &= ~(1 << TX_PIN);
    _NOP();
    delay4CyclesInlineExact(4);

    // 8 data bits
    uint8_t i = 8;
    do {
        if (aChar & 0x01) {
            // bit=1
            // to compensate for jump at data=0
            _NOP();
            PORTB |= 1 << TX_PIN;
        } else {
            // bit=0
            PORTB &= ~(1 << TX_PIN);
            // compensate for different cycles of sbrs
            _NOP();
            _NOP();
        }
        aChar = aChar >> 1;
        // 3 cycles padding
        _NOP();
        _NOP();
        _NOP();
        delay4CyclesInlineExact(3);
        --i;
    } while (i > 0);

    // to compensate for missing loop cycles at last bit
    _NOP();
    _NOP();
    _NOP();
    _NOP();

    // Stop bit
    PORTB |= 1 << TX_PIN;
    // -8 cycles to compensate for fastest repeated call (1 ret + 1 load + 1 call)
    delay4CyclesInlineExact(4);            // gives minimum 25 cycles for stop bit :-)
}
#endif

#if (F_CPU == 8000000)
/*
 * 35 cycles per bit, 350 per byte for 230400 baud at 8MHz Clock
 * 33 cycles between each cbi (Clear Bit in Io-register) or sbi command
 */
void write1Start8Data1StopNoParity(uint8_t aChar) {
    // start bit
    PORTB &= ~(1 << TX_PIN);
    _NOP();
    _NOP();
    delay4CyclesInlineExact(6);

    // 8 data bits
    uint8_t i = 8;
    do {
        if (aChar & 0x01) {
            // bit=1
            // to compensate for jump at data=0
            _NOP();
            PORTB |= 1 << TX_PIN;
        } else {
            // bit=0
            PORTB &= ~(1 << TX_PIN);
            // compensate for different cycles of sbrs
            _NOP();
            _NOP();
        }
        aChar = aChar >> 1;
        delay4CyclesInlineExact(6);
        --i;
    }while (i > 0);

    // to compensate for missing loop cycles at last bit
    _NOP();
    _NOP();
    _NOP();
    _NOP();

    // Stop bit
    PORTB |= 1 << TX_PIN;
    // -8 cycles to compensate for fastest repeated call (1 ret + 1 load + 1 call)
    delay4CyclesInlineExact(6);// gives minimum 33 cycles for stop bit :-)
}
#endif

void writeString(const char * aStringPtr) {
    if (sUseCliSeiForStrings) {
        while (*aStringPtr != 0) {
            cli();
            write1Start8Data1StopNoParityWithCliSei(*aStringPtr++);
            sei();
        }
    } else {
        while (*aStringPtr != 0) {
            write1Start8Data1StopNoParity(*aStringPtr++);
        }
    }
}

void writeStringWithoutCliSei(const char * aStringPtr) {
    while (*aStringPtr != 0) {
        write1Start8Data1StopNoParity(*aStringPtr++);
    }
}

void writeStringWithCliSei(const char * aStringPtr) {
    while (*aStringPtr != 0) {
        cli();
        write1Start8Data1StopNoParity(*aStringPtr++);
        sei();
    }
}

void writeStringSkipLeadingSpaces(const char * aStringPtr) {
    // skip leading spaces
    while (*aStringPtr == ' ' && *aStringPtr != 0) {
        aStringPtr++;
    }
    if (sUseCliSeiForStrings) {
        while (*aStringPtr != 0) {
            cli();
            write1Start8Data1StopNoParity(*aStringPtr++);
            sei();
        }
    } else {
        while (*aStringPtr != 0) {
            write1Start8Data1StopNoParity(*aStringPtr++);
        }
    }
}

void writeUnsignedByte(uint8_t aByte) {
    char tStringBuffer[4];
    utoa(aByte, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeUnsignedByteHex(uint8_t aByte) {
    char tStringBuffer[5];
    tStringBuffer[0] = '0';
    tStringBuffer[1] = 'x';
    utoa(aByte, &tStringBuffer[2], 16);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeByte(int8_t aByte) {
    char tStringBuffer[5];
    itoa(aByte, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeInt(int aInteger) {
    char tStringBuffer[7];
    itoa(aInteger, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeUnsignedInt(unsigned int aInteger) {
    char tStringBuffer[6];
    itoa(aInteger, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeLong(long aLong) {
    char tStringBuffer[12];
    ltoa(aLong, tStringBuffer, 10);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

void writeFloat(double aFloat) {
    char tStringBuffer[11];
    dtostrf(aFloat, 10, 3, tStringBuffer);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

/*
 * Generated Assembler for write1Start8Data1StopNoParity() - 1MHz version.
 * Check with .lss file created with: avr-objdump -h -S MyProgram.elf  >"MyProgram.lss".
 *
 c4 98           cbi 0x18, 4
 1 00 00           nop
 2 eb e0           ldi r30, 0x04
 3 f0 e0           ldi r31, 0x00
 4+5 31 97           sbiw    r30, 0x01
 6 + n*4 f1 f7           brne    .-4
 7 98 e0           ldi r25, 0x08
 8 2a e0           ldi r18, 0x03
 9 30 e0           ldi r19, 0x00

 Start of loop
 if (aChar & 0x01) {
 1 80 ff           sbrs    r24, 0
 2+3 02 c0           rjmp    .+6
 PORTB |= 1 << TX_PIN;
 3 00 00           nop
 4+5 c4 9a           sbi 0x18, 4
 6+7 02 c0           rjmp    .+6
 PORTB &= ~(1 << TX_PIN);
 4+5 c4 98           cbi 0x18, 4
 6 00 00           nop
 7 00 00           nop

 8 86 95           lsr r24
 9 00 00           nop
 10 00 00           nop
 11 00 00           nop
 12 f9 01           movw    r30, r18
 13+14 31 97           sbiw    r30, 0x01
 15 f1 f7           brne    .-4
 16 91 50           subi    r25, 0x01
 17+18 a1 f7           brne    .-34

 To compensate for missing loop cycles at last bit
 00 00           nop
 00 00           nop
 00 00           nop
 00 00           nop

 Stop bit
 c4 9a           sbi 0x18, 4
 8a e0           ldi r24, 0x04
 90 e0           ldi r25, 0x00
 01 97           sbiw    r24, 0x01
 f1 f7           brne    .-4

 08 95           ret

 *
 */
