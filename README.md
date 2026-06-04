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

## Usage

- All options are shown in a context menu with the right mouse click
- To zoom just press the left mouse button and drag a rectangle for the next view

## Life

Also available a "conways game of life" implementation.
With the option to use .life (1.05,1.06),.cell and .rle formated links 
from the web or paste cell patterns into the program.

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

There are two choices:
1. Use a download:

Download Gtk-3 release from 

<pre>
 https://github.com/wingtk/gvsbuild/releases/
</pre>

Modify the env as suggested on the main page, but add:
<pre>
set PKG_CONFIG_PATH=C:\GTK_INSTALL\lib\pkgconfig
</pre>

Get a recent Version >= 3.14 of python from python.org.

Open a command-line, install meson:
<pre>
pip install meson
</pre>

For this choice, you have to remove the libsoup dependency in meson.build, 
comment the referenes in LifeQueryDialog, the major function will still be available.

2. Second choice build it:

If you want the dependencies follow the "Build GTK" lead

<pre>
uv run gvsbuild build gtk3 gtkmm3 libsoup3
</pre>

The following will be true for both choices:

The meson script uses a library for the life-sources that will not work for
Visual-Studio&copy; so include the sources directly, and comment the test directory 
in meson.build.

Create a solution:
<pre>
meson setup buildVS -Dbackend=vs2022 -Dprefix=YOUR_PREFEED_PROGRAM_LOCATION
</pre>

