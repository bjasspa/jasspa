#!/usr/bin/env ruby
class MicroemacsMews < Formula
  desc "MicroEmacs 24 Text Editor GUI Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "20240903"
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
          sha256 "73D03DDBF1D39C1B40FCAA5D143B8D9FE70DADC2FB0508866AD12308E088B1D2"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_mews.zip"
          sha256 "84E78BA0B30CA8CFE7B2FEB432A548F81EB2177395754CBBD3B511146398FCD8"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_mews.zip"
    sha256 "48CC60861C6B9F30BC6F0B1740CB00FFC0911F4CE5D8D480A1C6BAAED49DAC22"
  elsif OS.windows?
      url "#{PREFIX}windows_mews.zip"
    sha256 "7E1EAA8E749D324A3635AA9E733547DC4BE195D5CFD0C4B62FDDFC3B7AF8E1FD"
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
