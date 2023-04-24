# Maintainer: Rpf <rpf@pfeifer-syscon.de>
pkgname=fract
pkgver=r42.b45137a
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
    cd ${srcdir}/..
#    autoreconf -fis
}

check() {
    cd "${srcdir}/.."
    make -k check
}


build() {
    cd "${srcdir}/.."
    # https://bbs.archlinux.org/viewtopic.php?id=280157 any arch ?
    export DFLAGS='-L-zrelro -L-znow'
    ./configure --prefix=/usr
    make
}

package() {
    cd "${srcdir}/.."
    make DESTDIR="${pkgdir}/" install
}
