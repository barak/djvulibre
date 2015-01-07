all: icons register-djvu-mime 

#icons

PNGICONS = \
hi16-djvu.png \
hi20-djvu.png \
hi22-djvu.png \
hi24-djvu.png \
hi32-djvu.png \
hi48-djvu.png \
hi64-djvu.png \
hi72-djvu.png \
hi96-djvu.png \
hi128-djvu.png \
hi256-djvu.png

icons: $(PNGICONS) hi-djvu.svgz

if HAVE_CONVERSION_RSVG
convert_icons_process = \
s=`echo $@ | sed -e 's/[a-z]*\([0-9]*\).*/\1/'`; \
${RSVG} \
-w $${s} \
-h $${s} -o $@ $<
endif

if HAVE_CONVERSION_INKSCAPE
convert_icons_process = \
s=`echo $@ | sed -e 's/[a-z]*\([0-9]*\).*/\1/'`; \
${INKSCAPE} \
--without-gui \
--export-width=$${s} \
--export-height=$${s} \
--export-png=$@ $<
endif

if HAVE_CONVERSION_CONVERT
convert_icons_process = \
s=`echo $@ | sed -e 's/[a-z]*\([0-9]*\).*/\1/'`; \
${CONVERT} \
-geometry $${s}x$${s} \
-depth 8 \
-background none $< $@
endif

png_verbose = $(png_verbose_@AM_V@)
png_verbose_ = $(png_verbose_@AM_DEFAULT_V@)
png_verbose_0 = @echo "  PNG     " $@;

${PNGICONS}: djvu.svg Makefile
if HAVE_CONVERSION_TOOL
	$(png_verbose)$(convert_icons_process)
else
	$(AM_V_at)cp ${top_srcdir}/desktopfiles/prebuilt-$@ $@
endif

svg_process =\
cat $< | gzip >$@

svg_verbose = $(svg_verbose_@AM_V@)
svg_verbose_ = $(svg_verbose_@AM_DEFAULT_V@)
svg_verbose_0 = @echo "  SVG     " $@;

hi-djvu.svgz: djvu.svg Makefile
	$(AM_V_at)rm -f $@
	$(svg_verbose)$(svg_process)

# mime

sed_mime_process = \
$(SED) \
-e 's,XDG_MIME,$(XDG_MIME),g' \
-e 's,XDG_ICON_RESOURCE,$(XDG_ICON_RESOURCE),g' \
-e 's,XDG_DESKTOP_MENU,$(XDG_DESKTOP_MENU),g' \
-e 's,DATADIR,$(datadir),g' \
-e 's,MYDIR,$(djvu_datadir),g' \
< $< > $@ || rm $@

mime_verbose = $(mime_verbose_@AM_V@)
mime_verbose_ = $(mime_verbose_@AM_DEFAULT_V@)
mime_verbose_0 = @echo "  MIME    " $@;

register-djvu-mime: register-djvu-mime.in Makefile
	$(AM_V_at)rm -f $@
	$(mime_verbose)$(sed_mime_process)
	$(AM_V_at)chmod a+x register-djvu-mime

djvu_datadir = $(datadir)/djvu/osi/desktop
djvu_data_DATA = \
register-djvu-mime \
hi-djvu.svgz \
djvulibre-mime.xml \
$(PNGICONS)

EXTRA_DIST = \
README \
djvu.svg \
djvulibre-mime.xml \
register-djvu-mime.in \
prebuilt-hi128-djvu.png \
prebuilt-hi16-djvu.png \
prebuilt-hi20-djvu.png \
prebuilt-hi22-djvu.png \
prebuilt-hi24-djvu.png \
prebuilt-hi256-djvu.png \
prebuilt-hi32-djvu.png \
prebuilt-hi48-djvu.png \
prebuilt-hi64-djvu.png \
prebuilt-hi72-djvu.png \
prebuilt-hi96-djvu.png

MOSTLYCLEANFILES = $(PNGICONS) hi-djvu.svgz register-djvu-mime 
