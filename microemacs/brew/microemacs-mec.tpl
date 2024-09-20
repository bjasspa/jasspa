
class MicroemacsMec < Formula
  desc "MicroEmacs 24 Text Editor Terminal Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "__version__"
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_bin_"
  depends_on "luit"
  depends_on "abduco"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_binaries.zip"
          sha256 "__sha_macos_apple__"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_binaries.zip"
          sha256 "__sha_macos_intel__"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_binaries.zip"
    sha256 "__sha_linux__"
  elsif OS.windows?
      url "#{PREFIX}windows_binaries.zip"
    sha256 "__sha_windows__"
  end

  def install
      if OS.linux?
          bin.install "bin/linux5-intel64/mec"
      elsif OS.mac? && Hardware::CPU.arm?
          bin.install "bin/macos14-apple64/mec"
      elsif OS.mac? && Hardware::CPU.intel?
          bin.install "bin/macos13-intel64/mec"
      elsif OS.windows?
          bin.install "bin/windows100-intel32/mec.exe"
      end
      puts "start Microemacs with: MEPATH=~/.config/jasspa:/home/linuxbrew/.linuxbrew/share/jasspa/macros:/home/linuxbrew/.linuxbrew/share/jasspa/spelling mec"
      puts "on MacOS replace /home/linuxbrew/.linuxbrew with /opt/homebrew for M1 Macs or /usr/local for intel Macs"
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
