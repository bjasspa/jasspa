
class MicroemacsOpenssl < Formula
  desc "MicroEmacs 24 Text Editor Open SSL libraries"
  homepage "https://github.com/bjasspa/jasspa"
  version "__version__"
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_bin_"
  depends_on "microemacs-mec"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_openssl.zip"
          sha256 "__sha_macos_apple__"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_openssl.zip"
          sha256 "__sha_macos_intel__"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_openssl.zip"
    sha256 "__sha_linux__"
  elsif OS.windows?
      url "#{PREFIX}windows_openssl.zip"
    sha256 "__sha_windows__"
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
