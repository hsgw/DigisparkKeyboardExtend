/*
 * Modified by Takuya Urakawa Dm9Records.com
 * 
 * Based on Obdev's AVRUSB code and under the same license.
 *
 * TODO: Make a proper file header. :-)
 * Modified for Digispark by Digistump
 */
#ifndef __DigiKeyboard_h__
#define __DigiKeyboard_h__

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <string.h>

#include "usbdrv.h"
#include "keycode.h"
#include "sendstring.h"

// TODO: Work around Arduino 12 issues better.
//#include <WConstants.h>
//#undef int()

typedef uint8_t byte;


#define RO_SIZE 6 // 6 for keystroke 


static uchar    idleRate;           // in 4 ms units 


/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and but we do allow
 * simultaneous key presses. 
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */
const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor */
  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop) 
  0x09, 0x06,                    // USAGE (Keyboard) 
  0xa1, 0x01,                    // COLLECTION (Application) 
  0x05, 0x07,                    //   USAGE_PAGE (Keyboard) 
  0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl) 
  0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI) 
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0) 
  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1) 
  0x75, 0x01,                    //   REPORT_SIZE (1) 
  0x95, 0x08,                    //   REPORT_COUNT (8) 
  0x81, 0x02,                    //   INPUT (Data,Var,Abs) 
  0x95, 0x06,                    //   REPORT_COUNT (simultaneous keystrokes) 
  0x75, 0x08,                    //   REPORT_SIZE (8) 
  0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (0xFF) 
  0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated)) 
  0x29, 0x8c,                    //   USAGE_MAXIMUM (Keyboard Application) 
  0x81, 0x00,                    //   INPUT (Data,Ary,Abs) 
  0xc0                           // END_COLLECTION 
};

class DigiKeyboardDeviceExtends {
 public:
  DigiKeyboardDeviceExtends () {
    keyBufferPos = 1;
    cli();
    usbDeviceDisconnect();
    _delay_ms(250);
    usbDeviceConnect();


    usbInit();
      
    sei();

    // TODO: Remove the next two lines once we fix
    //       missing first keystroke bug properly.
    memset(reportBuffer, 0, sizeof(reportBuffer));      
    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
  }

  void update() {
    usbPoll();
  }

	// delay while updating until we are finished delaying
	void delay(long milli) {
		unsigned long last = millis();
	  while (milli > 0) {
	    unsigned long now = millis();
	    milli -= now - last;
	    last = now;
	    usbPoll();
	  }
	}

  // initialize keycode buffer.
  void initKeyCode(void) {
    memset(reportBuffer, 0, sizeof(reportBuffer));
    keyBufferPos = 1;
  }

  // register QMK style keycode, but don't send keycodes to pc
  void registerKeyCode(uint16_t keycode) {
    uchar mod = keycode >> 8;
    uchar key = keycode & 0xFF;
    reportBuffer[0] |= mod;
    if(key != 0) {
      if(keyBufferPos > RO_SIZE) keyBufferPos = 1;
      reportBuffer[keyBufferPos] = key;
      keyBufferPos++;
    }
  }

  // send registered keycodes
  void sendRegisteredKeyCode(void) {
    while (!usbInterruptIsReady()) {
      // Note: We wait until we can send keyPress
      //       so we know the previous keyPress was
      //       sent.
    	usbPoll();
    	_delay_ms(5);
    }
    
    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
  }
  
  //sendKeyStroke: sends a key press AND release
  void sendKeyStroke(uint16_t keycode) {
    initKeyCode();
    registerKeyCode(keycode);
    sendRegisteredKeyCode();
    initKeyCode();
    sendRegisteredKeyCode();
  }

  // send string
  // if you used JIS keymap, #define USE_JIS in your sketch
  void sendString(const char PROGMEM *str) {
    uint8_t last = 0;
    uint8_t c = pgm_read_byte(str);
    str++;
    while(c != 0) {
      initKeyCode();
      if(c == last) sendRegisteredKeyCode();
      if(pgm_read_byte(&ascii_to_shift_lut[c])) registerKeyCode(KC_LSFT);
      registerKeyCode(pgm_read_byte(&ascii_to_keycode_lut[c]));
      sendRegisteredKeyCode();
      last = c;
      c = pgm_read_byte(str);
      str++;
    }
    initKeyCode();
    sendRegisteredKeyCode();
  }
    
// private: //TODO: Make friend?
  uchar    reportBuffer[RO_SIZE+1];    // buffer for HID reports [ 1 modifier byte + (len-1) key strokes]
  uchar    keyBufferPos;
};

DigiKeyboardDeviceExtends kbd = DigiKeyboardDeviceExtends();

#ifdef __cplusplus
extern "C"{
#endif 
  // USB_PUBLIC uchar usbFunctionSetup
	uchar usbFunctionSetup(uchar data[8]) {
    usbRequest_t    *rq = (usbRequest_t *)((void *)data);

    usbMsgPtr = kbd.reportBuffer; //
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
      /* class request type */

      if (rq->bRequest == USBRQ_HID_GET_REPORT) {
				/* wValue: ReportType (highbyte), ReportID (lowbyte) */

				/* we only have one report type, so don't look at wValue */
        // TODO: Ensure it's okay not to return anything here?    
				return 0;

      } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
				//usbMsgPtr = &idleRate;
				//return 1;
				return 0;
				
      } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
				idleRate = rq->wValue.bytes[1];
				
      }
    } else {
      /* no vendor specific requests implemented */
    }
		
    return 0;
  }
#ifdef __cplusplus
} // extern "C"
#endif


#endif // __DigiKeyboard_h__
