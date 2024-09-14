# Maintainer: Detlef Groth <your@email.com>

pkgname=microemacs
pkgver=20240902
pkgrel=1
pkgdesc="Jasspa MicroEmacs Text Editor"
arch=('x86_64')
url="https://github.com/bjasspa/jasspa"
license=('GPL')
source=("mecs.exe" "mews.exe" "readme.txt" "COPYING.txt")
sha256sums=(
    'digest-mecs.exe'
    'digest-mews.exe'
    'digest-readme.txt'
    'digest-COPYING.txt')

options=("!strip")
package() {
   install -Dm755 "${srcdir}/mecs.exe"    "${pkgdir}/usr/bin/mecs.exe"
   install -Dm755 "${srcdir}/mews.exe"    "${pkgdir}/usr/bin/mews.exe"   
   install -Dm644 "${srcdir}/readme.txt"  "${pkgdir}/usr/share/jasspa/readme.txt"
   install -Dm644 "${srcdir}/COPYING.txt" "${pkgdir}/usr/share/jasspa/COPYING.txt"
}
