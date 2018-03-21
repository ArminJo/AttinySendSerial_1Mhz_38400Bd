# Attiny Serial Out
Minimal bit-bang send serial 38400 or 115200 baud for 1 Mhz or 230400 baud on 8 MHz Attiny clock
### Perfect for debugging purposes
#### Tested on an Attiny85 with Arduino 1.6.8 / avr-gcc-4.8.1 and Arduino 1.8.5 / avr-gcc-4.9.2.
#### Code size only 76 Bytes@38400Bd or 196 Bytes@115200Bd (including first call)

In order to guarantee the correct timing, compile with Arduino standard settings or:
```
avr-g++ -I"C:\arduino\hardware\arduino\avr\cores\arduino" -I"C:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
```
**!!!! C functions (38400 and 230400) does not work with avr-gcc-7.3.0 !!!** since optimization is too bad.
