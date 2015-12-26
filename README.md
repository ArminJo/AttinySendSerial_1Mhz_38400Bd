# AttinySendSerial_1Mhz_38400Bd
Minimal bit-bang send serial 38400 baud for 1 Mhz Attiny clock
### Perfect for debugging purposes
#### Tested on an Attiny85
#### Code size only 60 bytes
```
Compile with:
avr-g++ -I"\arduino\hardware\arduino\avr\cores\arduino" -I"E:\arduino\hardware\arduino\avr\variants\standard" -Wall -g3 -gstabs -Os -ffunction-sections -fdata-sections -fno-exceptions -mmcu=attiny85 -DF_CPU=1000000UL -MMD -MP -MF"src/UART_TINY_1MHz_38400.d" -MT"src/UART_TINY_1MHz_38400.d" -c -o "src/UART_TINY_1MHz_38400.o"  -x c++ "../src/UART_TINY_1MHz_38400.cpp"

