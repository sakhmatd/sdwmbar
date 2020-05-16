PROJECT = sdwmbar
CC = cc

PREFIX = /usr/local

INCS   = -I/usr/local/include
LINKS  = -L/usr/local/lib -lX11
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
