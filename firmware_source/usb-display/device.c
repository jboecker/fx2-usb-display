/**
 * Copyright (C) 2009 Ubixum, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/


#include <fx2macros.h>
#include <gpif.h>
#include <delay.h>

#define xdata __xdata

//extern const char xdata WaveData[128];
//extern const char xdata FlowStates[36];
//extern const char xdata InitData[7];
#include "gpifsetup_fx2lib.c"

#ifdef DEBUG_FIRMWARE
#include <stdio.h>
#else
#define printf(...)
#endif


BYTE vendor_command = 0;

BOOL handle_get_descriptor() {
return FALSE;
}



//************************** Configuration Handlers *****************************

// change to support as many interfaces as you need
//volatile xdata BYTE interface=0;
//volatile xdata BYTE alt=0; // alt interface

// set *alt_ifc to the current alt interface for ifc
BOOL handle_get_interface(BYTE ifc, BYTE* alt_ifc) {
// *alt_ifc=alt;
 return TRUE;
}
// return TRUE if you set the interface requested
// NOTE this function should reconfigure and reset the endpoints
// according to the interface descriptors you provided.
BOOL handle_set_interface(BYTE ifc,BYTE alt_ifc) {  
 printf ( "Set Interface.\n" );
 //interface=ifc;
 //alt=alt_ifc;
 return TRUE;
}

// handle getting and setting the configuration
// 1 is the default.  If you support more than one config
// keep track of the config number and return the correct number
// config numbers are set int the dscr file.
//volatile BYTE config=1;
BYTE handle_get_configuration() { 
 return 1;
}

// NOTE changing config requires the device to reset all the endpoints
BOOL handle_set_configuration(BYTE cfg) { 
 printf ( "Set Configuration.\n" );
 //config=cfg;
 return TRUE;
}


//******************* VENDOR COMMAND HANDLERS **************************


#define CMD_SETPORTA 0xb0
#define CMD_RESET 0xb1
#define CMD_SINGLETRANSFER 0xb2
#define CMD_START 0xb3
#define CMD_STOP 0xb4
#define CMD_SETTXC 0xb5
#define CMD_PING 0xb6

BOOL handle_vendorcommand(BYTE cmd) {
 // your custom vendor handler code here..

    if (vendor_command != 0) {
        return FALSE; // busy
    }

    if (cmd == CMD_SETPORTA) {
        vendor_command = CMD_SETPORTA;
        EP0BCH = 0;
        EP0BCL = 1;
        return TRUE;
    }

    if (cmd == CMD_RESET) {
        vendor_command = CMD_RESET;
        return TRUE;
    }

    if (cmd == CMD_START) {
        vendor_command = CMD_START;
        return TRUE;
    }

    if (cmd == CMD_STOP) {
        vendor_command = CMD_STOP;
        return TRUE;
    }

    if (cmd == CMD_SETTXC) {
        vendor_command = CMD_SETTXC;
        EP0BCH = 0;
        EP0BCL = 4; // expect 32-bit transaction count
        return TRUE;
    }

    if (cmd == CMD_SINGLETRANSFER) {
        vendor_command = CMD_SINGLETRANSFER;
        EP0BCH = 0;
        EP0BCL = 3; // expect control byte and data word
        return TRUE;
    }

    if (cmd == CMD_PING) {
        vendor_command = CMD_PING;
        EP0BCH = 0;
        EP0BCL = 1;
        return TRUE;
    }

    return FALSE; // not handled by handlers
}


//********************  INIT ***********************

void main_init() {

vendor_command = 0;

    OEA=0xFF; IOA=0x01;
    IOA |= (1<<0); // set /RESET
    IOA &= ~(1<<2); // clear /CS

    REVCTL=3;
    SYNCDELAY16;
    //SETIF48MHZ();
    
    

    EP2CFG = 0xA0; // OUPUT, BULK, SIZE=512, quad buffered
    SYNCDELAY16;
    FIFORESET = 0x80;
    SYNCDELAY16;
    FIFORESET = 0x82;
    SYNCDELAY16;
    FIFORESET = 0x00;
    SYNCDELAY16;

    EP2FIFOCFG = /* bmAUTOOUT | */ bmWORDWIDE;
    SYNCDELAY16;
    EP2GPIFFLGSEL = 1; // fifo empty flag

    OUTPKTEND = 0x82;
    SYNCDELAY16;
    OUTPKTEND = 0x82;
    SYNCDELAY16;
    OUTPKTEND = 0x82;
    SYNCDELAY16;
    OUTPKTEND = 0x82;
    SYNCDELAY16;
    



    EP2FIFOCFG |= bmAUTOOUT;
    SYNCDELAY16;

    // config gpif
    gpif_init(WaveData, InitData);
    //OEC=0xFF;
    //OEB=0xFF;
    //OED=0xFF;

    //IFCFG &= ~(1<<5); // turn off IFCLK output

    
    printf ( "Initialization Done.\n" );

}


