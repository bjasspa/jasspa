# Maintainer: Detlef Groth <your@email.com>

pkgname=microemacs
pkgver=20240902
pkgrel=1
pkgdesc="Jasspa MicroEmacs Text Editor"
arch=('x86_64')
url="https://github.com/bjasspa/jasspa"
license=('GPL')
source=("mesc.exe" "mesw.exe" "readme.txt" "COPYING.txt")
sha256sums=(
    'digest-mesw.exe'
    'digest-mesw.exe'
    'digest-readme.txt'
    'digest-COPYING.txt')

options=("!strip")
package() {
   install -Dm755 "${srcdir}/mesc.exe"    "${pkgdir}/usr/bin/mesc.exe"
   install -Dm755 "${srcdir}/mesw.exe"    "${pkgdir}/usr/bin/mesw.exe"   
   install -Dm644 "${srcdir}/readme.txt"  "${pkgdir}/usr/share/jasspa/readme.txt"
   install -Dm644 "${srcdir}/COPYING.txt" "${pkgdir}/usr/share/jasspa/COPYING.txt"
}
