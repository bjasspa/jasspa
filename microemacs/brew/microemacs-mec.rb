
class MicroemacsMec < Formula
  desc "MicroEmacs 24 Text Editor Terminal Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "20240902"
  PREFIX="https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_abin_"
  FILE="Jasspa_MicroEmacs_20240902_abin_"
  if OS.mac?
      if Hardware::CPU.arm?
          # Code for Apple Silicon (M1, M2, etc.)
          url "#{PREFIX}macos_apple_mecs.zip"
          sha256 "40F2329E81E8F768D150FC9C22858DA49DEFF7AD35AE878A5424CFAEAA94F0C2"
      elsif Hardware::CPU.intel?
          url "#{PREFIX}macos_intel_mecs.zip"
          sha256 "D58A17332C5CE464BAD7CE20200CDCF0FDFC74AFA99D236207C2B10917699F92"
      end
  elsif OS.linux?
      url "#{PREFIX}linux_mecs.zip"
    sha256 "9547B9FA8A460DC11DE06FDED6F28BFD0199996C10E840FF7DEA6EE3C37C63CB"
  elsif OS.windows?
      url "#{PREFIX}windows_mecs.zip"
    sha256 "607A995FAAE2202A474F7C249A7245F8A0840D9F4E53CADFC9DBBD819E95A2D7"
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

  test do
    assert_match "MicroEmacs 24 - Date 2024/09/02 - linux", shell_output("#{bin}/mecs -V")
  end
end
