#!/usr/bin/env ruby
class MicroemacsMecs < Formula
  desc "MicroEmacs 24 Text Editor Standalone Terminal Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "__version__"
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_abin_"
  depends_on "luit"
  depends_on "abduco"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_mecs.zip"
          sha256 "__sha_macos_apple__"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_mecs.zip"
          sha256 "__sha_macos_intel__"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_mecs.zip"
    sha256 "__sha_linux__"
  elsif OS.windows?
      url "#{PREFIX}windows_mecs.zip"
    sha256 "__sha_windows__"
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
