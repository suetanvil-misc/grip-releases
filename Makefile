# Makefile for Grip and GCD

# OS
OS=$(shell uname -s)

# Compiler
CC=	gcc

# Install prefix
PREFIX=/usr

# Installation directory -- where the binary will go
INSTALLDIR= $(PREFIX)/bin

# Location to store auxilliary files
AUXDIR= $(PREFIX)/lib/grip


# Compiler flags
CFLAGS=	-Wall `gtk-config --cflags` -DAUXDIR=\"$(AUXDIR)\" \
	-DINSTALLDIR=\"$(INSTALLDIR)\" -D_REENTRANT

# Link libraries
LIBS=	`gtk-config --libs gthread`
ifeq ($(OS), Linux)
LIBS+=	-lpthread
endif
ifeq ($(OS), FreeBSD)
LIBS+=	-pthread
endif
PARLIBS=	cdparanoia/interface/libcdda_interface.a \
		cdparanoia/paranoia/libcdda_paranoia.a

# This is needed for "make install"
OWNER  = root
GROUP  = root
INSTALL  = /usr/bin/install -o $(OWNER) -g $(GROUP)


# ----------- You shouldn't need to make changes below here. -------------

VERSION= 2.96

OBJS=	cddb.o cd.o id3.o bug.o parsecfg.o dialog/input.o dialog/message.o

all: grip gcd

grip:	grip.o $(OBJS) cdpar.o
	$(CC) -o grip grip.o cdpar.o $(OBJS) $(LIBS) $(PARLIBS)

grip.o:	grip.c grip.h
	$(CC) $(CFLAGS) -DCDPAR -c grip.c -o grip.o

gripnopar.o:	grip.c grip.h
	$(CC) $(CFLAGS) -c grip.c -o gripnopar.o

gripnopar:	gripnopar.o $(OBJS)
	$(CC) -o grip gripnopar.o $(OBJS) $(LIBS)

gcd.o:	grip.h grip.c
	$(CC) $(CFLAGS) -DGRIPCD -c grip.c -o gcd.o

gcd:	gcd.o $(OBJS) grip.h
	$(CC) -o gcd gcd.o $(OBJS) $(LIBS)

$(OBJS):	grip.h config.h

grip.1:	grip.pod
	pod2man --section=1 --release="Gtk Applications" --center=" " \
	--official grip.pod > grip.1


install:
	$(INSTALL) -d $(INSTALLDIR)
	$(INSTALL) grip $(INSTALLDIR)
	$(INSTALL) gcd $(INSTALLDIR)
	$(INSTALL) -d $(AUXDIR)
	$(INSTALL) -d $(PREFIX)/man/man1
	$(INSTALL) grip.1 $(PREFIX)/man/man1
	$(INSTALL) grip.1 $(PREFIX)/man/man1/gcd.1

gcdinstall:
	$(INSTALL) -d $(INSTALLDIR)
	$(INSTALL) gcd $(INSTALLDIR)
	$(INSTALL) -d $(AUXDIR)
	$(INSTALL) -d $(PREFIX)/man/man1
	$(INSTALL) grip.1 $(PREFIX)/man/man1/gcd.1

# Source distribution
srcdist:	grip.1
	-rm -rf grip-$(VERSION)
	-mkdir grip-$(VERSION)
	cp grip.c grip-$(VERSION)
	cp grip.h grip-$(VERSION)
	cp cddb.c grip-$(VERSION)
	cp cd.c grip-$(VERSION)
	cp id3.c grip-$(VERSION)
	cp bug.c grip-$(VERSION)
	cp parsecfg.c grip-$(VERSION)
	cp parsecfg.h grip-$(VERSION)
	cp cdpar.c grip-$(VERSION)
	cp xpm.h grip-$(VERSION)
	cp config.h grip-$(VERSION)
	-mkdir grip-$(VERSION)/dialog
	cp dialog/message.c grip-$(VERSION)/dialog
	cp dialog/input.c grip-$(VERSION)/dialog
	cp dialog/dialog.h grip-$(VERSION)/dialog
	cp README grip-$(VERSION)
	cp CHANGES grip-$(VERSION)
	cp CREDITS grip-$(VERSION)
	cp LICENSE grip-$(VERSION)
	cp TODO grip-$(VERSION)
	cp grip.pod grip-$(VERSION)
	cp grip.1 grip-$(VERSION)
	cp Makefile grip-$(VERSION)
	cp -R pixmaps grip-$(VERSION)
	cp gripicon.tif grip-$(VERSION)
	cp gcdicon.tif grip-$(VERSION)
	cp gripicon.gif grip-$(VERSION)
	cp gcdicon.gif grip-$(VERSION)
	cp grip.spec grip-$(VERSION)
	tar -czf grip-$(VERSION).tgz grip-$(VERSION)

pardist: srcdist
	-cd cdparanoia && make distclean
	cd grip-$(VERSION) && ln -sf ../cdparanoia
	tar -czhf grip-$(VERSION).tgz grip-$(VERSION)
	rm grip-$(VERSION)/cdparanoia

# Redhat RPMs for the Grip and GCD distributions
rpm:	pardist
	cp grip-$(VERSION).tgz /usr/src/redhat/SOURCES
	cp gripicon.gif /usr/src/redhat/SOURCES
	cp gcdicon.gif /usr/src/redhat/SOURCES
	cp grip.spec grip-$(VERSION).spec
	chown root.root grip-$(VERSION).spec
	rpm -ba grip-$(VERSION).spec


# Tidy up after ourselves
clean:
	-rm -rf grip gcd *.o grip-* gcd-* *~
