/* Copyright 2023 Sergei Akhmatdinov             			     */
/*									     */
/* Licensed under the Apache License, Version 2.0 (the "License");	     */
/* you may not use this file except in compliance with the License.	     */
/* You may obtain a copy of the License at				     */
/*									     */
/*     http://www.apache.org/licenses/LICENSE-2.0			     */
/*									     */
/* Unless required by applicable law or agreed to in writing, software	     */
/* distributed under the License is distributed on an "AS IS" BASIS,	     */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/* See the License for the specific language governing permissions and	     */
/* limitations under the License. */

/* Includes */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>

/* Constants and macros */
#define BATT_LEN 5
#define LOAD_LEN 15
#define PLUG_LEN 3
#define STATUS_LEN 75
#define TIME_LEN 18
#define VRSN_LEN 10

/* Function prototypes */
static void printerr(char *msg);
static void getversion(char *buff);
static void get_load(char *buf);
static void get_time(char *buf);
static void get_plug(char *buf);
static int  getbatt(char *buf);
static void set_status(char *status);

/* Global variables */
#include "config-linux.h"

/* Prints the error messagea and terminates the execution. */
void
printerr(char *msg)
{
	fprintf(stderr, "sdwmbar: %s; errno: %s\n", msg, strerror(errno));
	exit(-1);
}

void
getversion(char *buff)
{
	FILE *fp;

	fp = popen("dwm -v 2>&1", "r");
	if (!fp)
		printerr("sdwmbar: no dwm binary found!");

	fgets(buff, VRSN_LEN, fp);
	pclose(fp);

	/* Remove the trailing newline */
	buff[strcspn(buff, "\n")] = 0;
}

/* Gets the average CPU load within the last minue, five minutes, and fifteen
 * minutes. */
void
get_load(char *buf)
{
	double results[3];
	if (getloadavg(results, 3) != 3)
		printerr("unable to load (getloadavg)");

	snprintf(buf, LOAD_LEN,
	         "%.2f %.2f %.2f", results[0], results[1], results[2]);
}

/* Gets date and time. */
void
get_time(char *buf)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	if (!tm)
		printerr("unable to get time (localtime)");

	if (!strftime(buf, TIME_LEN, "%F  %R", tm))
		printerr("unable to get time (strftime)");
}

/* Gets AC plug state. */
void
get_plug(char *buf)
{
	ssize_t bytes;
	int fd;

	/* No need to check for errors because this function will never be
	 * called if the battery has not been found. */
	fd = open(PLUG_PATH, O_RDONLY);
	bytes = read(fd, buf, PLUG_LEN-1);
	buf[bytes-1] = 0; /* -1 to discard '\n' */
	buf[0] = buf[0] == '1' ? '+' : 0; /* 1 if AC plug is plugged in. */
	close(fd);
}

/* Gets the battery charge level, returns 1 if there is a battery. */
int
getbatt(char *buf)
{
	ssize_t bytes;
	int fd;

	fd = open(BATT_PATH, O_RDONLY);
	if (fd < 0) /* There is no battery, most probably. */
		return 0;
	bytes = read(fd, buf, BATT_LEN-1);
	if (bytes < 0) /* The battery might have been taken out. */
		return 0;
	buf[bytes-1] = 0;
	close(fd);
	return 1;
}

/* Sets the status bar. */
void
set_status(char *status)
{
	Display *display;
	if (!(display = XOpenDisplay(0)))
		printerr("unable to connect to display (XOpenDisplay)");
	
	XStoreName(display, XDefaultRootWindow(display), status);
	XSync(display, 0);
	XCloseDisplay(display);
}

int
main(void) {
	char batt[BATT_LEN];
	char load[LOAD_LEN];
	char plug[PLUG_LEN];
	char time[TIME_LEN];
	char status[STATUS_LEN];
	char version[VRSN_LEN];
	
	/* Only need to get version once */
	getversion(version);

	if (!getbatt(batt))
		fprintf(stderr, "WARN: No battery was detected.\n");

	for (;; sleep(10)) {
		get_load(load);
		get_time(time);
		if (getbatt(batt)) {
			get_plug(plug);
			snprintf(status, STATUS_LEN, "%s  L:%s  %s  %s%s%%",
			         version, load, time, plug, batt);
		} else { /* No battery has been found. */
			snprintf(status, STATUS_LEN, "%s  L:%s  %s",
			         version, load, time);
		}
		set_status(status);
	}
}
