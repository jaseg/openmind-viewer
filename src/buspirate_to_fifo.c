/*
 * Copyright 2011 by Sebastian Götte <s@jaseg.de>
 * 
 * This file is part of openmind
 * 
 * openmind is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * openmind is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

//FIXME Unix include stuff. Cleanup necessary.
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>

#define SAMPLE_BUFFER_LENGTH 64
#define CHANNEL_NUMBER 4
#define FILENAME_BUFFER_SIZE 256

int start(char* device, int* portfd_out);
int work(int* fifofds, int portfd);
void help(char* argv);

int main(int argc, char** argv){
	//Parse the arguments
	if(argc != 3){
		help(argv[0]);
		return 1;
	}
	int portfd = -1;
	//Initialize the connection
	int ec;
	if((ec = start(argv[1], &portfd)) != 0){
		printf("Error starting communication with buspirate: Error code %i\n", ec);
		return ec;
	}
	//Open the files
	int fifofds[CHANNEL_NUMBER];
	for(int i=0; i<CHANNEL_NUMBER; i++){
		char fifoname[FILENAME_BUFFER_SIZE];
		snprintf(fifoname, FILENAME_BUFFER_SIZE, "%s_ch%i", argv[2], i);
		int ec;
		if((ec = mkfifo(fifoname, 0))){
			printf("Cannot create fifo %s\n", fifoname);
			return ec;
		}
		if((fifofds[i] = open(fifoname, 0)) != -1){
			printf("Cannot open the just created fifo %s\n", fifoname);
			return errno;
		}
	}
	while(work(fifofds, portfd) == 0);
	for(int i=0; i<CHANNEL_NUMBER; i++){
		char fifoname[FILENAME_BUFFER_SIZE];
		snprintf(fifoname, FILENAME_BUFFER_SIZE, "%s_ch%i", argv[2], i);
		if(unlink(fifoname) == -1){
			printf("Cannot remove fifo %s: error code %i\n", fifoname, errno);
			return errno;
		}
	}
	return 0;
}

void help(char* argv0){
	printf("openmind buspirate to fifo translator\nUsage: %s [buspirate device] [fifo prefix]\n", argv0);
}

int work (int* fifofds, int portfd){
	char buf[SAMPLE_BUFFER_LENGTH]; //should be enough for at least 8ch
	char* bufp = 0;
	int error_code;
	char c;
	int state = 0;
	//Sorry for the crappy state machine. If this should not be clear enough, write me a mail and I'll fix it. ;)
	for(bufp = buf; (error_code = read(portfd, &c, 1)) == 1 && bufp < buf + SAMPLE_BUFFER_LENGTH-1;){ //Have to account for trailing \0 here
		if(state == 1){
			if(c == ']'){
				state = 0;
				//Received a block
				break;
			}else if(c == '\\'){
				 state = 2;
			}
		}
		switch(state){
		case 0:
			 if(c == '[')
				state = 1;
			 break;
		case 2:
			*bufp = c;
			bufp ++;
			state = 3;
			break;
		case 3:
			state = 1;
			break;
		}
	}
	*bufp = '\0';
	if(error_code != 1){
		return (1<<(sizeof(int)-2)) | error_code; //This is kind of arbitrary...
	}
	//Ok, we did not catch an error but saw a complete block. Let's extract the data, and for good measure we do
	//this using cstrings. Nevertheless any person in possession of a good code taste is warmly invited to port
	//this to just another great API, for now I just want to get this working.
	char* saveptr;
	char* token;
	int i=0;
	while((token = strtok_r(buf, " ", &saveptr))){
		if (i >= CHANNEL_NUMBER){
			printf("Strange error: Receiving data for more channels than specified. Plz fix this.\n");
			return 2;
		}
		// TODO automatic vref acquisition
		char* endptr;
		//FIXME The following line is pretty certainly crap inherited from previous versions of this code.
		fifofds[i] = (short int)(strtol(token, &endptr, 16));// /(ADS_VREF/((2^15)-1.0F)));
		if(*endptr != 0){
			//An error occured (most likely some glitch between this code and the openmind firmware).
			return 1;
		}
		i++;
	}
	return 0;
}

int start(char* device, int* portfd_out){
	struct timeval universal_timeout = {0, 1000};
	int portfd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	fd_set portfds;
	if(portfd == -1){
		printf("Cannot open serial port. errno: %i\n", errno);
		return 1;
	}
	fcntl(portfd, F_SETFL, 0);
	struct termios options;
	tcgetattr(portfd, &options);
	//Set baud rate to 115200 Bd (BusPirate standard)
	cfsetspeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD); //Enable reciever, do not take ownership of the port. NOTE: does the transmitter also need to be enabled?
    options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~PARENB; //no parity
	options.c_cflag &= ~CSTOPB; //1 stopbit
	options.c_cflag &= ~CSIZE;
    options.c_cflag &= ~CRTSCTS; //no hardware flow control
	options.c_cflag |= CS8; //8-bit chars
	tcsetattr(portfd, TCSANOW, &options);

	int tries = 0;
	char buf[6];
	char* bufp = buf;
	int nread = 0;
	//Sorry for the endless loop, but I think it is more clear like this.
	while(1){
		//In fact I do not know if every char of the init string counts as one "try". Anyway, I do not think that is too
		//important.
	    if(tries++ == 20){
			printf("Too many initialization tries.\n");
			return 2;
	    }else{
			char c=0;
    	    if(write(portfd, &c, 1) < 0){
				printf("Write error. errno: %i\n", errno);
				return 3; //Write error
			}else{
				FD_ZERO(&portfds);
				FD_SET(portfd, &portfds);
				if(!select(1, &portfds, NULL, NULL, &universal_timeout) < 0){ //Wait 1 millisecond for a reply
					//Port ready for reading
					int count = read(portfd, bufp, 5-nread);
					if(count<0){
						printf("Read error. errno: %i\n", errno);
						return 4; //Some read error.
					}else{
						bufp += count;
						nread += count;
						if(nread == 5){
							break; //Read complete.
						}
					}
				}
    	    }
    	}
	}
	buf[5] = 0;
	if(strncmp(buf, "BBIO1", sizeof(buf))){
		printf("Wrong response from buspirate.\n");
		return 5; //Error in the bp binary mode initialization response string
	}
	char c = 0x01;
	if(write(portfd, &c, 1) < 0){
			printf("Write error. errno: %i\n", errno);
			return 3; //write error
	}
	nread = 0;
	for(bufp = buf; nread<4;){
		FD_ZERO(&portfds);
		FD_SET(portfd, &portfds);
		if(select(1, &portfds, NULL, NULL, &universal_timeout)){
			printf("Timeout while initializing the buspirate.\n");
			return 2; //Timeout
		}
		int ret = read(portfd, bufp, 6-nread);
		if(ret < 0){
			printf("Read error. errno: %i\n", errno);
			return 4; //read error
		}
		nread += ret;
		bufp += ret;
	} 
	if(strncmp(buf, "SPI1", sizeof(buf))){
		printf("Wrong response from buspirate.\n");
		return 6; //Normally, SPI mode would be entered now.
	}
	c = 0x80;
	if(write(portfd, &c, 1) < 0){
		printf("Write error. errno: %i\n", errno);
		return 3; //write error
	}
	FD_ZERO(&portfds);
	FD_SET(portfd, &portfds);
	if(select(1, &portfds, NULL, NULL, &universal_timeout)){
		printf("Timeout while initializing the buspirate.\n");
		return 2; //Timeout
	}
	if(read(portfd, &c, 1) != 1){
		printf("Read error. errno: %i\n", errno);
		return 4; //read error
	}
	if(c != 0x01){
		printf("Wrong response from buspirate.\n");
		return 7; //Wrong response
	}
	c = 0x0E;
	if(write(portfd, &c, 1) < 0){
		printf("Write error. errno: %i\n", errno);
		return 3; //write error
	}
	FD_ZERO(&portfds);
	FD_SET(portfd, &portfds);
	if(select(1, &portfds, NULL, NULL, &universal_timeout)){
		printf("Timeout while initializing the buspirate.\n");
		return 2; //Timeout
	}
	if(read(portfd, &c, 1) != 1){
		printf("Read error. errno: %i\n", errno);
		return 4; //read error
	}
	if(c != 0x01){
		printf("Wrong response from buspirate.\n");
		return 8; //Wrong response
	}
	*portfd_out = portfd;
	//Jackpot!
	return 0;
}
