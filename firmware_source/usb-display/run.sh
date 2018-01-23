#!/bin/bash
DEVLINE=$(lsusb | grep 'CY7C68013 EZ-USB FX2 USB 2.0 Development Kit' | head -n 1)
BUS=$(echo $DEVLINE | awk '{print $2}')
DEVICE=$(echo $DEVLINE | awk '{print $4}' | sed -e 's/://')
fxload -D /dev/bus/usb/$BUS/$DEVICE -I $1 -t fx2lp -v
