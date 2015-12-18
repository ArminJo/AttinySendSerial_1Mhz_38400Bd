#ifndef UART_TINY_1MHz_38400_H_
#define UART_TINY_1MHz_38400_H_

#include <stdint.h>

#ifndef TX_PIN
#define TX_PIN     1 // PB1
#endif

void write1Start8Data1StopNoParity(uint8_t aChar);
void writeString(const char * aStringPtr);
void writeStringWithCliSei(const char * aStringPtr);

void writeUnsignedByte(uint8_t aByte);
void writeByte(int8_t aByte);
void writeInt(int aInteger);
void writeLong(long aLong);
void writeFloat(double aFloat);

#endif /* UART_TINY_1MHz_38400_H_ */
