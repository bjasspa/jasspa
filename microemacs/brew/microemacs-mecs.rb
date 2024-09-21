#!/usr/bin/env ruby
class MicroemacsMecs < Formula
  desc "MicroEmacs 24 Text Editor Standalone Terminal Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "20240903"
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_abin_"
  depends_on "luit"
  depends_on "abduco"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_mecs.zip"
          sha256 "0F6279D26443506F721C88B2745ED65E0FE000BEBE9C163689B0B387DF258292"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_mecs.zip"
          sha256 "BCE548C11F5B5FBDF98EA25285E4746CB806FA800CA3D3964F8F1B4BFA099BC0"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_mecs.zip"
    sha256 "4ABD118A3FA20ECE663D2F1BD4A3D4295A0243D43E6B2B3B76FD4ADC8C9370EC"
  elsif OS.windows?
      url "#{PREFIX}windows_mecs.zip"
    sha256 "40AD16AA27E1A35856D4B048D2A28F5895708D0ED1C88A4B89470DC899F5B252"
  end

  def install
      if OS.linux?
          bin.install "bin/linux5-intel64/mecs"
      elsif OS.mac? && Hardware::CPU.arm?
          bin.install "bin/macos14-apple64/mecs"
      elsif OS.mac? && Hardware::CPU.intel?
          bin.install "bin/macos14-intel64/mecs"
      elsif OS.windows?
          bin.install "bin/windows100-intel32/mecs"
      end
  end
  
  def caveats 
      <<~EOS
        This application is better working if you install tools like
        luit and abduco, which you can install it with:
        brew install abduco luit
      EOS
  end
  
  test do
    assert_match "MicroEmacs 24 - Date 2024/09/02 - linux", shell_output("#{bin}/mecs -V")
  end
end
