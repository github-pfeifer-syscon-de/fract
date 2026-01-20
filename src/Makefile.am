
bin_PROGRAMS = fract

fract_LDFLAGS = @EXTRA_LDFLAGS@

# this is a no-tech approach,
#  if you don't like gcc/clang, it this should also work without these options
#  most options originate from https://github.com/cpp-best-practices/cmake_template/blob/main/cmake/Hardening.cmake
#    -W... the usual warings
#    GLIBC++ Assertions (vector[], string[], ...) enabled
#    g++/clang -fstack-protector-strong enabled
#    g++/clang -fcf-protection enabled
#    undefined behaviour sanitizer
if RELEASE
fract_CXXFLAGS = -mtune=native -march=native -O3
else
fract_CFLAGS = -Wall
fract_CXXFLAGS = -Wall \
        -Wconversion \
        -Wpedantic \
        -D_GLIBCXX_ASSERTIONS \
        -fstack-protector-strong \
        -fcf-protection \
        -fsanitize=undefined
fract_LDFLAGS += -fsanitize=undefined
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
