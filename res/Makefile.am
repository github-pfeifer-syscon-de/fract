PKGCONFIG = pkgconf
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)

FRACT_RESOURCE_XML=fract.gresources.xml
FRACT_RESOURCES=$(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies $(FRACT_RESOURCE_XML))

all:../src/resources.c

../src/resources.c: $(FRACT_RESOURCE_XML) $(FRACT_RESOURCES) 
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(dir $<) --generate-source $<

# The desktop files
desktopdir = $(datadir)/applications
desktop_in_files=fract.desktop.in
desktop_DATA=$(desktop_in_files:.desktop.in=.desktop)

# application icon
appicondir = $(datadir)/icons/hicolor/64x64/apps
appicon_DATA = fract.png
appdir_icon=$(appicondir)/$(appicon_DATA)

clean-local:
	-rm -rf $(desktop_DATA)
	
EXTRA_DIST = $(FRACT_RESOURCES) $(FRACT_RESOURCE_XML) $(desktop_in_files) $(appicon_DATA)

%.desktop: %.desktop.in
	@regex_cmd@ -e "s,\@APP_ICON\@,$(appdir_icon),g" $? > $@
