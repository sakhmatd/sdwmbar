PROJECT = sdwmbar
CC = cc

PREFIX = /usr/local

INCS   = -I/usr/local/include
LINKS  = -L/usr/local/lib -lX11
CFLAGS = -g -std=c99 -pedantic -Wall -Werror ${INCS} ${LINKS}

all:
	cc ${PROJECT}.c ${CFLAGS} -o ${PROJECT}

clean:
	rm -rf ${PROJECT}

install:
	cp -f ${PROJECT} ${PREFIX}/bin/
	chmod 755 ${PREFIX}/bin/${PROJECT}

uninstall:
	rm -rf ${PREFIX}/bin/${PROJECT}
