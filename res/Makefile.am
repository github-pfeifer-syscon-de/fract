PKGCONFIG = pkgconf
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)

FRACT_RESOURCE_XML=$(srcdir)/fract.gresources.xml
FRACT_RESOURCES=$(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(FRACT_RESOURCE_XML))
RESOURCE_C=$(top_srcdir)/src/resources.c

all:$(RESOURCE_C)

$(RESOURCE_C): $(FRACT_RESOURCE_XML) $(FRACT_RESOURCES)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(dir $<) --generate-source $<

# The desktop files
desktopdir = $(datadir)/applications
desktop_DATA=fract.desktop

# application icon, using only png, as there is no meaningful svg representation
appicondir=$(datadir)/icons/hicolor/64x64/apps
appicon_DATA=fract.png

clean-local:
	-rm -rf $(RESOURCE_C)

EXTRA_DIST = $(FRACT_RESOURCES) $(FRACT_RESOURCE_XML) $(desktop_DATA) $(appicon_DATA)
