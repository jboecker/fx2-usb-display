#!/usr/bin/env python3

import sys
import os
import time
import usb
import random
import struct
from PIL import Image


VID=0x16c0
PID=0x27d8

DEBUG_DISPLAYPROTOCOL = False

CMD_SETPORTA=0xb0
CMD_RESET=0xb1
CMD_SINGLETRANSFER=0xb2
CMD_START=0xb3
CMD_STOP=0xb4
CMD_SETTXC=0xb5
CMD_PING=0xb6


OUT=0x40
IN=0xc0


print("waiting for USB device...")
while not dev:
    dev = usb.core.find(idVendor=VID, idProduct=PID)

def init_lcd():
    LCD_Write_COM_DATA(0x00,0x0001)
    LCD_Write_COM_DATA(0x03,0xA8A4)
    LCD_Write_COM_DATA(0x0C,0x0000)
    LCD_Write_COM_DATA(0x0D,0x080C)
    LCD_Write_COM_DATA(0x0E,0x2B00)
    LCD_Write_COM_DATA(0x1E,0x00B7)
    LCD_Write_COM_DATA(0x01,0x2B3F)
    LCD_Write_COM_DATA(0x02,0x0600) # AC Settings
    LCD_Write_COM_DATA(0x10,0x0000) # exit sleep mode
    time.sleep(0.1)
    #LCD_Write_COM_DATA(0x11,0x6070)
    LCD_Write_COM_DATA(0x11, 0x6078)

    LCD_Write_COM_DATA(0x05,0x0000)
    LCD_Write_COM_DATA(0x06,0x0000)
    LCD_Write_COM_DATA(0x16,0xEF1C)
    LCD_Write_COM_DATA(0x17,0x0003)
    LCD_Write_COM_DATA(0x07,0x0233)
    LCD_Write_COM_DATA(0x0B,0x0000)
    LCD_Write_COM_DATA(0x0F,0x0000)
    LCD_Write_COM_DATA(0x41,0x0000)
    LCD_Write_COM_DATA(0x42,0x0000)
    LCD_Write_COM_DATA(0x48,0x0000)
    LCD_Write_COM_DATA(0x49,0x013F)
    LCD_Write_COM_DATA(0x4A,0x0000)
    LCD_Write_COM_DATA(0x4B,0x0000)
    LCD_Write_COM_DATA(0x44,0xEF00)
    LCD_Write_COM_DATA(0x45,0x0000)
    LCD_Write_COM_DATA(0x46,0x013F)
    LCD_Write_COM_DATA(0x30,0x0707)
    LCD_Write_COM_DATA(0x31,0x0204)
    LCD_Write_COM_DATA(0x32,0x0204)
    LCD_Write_COM_DATA(0x33,0x0502)
    LCD_Write_COM_DATA(0x34,0x0507)
    LCD_Write_COM_DATA(0x35,0x0204)
    LCD_Write_COM_DATA(0x36,0x0204)
    LCD_Write_COM_DATA(0x37,0x0502)
    LCD_Write_COM_DATA(0x3A,0x0302)
    LCD_Write_COM_DATA(0x3B,0x0302)
    LCD_Write_COM_DATA(0x23,0x0000)
    LCD_Write_COM_DATA(0x24,0x0000)
    LCD_Write_COM_DATA(0x25,0x8000)
    lcd_resetptr()

def lcd_resetptr():
    LCD_Write_COM_DATA(0x4f,0x0000)
    LCD_Write_COM_DATA(0x4e,0x0000)
    LCD_Write_COM(0x22)

def pil_image_to_buffer(im):
    buf = bytearray(320*240*2)
    offset = 0
    for y in range(240):
        for x in range(320):
            c = rgb(*im.getpixel((x, 239-y)))
            buf[offset] = lo(c)
            buf[offset+1] = hi(c)
            offset += 2
            #LCD_Write_DATA(rgb(*im.getpixel((x, 239-y))))
    assert len(buf) / 512 == 300
    return buf

