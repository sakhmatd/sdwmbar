/* Copyright 2017 Sergei Akhmatdinov                                         */
/*                                                                           */
/* Licensed under the Apache License, Version 2.0 (the "License");           */
/* you may not use this file except in compliance with the License.          */
/* You may obtain a copy of the License at                                   */
/*                                                                           */
/*     http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                           */
/* Unless required by applicable law or agreed to in writing, software       */
/* distributed under the License is distributed on an "AS IS" BASIS,         */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/* See the License for the specific language governing permissions and       */
/* limitations under the License. */

/* BSD definitions take priority. */
#define _BSD_SOURCE

/* Needed for sleep() */
#include <unistd.h>
#include <stdlib.h>

/* Needed for getversion() */
#include <string.h>

/* Needed for snprintf() */
#include <stdio.h>

/* Needed for gettime() */
#include <time.h>

/* Needed for getbatt() */
#include <sys/types.h>
#include <sys/sysctl.h>

/* Needed to set the statusbar */
#include <X11/Xlib.h>

void
printerr(char *err) 
{
	fprintf(stderr, "%s\n", err);
	exit(1);
}

char *
getversion(void)
{
	FILE *fp;
	
	fp = popen("dwm -v 2>&1", "r");
	if (!fp)
		printerr("No dwm binary found!");

	char *buff = malloc(10);
	if (!buff)
		printerr("Unable to allocate memory! (getversion)");
	
	fgets(buff, 10, fp);
	fclose(fp);

	/* Remove the trailing newline */
	buff[strcspn(buff, "\n")] = 0;

	return buff;
}

char *
gettime(void)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	if (!tm)
		printerr("sdwmbar: unable to get time (localtime)!");
	
	char *buff = malloc(65);
	if (!buff)
		printerr("sdwmbar: unable to allocate memory! (gettime)");
	
	if (!strftime(buff, 65, "%D  %R", tm))
		printerr("sdwmbar: unable to get time (strftime)!");
	
	return buff;
}

int
getbatt(void)
{
	int buff[1];
        size_t len = 1;
	sysctlbyname("hw.acpi.battery.life", buff, &len, NULL, 0);
		
	return *buff;
}

void
setstatus(char *status)
{
	Display *display;

	if (!(display = XOpenDisplay(0)))
		printerr("sdwmbar: unable to open display!");
	
	XStoreName(display, DefaultRootWindow(display), status);
	XSync(display, 0);
	XCloseDisplay(display);
}

char *
getload(void)
{
	double results[3];
	char   *buff = malloc(20);
	if (!buff)
		printerr("sdwmbar: unable to allocate memory (getload)!");

	if (getloadavg(results, 3) < 0)
		printerr("sdwmbar: unable to get load data!");
       
	snprintf(buff, 20, "%.2f %.2f %.2f", results[0], results[1], results[2]);
	return buff;
}

int
main(void)
{
	char status[100];
	char *load;
	char *time;
	char *version;
	int  batt;

	/* Only need to get version once */
	version = getversion();

	for (;;sleep(10)) {
		load = getload();
		time = gettime();
		batt = getbatt();

		snprintf(status, 100, "%s  L:%s  %s  %d%%",
				      version, load, time, batt);
		setstatus(status);
		free(load);
		free(time);
	}

	free(version);

	return 0;
}

