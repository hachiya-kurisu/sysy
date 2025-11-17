VERSION = 0.3

OS != uname -s

-include Makefile.$(OS)

CFLAGS += -Wall -Wextra -std=c99 -DVERSION=\"${VERSION}\"

CFLAGS += -fstack-protector-strong -D_FORTIFY_SOURCE=2
CFLAGS += -Wshadow -Wcast-align -Wstrict-prototypes
CFLAGS += -Wwrite-strings -Wconversion -Wformat-security
CFLAGS += -Wmissing-prototypes -Wold-style-definition

LINTFLAGS += --enable=all --inconclusive --language=c --library=posix
LINTFLAGS += --quiet --suppress=missingIncludeSystem

PREFIX ?= /usr/local
MANDIR ?= /share/man

all: sysy

again: clean all

libsysy.a: src/sysy.c
	${CC} ${CFLAGS} -c src/sysy.c -o sysy.o
	ar -cvqs libsysy.a sysy.o

sysy: libsysy.a src/main.c
	${CC} ${CFLAGS} -L. -o sysy src/main.c -lsysy
	strip sysy

doc: README.md

README.md: README.gmi
	cat README.gmi | sisyphus -f markdown > README.md

install:
	install sysy ${DESTDIR}${PREFIX}/bin/sysy
	install libsysy.a ${DESTDIR}${PREFIX}/lib/libsysy.a
	install src/sysy.h ${DESTDIR}${PREFIX}/include/sysy.h

lint:
	cppcheck ${LINTFLAGS} src/*.c

clean:
	rm -f libsysy.a sysy.o sysy

push:
	got send
	git push github

release: push
	git push github --tags