def image_to_buffer(filename):
    image_filename = os.path.join("images", filename)
    bin_filename = os.path.join("images_bin", filename)
    if not os.path.isdir("images_bin"):
        os.path.mkdir("images_bin")
    if not os.path.isfile(bin_filename):
        im = Image.open(image_filename)
        buf = pil_image_to_buffer(im)

        with open(bin_filename, "wb") as f:
            f.write(buf)

    with open(bin_filename, "rb") as f:
        return f.read()
    

def lcd_write_frame(buf, skipreset=False):
    assert len(buf) / 512 == 300

    if not skipreset:
        lcd_resetptr()
    # 320x240 = 76800 (1 16-bit transaction per pixel)
    dev.ctrl_transfer(OUT, CMD_SETTXC, 0, 0, struct.pack("<L", 76800))

    while len(buf) > 0:
        written = dev.write(2, buf)
        buf = buf[written:]
        


def cycle_images():
    images = os.listdir("images")
    images = list(map(image_to_buffer, images))
    random.shuffle(images)
    
    total_buffer = b''.join(images)
    total_lines = len(total_buffer) // (320*2)
    frame_len_bytes = 320*240*2
    total_buffer += images[0] # loop back to first image

    first = True

    framecount = 0
    start_time = time.time()
    while 1:
        for start_line in range(total_lines):
            start_offset = start_line * (320*2)
            lcd_write_frame(total_buffer[start_offset:start_offset+frame_len_bytes], not first)
            first = False

            framecount += 1
            elapsed_sec = (time.time() - start_time)
            print("%.2f fps" % (framecount / elapsed_sec))


