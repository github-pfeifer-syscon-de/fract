
bin_PROGRAMS = fract

fract_LDFLAGS = @EXTRA_LDFLAGS@

fract_CFLAGS = $(WARN_CFLAGS)
fract_CXXFLAGS = $(WARN_CXXFLAGS)

if RELEASE
fract_CXXFLAGS += $(OPTIM_CXXFLAGS)
else
fract_CXXFLAGS += $(INSTUMENT_CXXFLAGS)
fract_LDFLAGS += $(INSTUMENT_LINKFLAGS)
endif

fract_CPPFLAGS = $(GLIBMM_CFLAGS) \
    $(GTKMM_CFLAGS) \
    $(GTHREAD_CFLAGS)
fract_LDADD = $(GLIBMM_LIBS) \
    $(GTKMM_LIBS) \
    $(GTHREAD_LIBS)


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
