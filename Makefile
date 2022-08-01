PROJECT = sdwmbar
CC ?= cc

OS != uname
.if ${OS} == OpenBSD
XPREFIX ?= /usr/X11R6
.endif
PREFIX ?= /usr/local
XPREFIX ?= /usr/local

INCS   = -I${XPREFIX}/include
LINKS  = -L${XPREFIX}/lib -lX11
CFLAGS += -std=c99 -pedantic -Wall -Werror ${INCS} ${LINKS}

all:
	${CC} ${PROJECT}.c ${CFLAGS} ${LDFLAGS} -o ${PROJECT}

debug:
	${CC} ${PROJECT}.c ${CFLAGS} ${LDFLAGS} -g -o ${PROJECT}

clean:
	rm -rf ${PROJECT}

install:
	install ${PROJECT} ${DESTDIR}${PREFIX}/bin/${PROJECT}

uninstall:
	rm -rf ${PREFIX}/bin/${PROJECT}
