/*
 * UART_TINY_1MHz_38400.cpp
 *
 *  Created on: 28.11.2015
 *  Author: Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *  License: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *
 * For transmitting debug data over bit bang serial with 38400 baud for a TINY running on 1 MHz
 * 1 Start, 8 Data, 1 Stop, No Parity
 * 26 cycles per bit => 260 cycles per byte needed for 38400 baud
 *
 * Compile with
 * avr-g++ -I"\arduino\hardware\arduino\avr\cores\arduino" -I"E:\arduino\hardware\arduino\avr\variants\standard" -Wall -g3 -gstabs -Os -ffunction-sections -fdata-sections -fno-exceptions -mmcu=attiny85 -DF_CPU=1000000UL -MMD -MP -MF"src/UART_TINY_1MHz_38400.d" -MT"src/UART_TINY_1MHz_38400.d" -c -o "src/UART_TINY_1MHz_38400.o"  -x c++ "../src/UART_TINY_1MHz_38400.cpp"
 * to reproduce timing
 *
 */

#include  "UART_TINY_1MHz_38400.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#ifndef _NOP
#define _NOP()  __asm__ volatile ("nop")
#endif

/*
 * Only multiple of 4 cycles are possible. Last loop is only 3 cycles.
 * 3 -> 11 cycles
 * 4 -> 15 cycles
 */
inline void delay4MicrosecondsInlineExact(uint16_t a4Microseconds) {
    // the following loop takes 4 cycles (4 microseconds  at 1MHz) per iteration
    __asm__ __volatile__ (
            "1: sbiw %0,1" "\n\t"    // 2 cycles
            "brne .-4" : "=w" (a4Microseconds) : "0" (a4Microseconds)// 2 cycles
    );
}

/*
 * 26 cycles per bit, 260 per byte for 38400 baud
 * 24 cycles between each cbi or sbi command
 */
void write1Start8Data1StopNoParity(uint8_t aChar) {
    // start bit
    PORTB &= ~(1 << TX_PIN);
    _NOP();
    delay4MicrosecondsInlineExact(4);

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
        delay4MicrosecondsInlineExact(3);
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
    delay4MicrosecondsInlineExact(4); // gives minimum 25 cycles for stop bit :-)
}

void writeString(const char * aStringPtr) {
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
    while (*aStringPtr != 0) {
        write1Start8Data1StopNoParity(*aStringPtr++);
    }
}

void writeUnsignedByte(uint8_t aByte) {
    char tStringBuffer[4];
    utoa(aByte, tStringBuffer, 10);
    writeString(tStringBuffer);
}

void writeByte(int8_t aByte) {
    char tStringBuffer[5];
    itoa(aByte, tStringBuffer, 10);
    writeString(tStringBuffer);
}

void writeInt(int aInteger) {
    char tStringBuffer[7];
    itoa(aInteger, tStringBuffer, 10);
    writeString(tStringBuffer);
}

void writeLong(long aLong) {
    char tStringBuffer[12];
    ltoa(aLong, tStringBuffer, 10);
    writeString(tStringBuffer);
}

void writeFloat(double aFloat) {
    char tStringBuffer[11];
    dtostrf(aFloat, 10, 3, tStringBuffer);
    writeStringSkipLeadingSpaces(tStringBuffer);
}

/*
 * Generated Assembler for writeByte().
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
