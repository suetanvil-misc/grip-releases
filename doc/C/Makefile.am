grip_helpdir = $(datadir)/gnome/help/grip/C

grip_help_DATA =         \
        grip.html        \
        topic.dat

SGML_FILES =                    \
        grip.sgml

# files that aren't in a binary/data/library target have to be listed here
# to be included in the tarball when you 'make dist'
EXTRA_DIST = \
	*.png\
        topic.dat               \
        $(SGML_FILES)


## The - before the command means to ignore it if it fails.  that way
## people can still build the software without the docbook tools

all: 

grip.html: grip/grip.html
	-cp grip/grip.html .

grip/grip.html: $(SGML_FILES)
	-db2html grip.sgml

## when we make dist, we include the generated HTML so people don't
## have to have the docbook tools
dist-hook:
	mkdir $(distdir)/grip
	-cp grip/*.html grip/*.css grip/*.png $(distdir)/grip
	-cp grip.html $(distdir)

install-data-local: grip.html
	$(mkinstalldirs) $(grip_helpdir)/images
	-for file in $(srcdir)/grip/*.html $(srcdir)/grip/*.css $(srcdir)/grip/*.png; do \
	basefile=`basename $$file`; \
	$(INSTALL_DATA) $(srcdir)/$$file $(grip_helpdir)/$$basefile; \
	done

grip.ps: grip.sgml
	-db2ps $<

grip.rtf: grip.sgml       
	-db2rtf $<

