package main

import (
	"fmt"
	"image"
	"image/color"
	"log"
	"os"
	"strconv"
	"time"

	"github.com/google/gousb"
	"github.com/jboecker/go-fxload"
	"github.com/kbinani/screenshot"
)

type DisplayDevice struct {
	dev *gousb.Device
	ep2 *gousb.OutEndpoint
}

var display DisplayDevice

/*
CMD_SETPORTA=0xb0
CMD_RESET=0xb1
CMD_SINGLETRANSFER=0xb2
CMD_START=0xb3
CMD_STOP=0xb4
CMD_SETTXC=0xb5
CMD_PING=0xb6
*/

func lcd_write_frame(buf []byte) {
	lcd_resetptr()
	display.dev.Control(gousb.RequestTypeVendor, 0xb5, 0, 0, []byte{0, 44, 1, 0})
	display.ep2.Write(buf)
}

func LCD_Write_COM(cmd uint16) {
	var h, l uint8 = uint8(cmd >> 8), uint8(cmd & 0xff)
	_, err := display.dev.Control(gousb.RequestTypeVendor, 0xb2, 0, 0, []byte{0x00, l, h})
	if err != nil {
		log.Fatal(err.Error())
	}
}
func LCD_Write_DATA(data uint16) {
	var h, l uint8 = uint8(data >> 8), uint8(data & 0xff)
	display.dev.Control(gousb.RequestTypeVendor, 0xb2, 0, 0, []byte{0x01, l, h})
}
func LCD_Write_COM_DATA(cmd uint16, data uint16) {
	LCD_Write_COM(cmd)
	LCD_Write_DATA(data)
}
func lcd_resetptr() {
	LCD_Write_COM_DATA(0x4f, 0x0000)
	LCD_Write_COM_DATA(0x4e, 0x0000)
	LCD_Write_COM(0x22)
}
func lcd_init() {
	LCD_Write_COM_DATA(0x00, 0x0001)
	LCD_Write_COM_DATA(0x03, 0xA8A4)
	LCD_Write_COM_DATA(0x0C, 0x0000)
	LCD_Write_COM_DATA(0x0D, 0x080C)
	LCD_Write_COM_DATA(0x0E, 0x2B00)
	LCD_Write_COM_DATA(0x1E, 0x00B7)
	LCD_Write_COM_DATA(0x01, 0x2B3F)
	LCD_Write_COM_DATA(0x02, 0x0600) // AC Settings
	LCD_Write_COM_DATA(0x10, 0x0000) // exit sleep mode
	time.Sleep(100 * time.Millisecond)
	//LCD_Write_COM_DATA(0x11,0x6070)
	LCD_Write_COM_DATA(0x11, 0x6078)

	LCD_Write_COM_DATA(0x05, 0x0000)
	LCD_Write_COM_DATA(0x06, 0x0000)
	LCD_Write_COM_DATA(0x16, 0xEF1C)
	LCD_Write_COM_DATA(0x17, 0x0003)
	LCD_Write_COM_DATA(0x07, 0x0233)
	LCD_Write_COM_DATA(0x0B, 0x0000)
	LCD_Write_COM_DATA(0x0F, 0x0000)
	LCD_Write_COM_DATA(0x41, 0x0000)
	LCD_Write_COM_DATA(0x42, 0x0000)
	LCD_Write_COM_DATA(0x48, 0x0000)
	LCD_Write_COM_DATA(0x49, 0x013F)
	LCD_Write_COM_DATA(0x4A, 0x0000)
	LCD_Write_COM_DATA(0x4B, 0x0000)
	LCD_Write_COM_DATA(0x44, 0xEF00)
	LCD_Write_COM_DATA(0x45, 0x0000)
	LCD_Write_COM_DATA(0x46, 0x013F)
	LCD_Write_COM_DATA(0x30, 0x0707)
	LCD_Write_COM_DATA(0x31, 0x0204)
	LCD_Write_COM_DATA(0x32, 0x0204)
	LCD_Write_COM_DATA(0x33, 0x0502)
	LCD_Write_COM_DATA(0x34, 0x0507)
	LCD_Write_COM_DATA(0x35, 0x0204)
	LCD_Write_COM_DATA(0x36, 0x0204)
	LCD_Write_COM_DATA(0x37, 0x0502)
	LCD_Write_COM_DATA(0x3A, 0x0302)
	LCD_Write_COM_DATA(0x3B, 0x0302)
	LCD_Write_COM_DATA(0x23, 0x0000)
	LCD_Write_COM_DATA(0x24, 0x0000)
	LCD_Write_COM_DATA(0x25, 0x8000)
	lcd_resetptr()
}

func tryDownloadFirmware() error {
	ctx := gousb.NewContext()
	defer ctx.Close()

	dev, err := ctx.OpenDeviceWithVIDPID(0x04b4, 0x8613)
	if err != nil {
		return err
	}

	fmt.Println("found device, uploading firmware")
	err = fxload.DownloadFirmwareFile(dev, "firmware.ihx")

	return err
}

func main() {
	var left, top int
	if len(os.Args) == 3 {
		left, _ = strconv.Atoi(os.Args[1])
		top, _ = strconv.Atoi(os.Args[2])
	}

	ctx := gousb.NewContext()
	defer ctx.Close()

	fmt.Println("looking for dev board to upload to...")
	tryDownloadFirmware()

	fmt.Println("looking for USB device...")

	tstart := time.Now()

	var dev *gousb.Device
	var err error
	for {
		dev, err = ctx.OpenDeviceWithVIDPID(0x16c0, 0x27d8)
		display.dev = dev
		if err == nil {
			break
		}

		if time.Since(tstart) > 5*time.Second {
			log.Fatal("timed out while waiting for USB device")
		}
	}
	defer dev.Close()

	intf, done, err := dev.DefaultInterface()
	if err != nil {
		log.Fatalf("%s.DefaultInterface(): %v", dev, err)
	}
	defer done()

	ep2, err := intf.OutEndpoint(2)
	if err != nil {
		log.Fatalf("%s.OutEndpoint(2): %v", intf, err)
	}
	display.ep2 = ep2

	buf := make([]byte, 320*240*2, 320*240*2)

	for i := range buf {
		buf[i] = 0xFF
	}

	fmt.Print("resetting display...")
	dev.Control(gousb.RequestTypeVendor, 0xb1, 0, 0, nil)
	fmt.Print("lcd_init...")
	lcd_init()
	fmt.Print("writing frames...")

	for {
		rect := image.Rect(left, top, left+320, top+240)
		img, err := screenshot.CaptureRect(rect) // *image.RGBA
		if err != nil {
			log.Fatalln(err.Error())
		}

		lcd_write_frame(image_to_buffer(img))
	}

}

func image_to_buffer(img image.Image) []byte {
	buf := make([]byte, 320*240*2, 320*240*2)

	offset := 0
	for y := 0; y < 240; y++ {
		for x := 0; x < 320; x++ {
			rgb565 := rgbToRgb565(img.At(x, 239-y))
			var h, l uint8 = uint8(rgb565 >> 8), uint8(rgb565 & 0xff)
			buf[offset] = l
			buf[offset+1] = h
			offset += 2
		}
	}

	return buf
}

func rgbToRgb565(c color.Color) uint16 {
	r, g, b, a := c.RGBA()

	for a > 255 {
		a = a / 2
		r = r / 2
		g = g / 2
		b = b / 2
	}

	r = r >> 3
	g = g >> 2
	b = b >> 3

	return uint16((r << 11) | (g << 5) | b)

}
