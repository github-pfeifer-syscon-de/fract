# fract

A fractal generator with various options for
mandelbrot-, julia- and newton-sets.
And a improved rendering with antialiasing.

![fract](fract.png "fract")

The usual autotool procedure should work (but optimisation is recommended):

<pre>
   autoreconf -fis
   ./configure --prefix=/usr
   make CXXFLAGS="-mtune=native -march=native -O3"
   ./src/fract
</pre>

All options are shown in a context menu with the right mouse button.

To zoom just press the left mouse button and drag a rectangle for the next view.
