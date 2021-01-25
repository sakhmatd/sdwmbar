PROJECT = sdwmbar
CC = cc

PREFIX = /usr/local

OS != uname -s
.if ${OS} == OpenBSD
INCS  = -I/usr/X11R6/include
LINKS = -L/usr/X11R6/lib -lX11
.else
INCS  = -I/usr/local/include
LINKS = -L/usr/local/lib -lX11
.endif

CFLAGS = -std=c99 -pedantic -Wall -Werror ${INCS} ${LINKS}

all:
	cc ${PROJECT}.c ${CFLAGS} -o ${PROJECT}

debug:
	cc ${PROJECT}.c ${CFLAGS} -g -o ${PROJECT}

clean:
	rm -rf ${PROJECT}

install:
	install ${PROJECT} ${PREFIX}/bin/${PROJECT}

uninstall:
	rm -rf ${PREFIX}/bin/${PROJECT}