void main_loop() {
    
    if (vendor_command == CMD_SETPORTA) {
        if ((EP0CS & bmEPBUSY) == 0) {
            IOA = EP0BUF[0];
            vendor_command = 0;
        }
    }
    
    if (vendor_command == CMD_RESET) {

        GPIFABORT = 0xFF;

        IOA &= ~(1<<0); // clear /RESET
        delay(1);
        IOA |= (1<<0); // set /RESET

        // ack
        EP0BUF[0] = 42;
        EP0BCH = 0;
        EP0BCL = 1;
        vendor_command = 0;
    }
    
    if (vendor_command == CMD_START) {
        IOA &= ~(1<<0); // clear /RESET
        
        // ack
        EP0BUF[0] = 42;
        EP0BCH = 0;
        EP0BCL = 1;
        vendor_command = 0;
    }

    if (vendor_command == CMD_STOP) {
        IOA |= (1<<0); // set /RESET

        // ack
        EP0BUF[0] = 42;
        EP0BCH = 0;
        EP0BCL = 1;
        vendor_command = 0;
    }
    
    if (vendor_command == CMD_SETTXC) {
        if ((EP0CS & bmEPBUSY) == 0) {
            unsigned long txc = *((unsigned long*)EP0BUF);

            while (! GPIFDONE);

            IOA |= (1<<1); // data
            gpif_set_tc32(txc);
            gpif_fifo_write(GPIF_EP2);
        
            vendor_command = 0;
        }
    }
    
    if (vendor_command == CMD_SINGLETRANSFER) {
        if ((EP0CS & bmEPBUSY) == 0) {

            if (EP0BUF[0] == 0) {
                IOA &= ~(1<<1); // command
            } else {
                IOA |= (1<<1); // data
            }
            GPIFABORT = 0xFF;
            
            gpif_single_write16(&EP0BUF[1], 1);
            while (! GPIFDONE);

            vendor_command = 0; 
        }
    }

    if (vendor_command == CMD_PING) {
        if ((EP0CS & bmEPBUSY) == 0) {
            EP0BUF[0]++;

            EP0BCH = 0;
            EP0BCL = 1; // send the byte of data back to the host

            vendor_command = 0;
        }
    }
}





/* GPIF config plan

IFCFG[1:0] = 10   // switch from Ports mode to GPIF mode
EPxFIFOCFG[0] = 1 // WORDWIDE: enable 16-bit mode (default)
// transfer data over USB in litle endian byte order
IFCONFIG[7] = 1 (internal IFCLK, default)
IFCONFIG[6] = 0 (30 MHz) or 1 (48 MHz)
IFCONFIG[5] = output enable for internal clock source
IFCONFIG[4] = 1 to invert IFCLK

CTL assignments:
CTL0 = C_/D  (UTFT RS) 0=data 1=command
CTL1 = WR (pull low to trigger write cycle)

SOME GPIO = RESET

GPIFIDLECS.0 = 0 (tri-state data bus when idle)
GPIFCTLCFG.7 = 0 (no tristating of control outputs)
GPIFIDLECTL[5:0] = 11111 (control outputs are high in idle state)
GPIFCTLCFG[5:0] = 00000 (CMOS instead of open-drain output)
GPIFREADYCFG.5 = 1 (use TCexpire signal)
*/