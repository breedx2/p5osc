/* p5osc.c - by jason plumb
* 
* for more information:
* http://noisybox.net/computers/p5glove/ 
*
*****************************************************************************************
* Copyright (C) 2005 jason plumb
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*****************************************************************************************
*/

// Hi.  This is very quick and dirty...

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include "OSC-client.h"
#include "htmsocket.h"
#include "p5glove.h"
#include "p5osc.h"

char *hostname = "localhost";
int port = 9999;
P5Glove p5 = NULL;

/* TODO: Reevaluate these later */
#define MAXMESG 2048
#define SC_BUFFER_SIZE 32000
static char bufferForOSCbuf[SC_BUFFER_SIZE];
OSCbuf buf[1];
void *htmsocket;

int main(int argc, char **argv){

	if(!parseArgs(argc, argv)){
		showUsage();
		return -1;
	}

	printf("Creating socket for %s:%d...\n", hostname, port);
	short ttl = -1;
	htmsocket = OpenHTMSocket(hostname, port, &ttl);
	if(!htmsocket){
		printf("Aborting\n");
		return -1;
	}
	p5 = p5glove_open(0);
	if(p5 == NULL) {
		CloseHTMSocket(htmsocket);
		perror("*** p5 error: Couldn't open p5 glove device.\n");
		return -1;
	}
	
	printf("Working...move around.\n");
	while(1){
		int err = p5glove_sample(p5, -1);
		if (err < 0 && errno == EAGAIN){
			continue;
		}
		if (err < 0) {
			perror("*** p5 glove failure");
			break;
		}

		sendSample(err);

	}

	printf("Closing.\n");
	CloseHTMSocket(htmsocket);
	return 0;
}

bool sendSample(int rc){

	OSC_initBuffer(buf, SC_BUFFER_SIZE, bufferForOSCbuf);
	if (OSC_openBundle(buf, OSCTT_Immediately())) {
		printf("Problem opening bundle: %s\n", OSC_errorMessage);
		OSC_resetBuffer(buf);
		return 0;
	}

	int returnVal = OSC_writeAddress(buf, "/p5glove/buttons");
	if (returnVal) {
		printf("Problem writing address: %s\n", OSC_errorMessage);
	}


	/* Output button data */
	uint32_t buttons;
	p5glove_get_buttons(p5, &buttons);
	if(buttons & P5GLOVE_BUTTON_A)
		OSC_writeFloatArg(buf, 1);
	else
		OSC_writeFloatArg(buf, 0);

	if(buttons & P5GLOVE_BUTTON_B)
		OSC_writeFloatArg(buf, 1);
	else
		OSC_writeFloatArg(buf, 0);

	if(buttons & P5GLOVE_BUTTON_C)
		OSC_writeFloatArg(buf, 1);
	else
		OSC_writeFloatArg(buf, 0);

	/* Output finger data */
	returnVal = OSC_writeAddress(buf, "/p5glove/fingers");
	double clench;
	p5glove_get_finger(p5, P5GLOVE_FINGER_THUMB, &clench);
	OSC_writeFloatArg(buf, clench);

	p5glove_get_finger(p5, P5GLOVE_FINGER_INDEX ,&clench);
	OSC_writeFloatArg(buf, clench);

	p5glove_get_finger(p5, P5GLOVE_FINGER_MIDDLE ,&clench);
	OSC_writeFloatArg(buf, clench);

	p5glove_get_finger(p5, P5GLOVE_FINGER_RING ,&clench);
	OSC_writeFloatArg(buf, clench);

	p5glove_get_finger(p5, P5GLOVE_FINGER_PINKY ,&clench);
	OSC_writeFloatArg(buf, clench);

	returnVal = OSC_writeAddress(buf, "/p5glove/position");
	double pos[3];
	p5glove_get_position(p5, pos);
	OSC_writeFloatArg(buf, pos[0]);
	OSC_writeFloatArg(buf, pos[1]);
	OSC_writeFloatArg(buf, pos[2]);

	/* Output rotation data */
	returnVal = OSC_writeAddress(buf, "/p5glove/rotation");
	double axis[3], angle;
	p5glove_get_rotation(p5, &angle, axis);
	OSC_writeFloatArg(buf, axis[0]);
	OSC_writeFloatArg(buf, axis[1]);
	OSC_writeFloatArg(buf, axis[2]);
	OSC_writeFloatArg(buf, angle);


	OSC_closeBundle(buf);
	if(!SendHTMSocket(htmsocket, OSC_packetSize(buf), OSC_getPacket(buf))){
		perror("Sending failed!\n");
	}
	OSC_resetBuffer(buf);
	return 1;
}

bool parseArgs(int argc, char **argv){
/*
	while (1) {
		char *key;
		int option_index = 0;
		static struct option long_options[] = {
			{"host", 1, 0, 0},
			{"help", 0, 0, 0},
			{0, 0, 0, 0}
		};

		int c = getopt_long(argc, argv, "?h:", long_options, &option_index);
		if (c == -1)
		break;

		switch (c) {
			case 0:
				key = (char*)long_options[option_index].name;
				if (optarg){
					if(!strcmp(key, "help")){
						return 0;
					}
					else if(!strcmp(key, "host")){
						hostname = optarg;
					}
				}
				printf ("\n");
				break;

			case '?':
				return 0;

			case 'h':
				hostname = optarg;
				break;
		}
	}
	*/
	if(argc > 1){
		hostname = argv[1];
	}

	char *p = strchr(hostname, ':');
	if(p){
		port = atoi(p+1);
		*p = '\0';
	}
	if(!hostname) return 0;

	return 1;
}//parseArgs

void showUsage(){
	printf("Usage: --host <host>[:port]\n");
	printf("              (defaults to localhost:9999)\n\n");
}//showUsage
