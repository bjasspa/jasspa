
class MicroemacsMec < Formula
  desc "MicroEmacs 24 Text Editor Terminal Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "20240903"
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_bin_"
  depends_on "luit"
  depends_on "abduco"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_binaries.zip"
          sha256 "840E128556F9D772FCF88CBFFACA1A026C05041FD6B168C6ACF9DDA9B8259D3D"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_binaries.zip"
          sha256 "F358F6CB3F738D7FD7AEF4C844F3A346A43DDFA06B0716D96772D899DE2F23A4"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_binaries.zip"
    sha256 "22F436BE7E400BF56AAE3DC714494ED98413F243E02AED59DDB227F6BA424196"
  elsif OS.windows?
      url "#{PREFIX}windows_binaries.zip"
    sha256 "486C9CB70A79D4B82501482A389D0B68BBB9765767C0D91DD54B6B8E4998371D"
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
