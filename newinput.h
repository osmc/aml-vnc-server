#ifndef NEWINPUT_H
#define NEWINPUT_H

#include <linux/input.h>
#include <linux/uinput.h>

#include <rfb/rfb.h>
#include <rfb/keysym.h>

#include "common.h"

void initVirtKbd();
void closeVirtKbd();
void initVirtPtr();
void closeVirtPtr();
void writeEvent(int udev, __u16 type, __u16 code, __s32 value);
int keysym2scancode(rfbKeySym key);
void dokey(rfbBool down, rfbKeySym key, rfbClientPtr cl);
void doptr(int buttonMask, int x, int y, rfbClientPtr cl);

#endif


