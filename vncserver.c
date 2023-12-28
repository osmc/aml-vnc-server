/*
droid vnc server - Android VNC server
Copyright (C) 2009 Jose Pereira <onaips@gmail.com>

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

#include "common.h"
#include "version.h"
#include "framebuffer.h"
#include "newinput.h"

#include <rfb/rfb.h>
#include <rfb/keysym.h>

#include <time.h>
#include <signal.h>

#define CONCAT2(a,b) a##b
#define CONCAT2E(a,b) CONCAT2(a,b)
#define CONCAT3(a,b,c) a##b##c
#define CONCAT3E(a,b,c) CONCAT3(a,b,c)

int update_loop = 1;

char VNC_SERVERNAME[256] = "AML-VNC";
char VNC_PASSWORD[256] = "";
int VNC_PORT = 5900;

unsigned int *vncbuf;

static rfbScreenInfoPtr vncscr;

uint32_t idle = 1;
uint32_t standby = 1;

// Reverse connection
char *rhost = NULL;
int rport = 5500;

void (*updateScreen)(void) = NULL;

#define OUT 32
#include "updateScreen.c"
#undef OUT

void setIdle(int i) {
	idle=i;
}

ClientGoneHookPtr clientGone(rfbClientPtr cl) {
	return 0;
}

rfbNewClientHookPtr clientHook(rfbClientPtr cl) {
	cl->clientGoneHook=(ClientGoneHookPtr)clientGone;
	return RFB_CLIENT_ACCEPT;
}

void extractReverseHostPort(char *str) {
	int len = strlen(str);
	char *p;
	/* copy in to host */
	rhost = (char *) malloc(len + 1);
	if (! rhost) {
		L("Reverse connection: could not malloc string %d\n", len);
		exit(-1);
	}
	strncpy(rhost, str, len);
	rhost[len] = '\0';

	/* Extract port, if any */
	if ((p = strrchr(rhost, ':')) != NULL) {
		rport = atoi(p + 1);
		if (rport < 0) {
			rport = -rport;
		}
		else if (rport < 20) {
			rport = 5500 + rport;
		}
		*p = '\0';
	}
}

void initReverseConnection(void) {
	rfbClientPtr cl;
	cl = rfbReverseConnection(vncscr, rhost, rport);
	if (cl == NULL) {
		char *str = malloc(255 * sizeof(char));
		L(" Couldn't connect to remote host: %s\n",rhost);
		free(str);
	} else {
		cl->onHold = FALSE;
		rfbStartOnHoldClient(cl);
	}
}

void initServer(int argc, char **argv) {
	L("-- Initializing VNC server --\n");
	L(" Screen resolution: %dx%d, bit depth: %d bpp.\n",
		(int)screenformat.width, (int)screenformat.height, (int)screenformat.bitsPerPixel);
	L(" RGBA colormap: %d:%d:%d, length: %d:%d:%d.\n",
		screenformat.redShift, screenformat.greenShift, screenformat.blueShift,
		screenformat.redMax, screenformat.greenMax, screenformat.blueMax);

	vncbuf = calloc(screenformat.width * screenformat.height, screenformat.bitsPerPixel / CHAR_BIT);
	assert(vncbuf != NULL);

	vncscr = rfbGetScreen(&argc, argv, screenformat.width, screenformat.height, 0 /* not used */ , 3,  screenformat.bitsPerPixel / CHAR_BIT);
	assert(vncscr != NULL);

	vncscr->desktopName = VNC_SERVERNAME;
	vncscr->frameBuffer = (char *)vncbuf;
	vncscr->port = VNC_PORT;
	vncscr->ipv6port = VNC_PORT;
	vncscr->kbdAddEvent = dokey;
	vncscr->ptrAddEvent = doptr;
	vncscr->newClientHook = (rfbNewClientHookPtr)clientHook;

	if (strcmp(VNC_PASSWORD, "") != 0) {
		char **passwords = (char **)malloc(2 * sizeof(char **));
		passwords[0] = VNC_PASSWORD;
		passwords[1] = NULL;
		vncscr->authPasswdData = passwords;
		vncscr->passwordCheck = rfbCheckPasswordByList;
	}

	vncscr->serverFormat.redShift = screenformat.redShift;
	vncscr->serverFormat.greenShift = screenformat.greenShift;
	vncscr->serverFormat.blueShift = screenformat.blueShift;

	vncscr->serverFormat.redMax = (( 1 << screenformat.redMax) -1);
	vncscr->serverFormat.greenMax = (( 1 << screenformat.greenMax) -1);
	vncscr->serverFormat.blueMax = (( 1 << screenformat.blueMax) -1);

	vncscr->serverFormat.trueColour = TRUE;
	vncscr->serverFormat.bitsPerPixel = screenformat.bitsPerPixel;

	vncscr->alwaysShared = TRUE;

	L("-- Starting the server --\n");
	rfbInitServer(vncscr);

	if (rhost)
		initReverseConnection();

	updateScreen = update_screen_32;

	/* Mark as dirty since we haven't sent any updates at all yet. */
	rfbMarkRectAsModified(vncscr, 0, 0, vncscr->width, vncscr->height);
}

