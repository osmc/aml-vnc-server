/*
droid VNC server  - a vnc server for android
Copyright (C) 2011 Jose Pereira <onaips@gmail.com>

Other contributors:
  Oleksandr Andrushchenko <andr2000@gmail.com>

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

#include "framebuffer.h"
#include <limits.h>

screenFormat screenformat;

int fbfd = -1;
unsigned int *fbmmap;

char framebuffer_device[256] = "/dev/fb0";

int roundUpToPageSize(int x);

struct fb_var_screeninfo scrinfo;
struct fb_fix_screeninfo fscrinfo;

void setFrameBufferDevice(char *s) {
	strcpy(framebuffer_device,s);
}

void updateFrameBufferInfo(void) {
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &scrinfo) != 0) {
		L(" 'ioctl' error!\n");
		exit(EXIT_FAILURE);
	}
}

inline int roundUpToPageSize(int x) {
	return (x + (sysconf(_SC_PAGESIZE)-1)) & ~(sysconf(_SC_PAGESIZE)-1);
}

int initFrameBuffer(void) {
	L("-- Initializing framebuffer device --\n");

	fbmmap = MAP_FAILED;

	if ((fbfd = open(framebuffer_device, O_RDONLY)) == -1) {
		L(" Cannot open framebuffer device '%s'\n", framebuffer_device);
		return -1;
	} else {
		L(" The framebuffer device has been attached.\n");
	}

	updateFrameBufferInfo();

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fscrinfo) != 0) {
		L(" 'ioctl' error!\n");
		return -1;
	}

	// Framebuffer debug information
	/*
	L(" line_length=%d xres=%d, yres=%d, xresv=%d, yresv=%d, xoffs=%d, yoffs=%d, bpp=%d\n",
		(int)fscrinfo.line_length, (int)scrinfo.xres, (int)scrinfo.yres,
		(int)scrinfo.xres_virtual, (int)scrinfo.yres_virtual,
		(int)scrinfo.xoffset, (int)scrinfo.yoffset,
		(int)scrinfo.bits_per_pixel);
	*/

	size_t size = scrinfo.yres_virtual;
	size_t fbSize = roundUpToPageSize(fscrinfo.line_length * size);

	fbmmap = mmap(NULL, fbSize, PROT_READ, MAP_SHARED, fbfd, 0);

	if (fbmmap == MAP_FAILED) {
		L(" 'mmap' failed!\n");
		return -1;
	}

	fillScreenValues();

	return 1;
}

void closeFrameBuffer(void) {
	if(fbfd != -1)
	close(fbfd);
	L(" The framebuffer device has been detached.\n");
}

int checkResolutionChange(void) {
	if ((scrinfo.xres != screenformat.width) || (scrinfo.yres != screenformat.height)) {
		fillScreenValues();
		return 1;
	} else {
		return 0;
	}
}

void fillScreenValues(void) {
	screenformat.width = scrinfo.xres;
	screenformat.height = scrinfo.yres;
	screenformat.bitsPerPixel = scrinfo.bits_per_pixel;
	screenformat.size = screenformat.width * screenformat.height * screenformat.bitsPerPixel / CHAR_BIT;
	screenformat.redShift = scrinfo.red.offset;
	screenformat.redMax = scrinfo.red.length;
	screenformat.greenShift = scrinfo.green.offset;
	screenformat.greenMax = scrinfo.green.length;
	screenformat.blueShift = scrinfo.blue.offset;
	screenformat.blueMax = scrinfo.blue.length;
}

struct fb_var_screeninfo FB_getscrinfo(void) {
	return scrinfo;
}

unsigned int *readBufferFB(void) {
	updateFrameBufferInfo();
	return fbmmap;
}
