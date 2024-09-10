
class MicroemacsMacros < Formula
  desc "MicroEmacs 24 Text Editor Spelling Rules American"
  homepage "https://github.com/bjasspa/jasspa"
  url "https://github.com/bjasspa/jasspa/releases/download/me_20240902/Jasspa_MicroEmacs_20240902_macros.zip"
  sha256 "BDB62B43AC4B948AA641C372D6BA9DD861C80DFB29B88AC6A5EB69862B3D2F9F"
  version "20240902"
  depends_on "microemacs-mec"

  def install
    # Define the target directory
    macrofolder = "#{share}/jasspa/macros"
    # Create the directory if it does not exist
    require 'fileutils'
    FileUtils.mkdir_p(macrofolder) unless Dir.exist?(macrofolder)
    # Example: List files in the buildpath
    Dir.glob("#{buildpath}/macros/*").each do |file|
        puts "Found file: #{file}"
        cp "#{file}", macrofolder
    end  
    ### setting MEPATH for mews and mecs does not work, what could we do?
    puts "start Microemacs with: MEPATH=~/.config/jasspa:/home/linuxbrew/.linuxbrew/share/jasspa/:/home/linuxbrew/.linuxbrew/share/jasspa/spelling mews"
  end

end
