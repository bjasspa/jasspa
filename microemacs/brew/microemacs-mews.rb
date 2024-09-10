
class MicroemacsMews < Formula
  desc "MicroEmacs 24 Text Editor GUI Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "20240902"
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
          sha256 "40F2329E81E8F768D150FC9C22858DA49DEFF7AD35AE878A5424CFAEAA94F0C2"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_mews.zip"
          sha256 "CF272913DF93D139A3F3391BDD493D74C856F3B1BBD46D2552D8010365789761"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_mews.zip"
    sha256 "00CB8B47023BFE9BD27326B2E52614FD006F8A3408089613FEE8CE68DF6536B8"
  elsif OS.windows?
      url "#{PREFIX}windows_mews.zip"
    sha256 "041376D26F0D9D651F69AB63CBB199B8F2BD8455405F7F52AAC4952597BA6920"
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
         open X11 with this command:
         open -a XQuartz
      EOS
  end
  test do
    assert_match "MicroEmacs 24 - Date 2024/09/02 - linux", shell_output("#{bin}/mews -V")
  end
end
