# Attiny Serial Out
Minimal bit-bang send serial 38400 baud for 1 Mhz or 230400 baud on 8 MHz Attiny clock
### Perfect for debugging purposes
#### Tested on an Attiny85
#### Code size only 60 bytes

Compile with Arduino standard settings or:
```
avr-g++ -I"\arduino\hardware\arduino\avr\cores\arduino" -I"E:\arduino\hardware\arduino\avr\variants\standard" -c -g -w -Os -ffunction-sections -fdata-sections -mmcu=attiny85 -DF_CPU=1000000UL -MMD -o "TinySerialOut.o" "TinySerialOut.cpp"
```
in order to guarantee the correct timing

