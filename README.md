Client for SpeedTest.net infrastructure written in pure C99 standard using only POSIX and OpenSSL libraries.

Main purpose for this project was to deliver client for Speedtest.net 
infrastructure for embedded devices.

All application is written in pure C99 standard (it should compile with C90
too), without using any external libraries - only POSIX and OpenSSL is used.

Code is not perfect, and it has a few bugs, but it is stable and it works.

Big thanks for Luke Graham for his http function.

To compile, just type make.

If you want to build it for FreshTomato Firmware, copy this repository into
release/<src-rt-6.x.4708 or src-rt, depends on platform>/router

and add following line somewhere at beginning of Makefile inside router directory:

obj-y += SpeedTestC
