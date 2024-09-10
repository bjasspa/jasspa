
class MicroemacsMec < Formula
  desc "MicroEmacs 24 Text Editor Terminal Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "20240902"
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_bin_"
  depends_on "luit"
  depends_on "abduco"
  depends_on "microemacs-macros"
  depends_on "microemacs-help"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_binaries.zip"
          sha256 "0F08317178A134F9D4ED4FE0C144233411CAB2E7228EC4ACB61EB31DF67E3D34"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_binaries.zip"
          sha256 "C6582CDF9160242C4F8C1B87C526DC791F428D3D4622C92381932965F56B93D3"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_binaries.zip"
    sha256 "3767B420124A960C1321D050DC5700172C54751D9B0B658AA7FF2642893CDCBB"
  elsif OS.windows?
      url "#{PREFIX}windows_binaries.zip"
    sha256 "DC6318FA50750A14D646FA2E50EA9A6A3F3D8B81618943ED42B5891F5DCAF40A"
  end

  def install
      if OS.linux?
          bin.install "bin/linux5-intel64/mec"
      elsif OS.mac? && Hardware::CPU.arm?
          bin.install "bin/macos14-apple64/mec"
      elsif OS.mac? && Hardware::CPU.intel?
          bin.install "bin/macos14-intel64/mec"
      elsif OS.windows?
          bin.install "bin/windows100-intel32/mec.exe"
      end
      puts "start Microemacs with: MEPATH=~/.config/jasspa:#{share}/jasspa/macros:#{share}/jasspa/spelling mec"      
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
