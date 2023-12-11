/*
droid VNC server  - a vnc server for android
Copyright (C) 2011 Jose Pereira <onaips@gmail.com>

Modified for AML TV Boxes by kszaq <kszaquitto@gmail.com>

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

#define OUT_T CONCAT3E(uint,OUT,_t)
#define FUNCTION CONCAT2E(update_screen_,OUT)
#define COMPRECT 4
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void FUNCTION(void) {
	OUT_T* b = (OUT_T*)readBufferFB();
	OUT_T* a = (OUT_T*)vncbuf;

	struct fb_var_screeninfo scrinfo; //we'll need this to detect double FB on framebuffer
	scrinfo = FB_getscrinfo();

	idle = 1;

	int i, j;
	int offset = 0, fb_offset = 0;
	int max_x = -1, max_y = -1, min_x = 99999, min_y = 99999;

	for (j = 0; j < vncscr->height; j+=COMPRECT) {
		offset = j * vncscr->width;
		fb_offset = (j + scrinfo.yoffset) * scrinfo.xres_virtual + scrinfo.xoffset;
		for (i = 0; i < vncscr->width; i+=COMPRECT) {
			if (a[i + offset] != b[i + fb_offset]) {
				min_y=MIN(j - COMPRECT, min_y);
				max_y=MAX(j + COMPRECT, max_y);
				idle = 0;
			}
		}
	}

	if (!idle) {
		min_x=0;
		min_y=MAX(0, min_y);
		max_x=screenformat.width - 1;
		max_y=MIN(screenformat.height - 1, max_y);

		for (j = min_y; j <= max_y; j++) {
			offset = j * vncscr->width;
			fb_offset = (j + scrinfo.yoffset) * scrinfo.xres_virtual + scrinfo.xoffset;
			memcpy(vncbuf + offset, b + fb_offset, screenformat.width * screenformat.bitsPerPixel / CHAR_BIT);
		}

		rfbMarkRectAsModified(vncscr, min_x, min_y, max_x, max_y);
	}
}
