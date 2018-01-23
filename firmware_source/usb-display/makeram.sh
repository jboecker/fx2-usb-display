#!/bin/bash
python2 ../fx2lib/utils/gpif2dat gpifsetup.c gpifsetup_fx2lib.c
touch device.c; make ram && ./test.py
