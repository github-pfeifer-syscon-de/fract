# fract

A fractal generator with various options for
mandelbrot-, julia- and newton-sets.
And a improved rendering with antialiasing.

![fract](fract.png "fract")

## Building


### Debian

Use the following commands to get the prerequisits (run with sudo or as root):
<pre>
apt-get install git build-essential meson
apt-get install libgtkmm-3.0-dev
apt-get install libsoup-3.0-dev
</pre>

The compile goes this way:
<pre>
   meson setup build -Dprefix=/usr
   cd build
   meson compile
   ./fract
</pre>

If you decide to keep it (run with sudo or as root):
<pre>
   cd build
   meson install
</pre>

### Windows

Use msys2 choose a preferred flavor and stick to it (a bit more ist explained with genericImg):
<pre>
pacman -S base-devel
pacman -S ${MINGW_PACKAGE_PREFIX}-gcc
pacman -S ${MINGW_PACKAGE_PREFIX}-meson
pacman -S ${MINGW_PACKAGE_PREFIX}-gtkmm3
pacman -S ${MINGW_PACKAGE_PREFIX}-libsoup-3.0
</pre>

The compile goes this way:
<pre>
   meson setup build -Dprefix=${MINGW_PREFIX}
   cd build
   meson compile
   ./fract.exe
</pre>


#### Visual-Studio&copy;

The following is experimental, it's doable, but you have tweak some pieces, and some experience with VS you may get this to work :

Download/build Gtk-3 release from ???

 https://github.com/wingtk/gvsbuild/releases/

https://gitlab.gnome.org/GNOME/libsoup

Modify the env as suggested on the main page, but add:
<pre>
set PKG_CONFIG_PATH=C:\GTK_INSTALL\lib\pkgconfig
</pre>

Get a recent Version >= 3.14 of python from python.org.

Open a command-line, install meson:
<pre>
pip install meson
</pre>

As the managment of the depencies is possible,
if you have some experience with Opensource projects,
but its a chain (libsoup needs libnghttp needs sqllite) that does not end quickly,
so remove the libsoup dependency at least for a start, 
comment the referenes in LifeQueryDialog.

The meson script uses a library for the life-sources that will not work for
Visual-Studio&copy; so include the sources directly, and comment the test directory 
in meson.build.

Create a solution:
<pre>
meson setup buildVS -Dbackend=vs2026 -Dprefix=YOUR_PREFEED_PROGRAM_LOCATION
</pre>

## Usage

- All options are shown in a context menu with the right mouse click
- To zoom just press the left mouse button and drag a rectangle for the next view

## Life

Also available a "conways game of life" implementation.
With the option to use .life (1.05,1.06),.cell and .rle formated links 
from the web or paste cell patterns into the program.
