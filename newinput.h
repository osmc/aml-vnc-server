#ifndef NEWINPUT_H
#define NEWINPUT_H

#include <linux/input.h>
#include <linux/uinput.h>

#include <rfb/rfb.h>
#include <rfb/keysym.h>

#include "common.h"

#define KEY_SOFT1 KEY_UNKNOWN
#define KEY_SOFT2 KEY_UNKNOWN
#define KEY_CENTER KEY_UNKNOWN
#define KEY_SHARP KEY_UNKNOWN
#define KEY_STAR KEY_UNKNOWN

#define BTN_LEFT_MASK 0x1
#define BTN_MIDDLE_MASK 0x2
#define BTN_RIGHT_MASK 0x4
#define WHEEL_UP_MASK 0x8
#define WHEEL_DOWN_MASK 0x10

void initVirtKbd();
void initVirtPtr();
void closeVirtKbd();
void closeVirtPtr();
void writeEvent(int udev, __u16 type, __u16 code, __s32 value);
int keysym2scancode(rfbKeySym key);
void dokey(rfbBool down, rfbKeySym key, rfbClientPtr cl);
void doptr(int buttonMask, int x, int y, rfbClientPtr cl);

#endif


