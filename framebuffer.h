/*
droid VNC server  - a vnc server for android
Copyright (C) 2011 Jose Pereira <onaips@gmail.com>

Modified for AML TV Boxes by kszaq <kszaquitto@gmail.com>
Additional developments by dtech(.hu) <dee.gabor@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "common.h"

#include <limits.h>

#define FB_DEVICE "/dev/fb0"

extern struct fb_var_screeninfo screenInfo;

void updateFrameBufferInfo(void);
int roundUpToPageSize(int x);
int initFrameBuffer(void);
void closeFrameBuffer(void);
int checkResolutionChange(void);
void updateScreenFormat(void);
struct fb_var_screeninfo getScreenInfo(void);
unsigned int *readFrameBuffer(void);

#endif
