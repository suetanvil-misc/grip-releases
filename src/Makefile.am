
INCLUDES = -I$(top_srcdir) -I$(includedir) $(GNOME_INCLUDEDIR) \
         -DG_LOG_DOMAIN=\"grip\" -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I../intl -I$(top_srcdir)/intl

bin_PROGRAMS = grip

grip_SOURCES =         \
        main.c           \
	common.h \
        grip.c \
	grip.h \
	cddev.c	\
	cddev.h \
	cdplay.c \
	cdplay.h \
	discedit.c \
	discedit.h \
	rip.c \
	rip.h \
	cdpar.c \
	id3.c \
	grip_id3.h \
	discdb.c \
	discdb.h \
	uihelper.c \
	xpm.h \
	dialog.c \
	dialog.h \
	gripcfg.c \
	gripcfg.h \
	parsecfg.c \
	parsecfg.h \
	launch.c \
	launch.h

grip_LDADD = $(GNOMEUI_LIBS) $(GNOME_LIBDIR) $(INTLLIBS) $(PTHREAD_LIB) \
	$(GHTTP_LIB) $(CDPAR_LIBS) $(ID3_LIBS)
