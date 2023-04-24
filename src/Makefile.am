
bin_PROGRAMS = fract

fract_CPPFLAGS = $(GLIBMM_CFLAGS) $(GTKMM_CFLAGS) $(GTHREAD_CFLAGS)
fract_CFLAGS = -Wall
fract_CXXFLAGS = -Wall
fract_LDADD = $(GLIBMM_LIBS) $(GTKMM_LIBS) $(GTHREAD_LIBS)

if RELEASE
## this does not conform to automake standards but speeds up computation considerably
fract_CXXFLAGS += -O2
endif

fract_SOURCES = FractApp.cpp \
	FractWin.cpp \
	FractView.cpp \
	Param.cpp \
	ParamDlg.cpp \
	Worker.cpp \
	resources.c \
	FractView.h \
	ParamDlg.h \
	Param.h \
	Worker.h \
	FractApp.h \
	FractWin.h
