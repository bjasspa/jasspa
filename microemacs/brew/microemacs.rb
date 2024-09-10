
class Microemacs < Formula
  desc "MicroEmacs 24 Text Editor"
  homepage "https://github.com/bjasspa/jasspa"
  url "https://github.com/bjasspa/jasspa/releases/download/me_20240902/Jasspa_MicroEmacs_20240902_help_ehf.zip"
  sha256 "CC95587293B33B0805906D1143CF473F419325F493E146C695C6325A45D56D27"
  version "20240902"
  depends_on "microemacs-mec"
  depends_on "microemacs-mew"  

  def install
    # Define the target directory
    (share/"jasspa").mkpath
    # Install the license and README files
    cp "COPYING.txt", share/"jasspa"
    cp "readme.txt", share/"jasspa"
    # Install help file
    cp "macros/me.ehf", share/"jasspa"
    # Clean up the downloaded ZIP file
  end

end
