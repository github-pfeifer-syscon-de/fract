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

The tool that makes the following happen is:
<pre>
 https://github.com/wingtk/gvsbuild
</pre>

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

Bring some gigabytes of storage and hours of time.

If you want the dependencies follow the "Build GTK" lead, then

<pre>
uv run gvsbuild build gtk3 
uv run gvsbuild build gtkmm3 
</pre>

- for libsoup3 the VisualC&copy; 2013 runtime is required download it from: 
https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#visual-studio-2013-vc-120
and install it
- for libsoup3 a py executable is expected so copy in python bin directory python.exe to py.exe

<pre>
uv run gvsbuild build libsoup3
</pre>

The following will be true for both choices:

The meson script uses a library for the life-sources that will not work for
Visual-Studio&copy; so include the sources directly, and comment the test directory 
in meson.build.

Create a solution:
<pre>
meson setup buildVS -Dbackend=vs2022 -Dprefix=YOUR_PREFEED_PROGRAM_LOCATION
</pre>

If the build complains about the fontconfig include is missing, this is due to fact that the include directories
are set for each package, but the common include is missing, edit the fract@exe.vcxproj
in AdditionalIncludeDirectories add the common gvs include directory (the first listed in INCLUDE=).
