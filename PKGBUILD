# Maintainer: Rpf <rpf@pfeifer-syscon.de>
pkgname=fract
pkgver=r15.f9b0e25
pkgrel=1
pkgdesc="Calculate fractales"
arch=("x86_64")
url="http://wp11237257.server-he.de/wiki/index.php/Gtk3"
license=('GPL3')
depends=('gtkmm3' )
makedepends=('automake')
provides=()
conflicts=()
replaces=()
options=()
source=('configure.ac')
sha256sums=('SKIP')

pkgver() {
  printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
    cd "${srcdir}/../build"
#    autoreconf -fis
}

check() {
    cd "${srcdir}/../build"
    make -k check
}


build() {
    cd "${srcdir}/../build"
    # https://bbs.archlinux.org/viewtopic.php?id=280157 any arch ?
    #export DFLAGS='-L-zrelro -L-znow'
    ./configure --prefix=/usr
    make CFLAGS="-mtune=native -march=native -O3" CXXFLAGS="-mtune=native -march=native -O3"
}

package() {
    cd "${srcdir}/../build"
    make DESTDIR="${pkgdir}/" install
}
