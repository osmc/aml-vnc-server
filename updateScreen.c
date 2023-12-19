/*
droid VNC server  - a vnc server for android
Copyright (C) 2011 Jose Pereira <onaips@gmail.com>

Modified for AML TV Boxes by kszaq <kszaquitto@gmail.com>
Additional developments by dtechsrv <dee.gabor@gmail.com>

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
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define SQUARE(x) ((x)*(x))

void FUNCTION(void) {
	idle = 1; // Reset idle state first

	OUT_T* fb = (OUT_T*)readBufferFB();
	OUT_T* vb = (OUT_T*)vncbuf;

	// Get screen information
	struct fb_var_screeninfo scrinfo;
	scrinfo = FB_getscrinfo();

	int i, j, gridslip, px_step, px_random;
	int vb_offset = 0, fb_offset = 0, px_offset = 0;
	int max_x = -1, max_y = -1, min_x = 99999, min_y = 99999;

	// Set the pixel grid slip distance (depends on the screen resolution)
	if (vncscr->height <= 540) {
		gridslip = 2;
	} else if (vncscr->height > 540 && vncscr->height < 1080) {
		gridslip = 3;
	} else {
		gridslip = 4;
	}

	// Set the inline pixel step distance
	px_step = SQUARE(gridslip) - 1;

	// Generate a random step offset (It helps to eliminate any remaining dirty zones between each image update.)
	srand(time(NULL));
	px_random = rand() % px_step;

	// Compare the buffers and find the differences in every line
	for (j = 0; j < vncscr->height; j++) {

		// Set all offsets
		vb_offset = j * vncscr->width;
		fb_offset = (j + scrinfo.yoffset) * scrinfo.xres_virtual + scrinfo.xoffset;
		px_offset = (j * gridslip + px_random) % px_step;

		// Compare certain pixels in this line with an offset
		for (i = px_offset; i < vncscr->width; i += px_step) {
			if (vb[i + vb_offset] != fb[i + fb_offset]) {
				if (idle) {
					idle = 0;
					min_y = MIN(j - gridslip, min_y); // Set the current line as the first
				}

				// Set the current line as the last
				max_y = MAX(j + gridslip, max_y);
				break; // There is no need to examine this line anymore if it already has a difference
			}
		}
	}

	if (!idle) {
		min_x = 0;
		min_y = MAX(0, min_y);
		max_x = screenformat.width - 1;
		max_y = MIN(screenformat.height - 1, max_y);

		// Fill the image buffer with the new content
		for (j = min_y; j <= max_y; j++) {
			vb_offset = j * vncscr->width;
			fb_offset = (j + scrinfo.yoffset) * scrinfo.xres_virtual + scrinfo.xoffset;
			memcpy(vncbuf + vb_offset, fb + fb_offset, screenformat.width * screenformat.bitsPerPixel / CHAR_BIT);
		}

		rfbMarkRectAsModified(vncscr, min_x, min_y, max_x, max_y);
	}
}
