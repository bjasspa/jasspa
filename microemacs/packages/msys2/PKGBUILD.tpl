# Maintainer: Detlef Groth <your@email.com>

pkgname=microemacs
pkgver=__VERSION__
pkgrel=1
pkgdesc="Jasspa MicroEmacs Text Editor"
arch=('x86_64')
url="https://github.com/bjasspa/jasspa"
license=('GPL')
source=("mesc.exe" "mesw.exe" "readme.txt" "LICENSE")
sha256sums=(
    'digest-mesc.exe'
    'digest-mesw.exe'
    'digest-readme.txt'
    'digest-LICENSE')

options=("!strip")
package() {
   install -Dm755 "${srcdir}/mesc.exe"    "${pkgdir}/usr/bin/mesc.exe"
   install -Dm755 "${srcdir}/mesw.exe"    "${pkgdir}/usr/bin/mesw.exe"   
   install -Dm644 "${srcdir}/readme.txt"  "${pkgdir}/usr/share/jasspa/readme.txt"
   install -Dm644 "${srcdir}/LICENSE" "${pkgdir}/usr/share/jasspa/LICENSE"
}
