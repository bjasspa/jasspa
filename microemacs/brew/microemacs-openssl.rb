
class MicroemacsOpenssl < Formula
  desc "MicroEmacs 24 Text Editor Open SSL libraries"
  homepage "https://github.com/bjasspa/jasspa"
  version "20240903"
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_bin_"
  depends_on "microemacs-mec"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_openssl.zip"
          sha256 "9B180CCE2E28EC3432B81A48B94EB35B713D184486E4B917D6329729F3C92FD7"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_openssl.zip"
          sha256 "3A6743DF66ACF845E3A515A333510E6AF57F189CDB26AF95C43DA058BCB5E63B"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_openssl.zip"
    sha256 "C0167D5D6C1F8C7079A1600904B02DB1372514A56FD33F75380C1ACFF8B135D4"
  elsif OS.windows?
      url "#{PREFIX}windows_openssl.zip"
    sha256 "4DDC326CCED0B6B2807BBA0D47FC39824EFD78DC707D0CFA7461845AB7CC38D2"
  end

  def install
      if OS.linux?
          lib.install "linux5-intel64/libcrypto.so"
          lib.install "linux5-intel64/libssl.so"          
      elsif OS.mac? && Hardware::CPU.arm?
          lib.install "macos14-apple64/libssl.3.dylib"
          lib.install "macos14-apple64/libcrypto.3.dylib"
      elsif OS.mac? && Hardware::CPU.intel?
          lib.install "macos13-intel64/libssl.3.dylib"
          lib.install "macos13-intel64/libcrypto.3.dylib"
      elsif OS.windows?
          lib.install "windows100-intel32/libcrypto-3.dll"
          lib.install "bin/windows100-intel32/libssl-3.dll"          
      end
      puts "start Microemacs with: MEPATH=~/.config/jasspa:${share}/jasspa/macros:${share}/jasspa/spelling mew"      
  end
  
  def caveats 
      <<~EOS
        This application is better working if you install tools like
        xfontscale to select X11 TrueType fonts
      EOS
  end
  
  test do
    assert_match "MicroEmacs 24 - Date 2024/09/02 - linux", shell_output("#{bin}/mew -V")
  end
end