def video(devicefile):
    target_fps = 29.97
    target_fps = 60
    import cv2

    camera = cv2.VideoCapture(devicefile)
    camera.set(3, 320) # height
    camera.set(4, 240) # width

    tstart = time.time()
    framecount = 0
    while 1:
        frame = None
        while 1:
            frame = camera.read()
            if frame[0]:
                break
        dataBGR = frame[1] # numpy array
        dataBGR = cv2.resize(dataBGR, (320, 240))
        data565 = cv2.cvtColor(dataBGR, cv2.COLOR_BGR2BGR565)
        buf = bytearray(data565)

        # flip vertically
        offset = 320*240*2
        for i in range(0, 240//2):
            row_a = i
            row_b = 239 - i

            row_a_offset = 320*row_a*2
            row_b_offset = 320*row_b*2

            row_a_data = bytes(buf[row_a_offset:row_a_offset+320*2])
            row_b_data = bytes(buf[row_b_offset:row_b_offset+320*2])

            buf[row_a_offset:row_a_offset+320*2] = row_b_data
            buf[row_b_offset:row_b_offset+320*2] = row_a_data

        lcd_write_frame(buf)
        framecount += 1

        t_target = tstart + (framecount/target_fps)
        while(t_target > time.time()):
            pass

        print("fps: %.2f" % (framecount/(time.time() - tstart)))



def share_screen():
    import PyQt4
    from PyQt4.QtGui import QPixmap, QApplication, QImage
    from PyQt4.Qt import QBuffer, QIODevice
    import io

    app = QApplication(sys.argv)
    def getScreenByQt():
        
        buffer = QBuffer()
        buffer.open(QIODevice.ReadWrite)
        img = QPixmap.grabWindow(QApplication.desktop().winId()).toImage().copy(0,0,320,240).convertToFormat(QImage.Format_RGB16)

        buf = bytearray(img.bits().asarray(320*240*2))
        
        # flip vertically
        offset = 320*240*2
        for i in range(0, 240//2):
            row_a = i
            row_b = 239 - i

            row_a_offset = 320*row_a*2
            row_b_offset = 320*row_b*2

            row_a_data = bytes(buf[row_a_offset:row_a_offset+320*2])
            row_b_data = bytes(buf[row_b_offset:row_b_offset+320*2])

            buf[row_a_offset:row_a_offset+320*2] = row_b_data
            buf[row_b_offset:row_b_offset+320*2] = row_a_data
        return buf

    while 1:
        t_a = time.time()
        buf = getScreenByQt()
        t_b = time.time()
        lcd_write_frame(buf)
        t_c = time.time()

        print("%.2f  %.2f" % (
            t_b - t_a, t_c - t_b))
        

def lo(word):
    return word & 0xFF

def hi(word):
    return (word & 0xFF00) >> 8

cmd_meaning = {
    0x00: "Oscillation Start",
    0x01: "Driver output",
    0x02: "LCD drive AC control",
    0x03: "Power control (1)",
    0x05: "Compare register (1)",
    0x06: "Compare register (2)",
    0x07: "Display control",
    0x0B: "Frame cycle control",
    0x0C: "Power control (2)",
    0x0D: "Power control (3)",
    0x0E: "Power control (4)",
    0x0F: "Gate scan start position",
    0x10: "Sleep mode",
    0x11: "Entry mode",
    0x12: "Optimize access speed 3",
    0x15: "Generic interface control",
    0x16: "Horizontal porch",
    0x17: "Vertical porch",
    0x1E: "Power control (5)",
    0x22: "RAM data write",
    0x23: "RAM write data mask (1)",
    0x24: "RAM write data mask (2)",
    0x25: "Frame Frequency",

    0x30: "Gamma control (1)",
    0x31: "Gamma control (2)",
    0x32: "Gamma control (3)",
    0x33: "Gamma control (4)",
    0x34: "Gamma control (5)",
    0x35: "Gamma control (6)",
    0x36: "Gamma control (7)",
    0x37: "Gamma control (8)",
    0x3A: "Gamma control (9)",
    0x3B: "Gamma control (10)",
    
    0x41: "Vertical scroll control (1)",
    0x42: "Vertical scroll control (2)",
    0x44: "Horizontal RAM address position",
    0x45: "Vertical RAM address start position",
    0x46: "Vertical RAM address end position",
    0x48: "First window start",
    0x49: "First window end",

    0x4A: "Second window start",
    0x4B: "Second window end",

    0x4E: "Set GDDRAM X address counter",
    0x4F: "Set GDDRAM Y address counter",

    
}

def LCD_Write_COM_DATA(cmd, data):
    dev.ctrl_transfer(OUT, CMD_SINGLETRANSFER, 0, 0, [0x00, lo(cmd), hi(cmd)])
    dev.ctrl_transfer(OUT, CMD_SINGLETRANSFER, 0, 0, [0x01, lo(data), hi(data)])
    if DEBUG_DISPLAYPROTOCOL:
        print("COM_DATA %02X  %02X  %s" % (cmd, data, cmd_meaning.get(cmd, "")))
    

def LCD_Write_COM(cmd):
    dev.ctrl_transfer(OUT, CMD_SINGLETRANSFER, 0, 0, [0x00, lo(cmd), hi(cmd)])
    if DEBUG_DISPLAYPROTOCOL:
        print("COM %02X  %s" % (cmd, cmd_meaning.get(cmd, "")))

def LCD_Write_DATA(data):
    dev.ctrl_transfer(OUT, CMD_SINGLETRANSFER, 0, 0, [0x01, lo(data), hi(data)])


def blink():
    c = 0
    while True:
        dev.ctrl_transfer(OUT, CMD_SETPORTA, 0, 0, [c])
        c = (c+1) % 4

def rgb(r, g, b):
    r = r >> 3
    g = g >> 2
    b = b >> 3
    return (r << 11) | (g << 5) | b

time.sleep(0.2)

# clear /RESET
dev.ctrl_transfer(IN, CMD_START, 0, 0, 1)
time.sleep(0.0015)
# set /RESET
dev.ctrl_transfer(IN, CMD_STOP, 0, 0, 1)
# init
init_lcd()

print("Init LCD done.")
#share_screen()

if len(sys.argv) > 1:
    # play the video file passed as parameter
    video(sys.argv[1])
else:
    # or scroll a few example images
    cycle_images()

