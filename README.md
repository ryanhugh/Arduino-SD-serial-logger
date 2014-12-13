Arduino-SD-serial-logger
========================

A small arduino program to log data from the UART port 
to an SD card very quickly. Works at 115200 baud.

Files on the SD card are named LOG00000.TXT, LOG00001.TXT, etc
Each file is always 512,000 bytes, and the parts that have not been written to yet are all 0's

You can change the arduino's internal serial buffer at C:\Program Files (x86)\Arduino\hardware\arduino\cores\arduino\HardwareSerial.cpp
by changing the line "#define SERIAL_BUFFER_SIZE 64"
I had it set to 500


Adopted from the RawWrite.ino example found here:
https://github.com/greiman/FreeRTOS-Arduino/blob/master/libraries/SdFat/examples/RawWrite/RawWrite.ino

Also used code from OpenLog
https://github.com/sparkfun/OpenLog
