# Sub-$30 USB Display

This project aims to control a small 320x240 TFT display module over USB using a Cypress EZ-USB FX2LP development board.

It is a proof of concept right now.

# Circuit

For an overview of the 40-pin display module pinout, see:
http://www.rinkydinkelectronics.com/files/UTFT_Requirements.pdf

My display module has an SSD1289 controller and is marked "TFT_320QVT".


FX2LP Dev Board -> Display module (40-pin connector)

````
GND -> 1 (GND)
VCC -> 2 (VCC), 6 (RD), 19 (backlight)
CTL0 -> 5 (WR)
PA0 -> 17 (RESET)
PA1 -> 4  (RS aka data/command)
PA2 -> 15 (CS)
PB0 -> 21 (DB0)
PB1 -> 22 (DB1)
PB2 -> 23 (DB2)
PB3 -> 24 (DB3)
PB4 -> 25 (DB4)
PB5 -> 26 (DB5)
PB6 -> 27 (DB6)
PB7 -> 28 (DB7)
PD0 -> 7 (DB8)
PD1 -> 8 (DB9)
PD2 -> 9 (DB10)
PD3 -> 10 (DB11)
PD4 -> 11 (DB12)
PD5 -> 12 (DB13)
PD6 -> 13 (DB14)
PD7 -> 14 (DB15)
````

# Building the Software

Requirements:
You will need sdcc (the small device C compiler) in your PATH.
You will also need Python 3 and PyUSB.

I have only tested this on Arch Linux so far.

````
cd usb-display
make
````

To get it to compile on Arch Linux, I had to change the type signature of `putchar` and `getchar` in `fx2lib/include/serial.h` to the following:

````
void putchar(unsigned char c);
unsigned char getchar();
````

Attach your development board, run `./test.py` and see if it works. It might need several tries to get the timing right.

