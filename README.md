# fract

A fractal generator with various options for
mandelbrot-, julia- and newton-sets.
And a improved rendering with antialiasing.

![fract](fract.png "fract")

The usual autotool procedure should work (expects gcc/clang? otherwise see Building):

For a out of tree compile this is the procedure after cloning:
<pre>
   autoreconf -fis
   mkdir build
   cd build
   ../configure --prefix=/usr
   make
   ./src/fract
</pre>

## Usage

- All options are shown in a context menu with the right mouse click
- To zoom just press the left mouse button and drag a rectangle for the next view

## Building

This comes with ignoring some of the autotools best practices, 
the compile options for gcc/clang are integrated in Makefiles.
- as optimization is required for this kind of program
- handling of compile options may not be easy for everyone trying this
- i'm too lazy to fill in debug settings (use `configure ... ---enable-debug=yes` to get them)
- the autotools offer not many options to make compile options dynamic (at least not without m4 macro magic)
- sorry all those no gcc friends see Makefile.am (windows gui switch) and also src/Makefile.am (optimization-/debug options) and adapt these to your compiler