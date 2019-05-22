// If use JIS keymap and sendString(), uncomment below.
// #define USE_JIS

#include "DigiKeyboardExt.h"
#include "keycode.h"

const PROGMEM char message[] = "Hello World!\n";

void setup() {
  // don't need to set anything up to use DigiKeyboard
}

void loop() {
//  kbd.sendKeyStroke(KC_A);
//  kbd.sendKeyStroke(LCTRL(KC_S));
  kbd.sendString(message);

//  // register keycodes, up to 6keys roll over.
//  kbd.registerKeyCode(KC_1);
//  kbd.registerKeyCode(KC_2);
//  kbd.registerKeyCode(KC_3);
//  // send keycodes to pc
//  kbd.sendRegisteredKeyCode();
//
//  // release all
//  kbd.initKeyCode(); 
//  kbd.sendRegisteredKeyCode(); 
  
  // It's better to use kbd.delay() over the regular Arduino delay()
  // if doing keyboard stuff because it keeps talking to the computer to make
  // sure the computer knows the keyboard is alive and connected
  kbd.delay(1000);
}
