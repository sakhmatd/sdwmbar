/* Copyright 2020 Sergei Akhmatdinov					     */
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
#ifdef __OpenBSD__
#include <sys/ioctl.h>
#include <fcntl.h>
#include <machine/apmvar.h>
#endif /* __OpenBSD__ */

/* Needed to set the statusbar */
#include <X11/Xlib.h>

#define STAT_LEN 75
#define VRSN_LEN 10
#define TIME_LEN 65
#define LOAD_LEN 20
#define AC_LEN 2

#ifdef __OpenBSD__
/* /dev/apm file descriptor */
int apmfd;
#endif /* __OpenBSD__ */

void
printerr(char *err)
{
	fprintf(stderr, "%s\n", err);
	exit(1);
}

void
getversion(char *buff)
{
	FILE *fp;

	fp = popen("dwm -v 2>&1", "r");
	if (!fp)
		printerr("sdwmbar: no dwm binary found!");

	fgets(buff, VRSN_LEN, fp);
	fclose(fp);

	/* Remove the trailing newline */
	buff[strcspn(buff, "\n")] = 0;
}

void
gettime(char *buff)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	if (!tm)
		printerr("sdwmbar: unable to get time (localtime)!");

	if (!strftime(buff, TIME_LEN, "%D  %R", tm))
		printerr("sdwmbar: unable to get time (strftime)!");
}

int
getbatt(int *buff)
{
#ifdef __OpenBSD__
	struct apm_power_info pinfo;

	if (ioctl(apmfd, APM_IOC_GETPOWER, &pinfo) < 0)
		printerr("sdwmbar: unable to get power state");

	if (pinfo.battery_state == APM_BATTERY_ABSENT) {
		return 0;
	} else {
		*buff = pinfo.battery_life;
		return 1;
	}
#else
	size_t len = sizeof(buff);
	int ret = sysctlbyname("hw.acpi.battery.life", buff, &len, NULL, 0);

	if (ret < 0) {
		return 0;
	} else {
		return 1;
	}
#endif /* __OpenBSD__ */
}

void
getac(char *buff)
{
#ifdef __OpenBSD__
	struct apm_power_info pinfo;

	if (ioctl(apmfd, APM_IOC_GETPOWER, &pinfo) < 0)
		printerr("sdwmbar: unable to get power state");

	*buff = (pinfo.ac_state == APM_AC_ON) ? '+' : '\0';
#else
	int ac;
	size_t len = sizeof(ac);
	sysctlbyname("hw.acpi.acline", &ac, &len, NULL, 0);

	*buff = (ac) ? '+' : '\0';
#endif
	/* Ensure null termination */
	*(buff+1) = '\0';
}

void
getload(char *buff)
{
	double results[3];

	if (getloadavg(results, 3) < 0)
		printerr("sdwmbar: unable to get load data!");

	snprintf(buff, LOAD_LEN, "%.2f %.2f %.2f", results[0], results[1],
		 results[2]);
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

int
main(void)
{
	char status[STAT_LEN];
	char load[LOAD_LEN];
	char time[TIME_LEN];
	char version[VRSN_LEN];
	char ac[AC_LEN];
	int  batt;

#ifdef __OpenBSD__
	apmfd = open("/dev/apm", O_RDONLY);
	if (apmfd < 0)
		printerr("sdwmbar: unable to get apm file descriptor");
#endif

	/* Only need to get version once */
	getversion(version);

	if (!getbatt(&batt))
		fprintf(stderr, "WARN: No battery was detected.\n");

	for (;;sleep(10)) {
		getload(load);
		gettime(time);

		/* getbatt() returns 0 if no battery exists */
		if (getbatt(&batt)) {
			getac(ac);
			snprintf(status, STAT_LEN, "%s  L:%s  %s  %s%d%%",
					version, load, time, ac, batt);
		} else {
			snprintf(status, STAT_LEN, "%s  L:%s  %s",
					 version, load, time);
		}

		setstatus(status);
	}

	return 0;
}

