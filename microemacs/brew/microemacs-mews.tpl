#!/usr/bin/env ruby
class MicroemacsMews < Formula
  desc "MicroEmacs 24 Text Editor GUI Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "__version__"
  on_macos do
    depends_on "xquartz"
    depends_on "libx11"
    depends_on "libxext"
    depends_on "libxaw"
    depends_on "libxt"
  end  
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_abin_"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_mews.zip"
          sha256 "__sha_macos_apple__"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_mews.zip"
          sha256 "__sha_macos_intel__"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_mews.zip"
    sha256 "__sha_linux__"
  elsif OS.windows?
      url "#{PREFIX}windows_mews.zip"
    sha256 "__sha_windows__"
  end

  def install
      if OS.linux?
          bin.install "bin/linux5-intel64/mews"
      elsif OS.mac? && Hardware::CPU.arm?
          bin.install "bin/macos14-apple64/mews"
      elsif OS.mac? && Hardware::CPU.intel?
          bin.install "bin/macos14-intel64/mews"
      elsif OS.windows?
          bin.install "bin/windows100-intel32/mews"
      end
  end
  def caveats
      <<~EOS  
         On MacOS:
         Before you can open the X-Window version please 
         open X11 with this command in a terminal:
         open -a XQuartz
      EOS
  end
  test do
    assert_match "MicroEmacs 24 - Date 2024/09/02 - linux", shell_output("#{bin}/mews -V")
  end
end
