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

#include "updatescreen.h"

unsigned int *vncBuffer;

rfbScreenInfoPtr vncScreen;

int updateScreen(int width, int height, int bpp) {
	int x, y, slip, step, shift;
	int vb_offset = 0, fb_offset = 0, px_offset = 0;
	int max_x = -1, max_y = -1, min_x = 99999, min_y = 99999;
	int idle = 1;

	// Create buffers
	uint32_t* fb = (uint32_t*)readFrameBuffer();
	uint32_t* vb = (uint32_t*)vncBuffer;

	// Set the pixel grid slip (depends on the resolution)
	if (height < 540) {
		slip = 2;
	} else if (height >= 540 && height < 720) {
		slip = 3;
	} else if (height >= 720 && height < 1080) {
		slip = 4;
	} else if (height >= 1080 && height < 1440) {
		slip = 5;
	} else {
		slip = 6;
	}

	// Set the inline pixel step
	step = SQUARE(slip) - 1;

	// Generate a random step shift (It helps to eliminate any remaining dirty zones between each image update.)
	srand(time(NULL));
	shift = rand() % step;

	// Compare the buffers and find the differences in every line
	for (y = 0; y < height; y++) {
		// Set all offsets
		vb_offset = y * width;
		fb_offset = (y + screenInfo.yoffset) * screenInfo.xres_virtual + screenInfo.xoffset;
		px_offset = (y * slip + shift) % step;

		// Compare certain pixels in every line with an offset
		for (x = px_offset; x < width; x += step) {
			if (vb[x + vb_offset] != fb[x + fb_offset]) {
				if (idle) {
					// The current line reduced by the slip value -> Set as the first different line
					min_y = MIN(y - slip, min_y);
					idle = 0;
				}
				// The current line increased by the slip value -> Set as the last different line
				max_y = MAX(y + slip, max_y);
				break; // There is no need to examine this line anymore if it already has a difference
			}
		}
	}

	// Fill the image buffer with the new content
	if (!idle) {
		min_x = 0;
		min_y = MAX(0, min_y);
		max_x = width - 1;
		max_y = MIN(height - 1, max_y);

		for (y = min_y; y <= max_y; y++) {
			vb_offset = y * width;
			fb_offset = (y + screenInfo.yoffset) * screenInfo.xres_virtual + screenInfo.xoffset;
			memcpy(vncBuffer + vb_offset, fb + fb_offset, width * bpp / CHAR_BIT);
		}

		rfbMarkRectAsModified(vncScreen, min_x, min_y, max_x, max_y);
	}

	return idle;
}
