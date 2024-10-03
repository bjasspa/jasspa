# -!- ruby -!-
class MicroemacsOpenssl < Formula
  desc "Jasspa MicroEmacs Text Editor - Open SSL libraries"
  homepage "https://github.com/bjasspa/jasspa"
  version "##VERSION##"
  SHRPTH="#{HOMEBREW_PREFIX}/share"
  URLPFX="https://github.com/bjasspa/jasspa/releases/download/me_##VERSION##"
  depends_on "microemacs-binaries"
  if OS.linux?
    url "#{URLPFX}/Jasspa_MicroEmacs_##VERSION##_bin_linux_openssl.zip"
    sha256 "##SHA256##"
  elsif OS.mac?
    if Hardware::CPU.arm?
      # Code for Apple Silicon (M1, M2, etc.)
      url "#{URLPFX}/Jasspa_MicroEmacs_##VERSION##_bin_macos_apple_openssl.zip"
      sha256 "##SHA256##"
    elsif Hardware::CPU.intel?
      url "#{URLPFX}/Jasspa_MicroEmacs_##VERSION##_bin_macos_intel_openssl.zip"
      sha256 "##SHA256##"
    else
      odie "Unexpected macOS Hardware, not arm or intel!"
    end
  else
    odie "Unexpected OS, not linux or macos!"
  end

  def install
    if OS.linux?
      lib.install "linux5-intel64/libcrypto.so"
      lib.install "linux5-intel64/libssl.so"          
    elsif Hardware::CPU.arm?
      lib.install "macos14-apple64/libssl.3.dylib"
      lib.install "macos14-apple64/libcrypto.3.dylib"
    elsif Hardware::CPU.intel?
      lib.install "macos13-intel64/libssl.3.dylib"
      lib.install "macos13-intel64/libcrypto.3.dylib"
    end
  end
  
end
