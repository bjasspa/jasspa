
class MicroemacsSpellingPtpt < Formula
  desc "MicroEmacs 24 Text Editor Spelling Rules Portuguese"
  homepage "https://github.com/bjasspa/jasspa"
  url "https://github.com/bjasspa/jasspa/releases/download/me_20240902/Jasspa_MicroEmacs_20240902_spelling_ptpt.zip"
  sha256 "8F44946B2AE80A9B82808AE0CD5910A75402E5E7E80FA8963FEF11C27A7F65F4"
  version "20240902"
  depends_on "microemacs-macros"
  def install
    # Define the target directory
    spellfolder = "#{share}/jasspa/spelling"
    # Create the directory if it does not exist
    require 'fileutils'
    FileUtils.mkdir_p(spellfolder) unless Dir.exist?(spellfolder)
    # Example: List files in the buildpath
    Dir.glob("#{buildpath}/*").each do |file|
        puts "Found file: #{file}"
        cp "#{file}", spellfolder
    end  
    ### setting MEPATH for mews and mecs does not work, what could we do?
    puts "start Microemacs with: MEPATH=~/.config/jasspa:/home/linuxbrew/.linuxbrew/share/jasspa/:/home/linuxbrew/.linuxbrew/share/jasspa/spelling mews"
  end

end