void sigHandler() {
	update_loop = 0;
}

void printUsage(char *str) {
	L("A framebuffer based VNC Server for Amlogic devices\n\n"
		"Usage: %s [parameters]\n"
		"-h\t\t- Print this help\n"
		"-f <device>\t- Framebuffer device\n"
		"-P <port>\t- Listening port\n"
		"-n <name>\t- Server name\n"
		"-p <password>\t- Password to access server\n"
		"-R <host:port>\t- Host for reverse connection\n", str);
}

int main(int argc, char **argv) {
	long usec;

	// Set the default server name based on the hostname
	gethostname(VNC_SERVERNAME, sizeof(VNC_SERVERNAME));

	// Preset values from environment variables (However, the values specified in the arguments have priority.)
	if (getenv("VNC_SERVERNAME"))
		strcpy(VNC_SERVERNAME, getenv("VNC_SERVERNAME"));
	if (getenv("VNC_PASSWORD"))
		strcpy(VNC_PASSWORD, getenv("VNC_PASSWORD"));
	if (getenv("VNC_PORT"))
		VNC_PORT = atoi(getenv("VNC_PORT"));

	L("AML-VNC Server v%d.%d.%d", MAIN_VERSION_MAJOR, MAIN_VERSION_MINOR, MAIN_VERSION_PATCH);
	if (MAIN_VERSION_BETA != 0)
		L(" Beta %d", MAIN_VERSION_BETA);
	L(" (Release date: %s)\n", MAIN_VERSION_DATE);

	if(argc > 1) {
		int i = 1;
		while(i < argc) {
			if(*argv[i] == '-') {
				switch(*(argv[i] + 1)) {
					case 'h':
						printUsage(argv[0]);
						exit(0);
						break;
					case 'n':
						i++;
						strcpy(VNC_SERVERNAME,argv[i]);
						break;
					case 'p':
						i++;
						strcpy(VNC_PASSWORD,argv[i]);
						break;
					case 'f':
						i++;
						FB_setDevice(argv[i]);
						break;
					case 'P':
						i++;
						VNC_PORT=atoi(argv[i]);
						break;
					case 'R':
						i++;
						extractReverseHostPort(argv[i]);
						break;
				}
			}
		i++;
		}
	}

	// Start initialization
	initFB();
	initVirtKbd();
	initVirtPtr();
	initServer(argc, argv);

	signal(SIGINT, sigHandler);

	// Start the update loop
	while (update_loop) {
		usec = (vncscr->deferUpdateTime + standby) * 1000;
		rfbProcessEvents(vncscr, usec);
		if (idle)
			standby = 100;
		else
			standby = 10;

		if (vncscr->clientHead != NULL) {
			if (!checkResChange()) {
				updateScreen();
			} else {
				L("-- Screen resolution changed, reinitialization started --\n");
				rfbShutdownServer(vncscr, TRUE);
				free(vncscr->frameBuffer);
				rfbScreenCleanup(vncscr);
				closeVirtPtr();
				initVirtPtr();
				initServer(argc, argv);
			}
		}
	}

	// VNC server shutdown
	L("-- Shutting down the server --\n");
	rfbShutdownServer(vncscr, TRUE);
	free(vncscr->frameBuffer);
	rfbScreenCleanup(vncscr);

	L("-- Cleaning up --\n");
	closeFB();
	closeVirtKbd();
	closeVirtPtr();

	return 0;
}
