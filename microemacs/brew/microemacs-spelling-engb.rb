
class MicroemacsSpellingEngb < Formula
  desc "MicroEmacs 24 Text Editor Spelling Rules British"
  homepage "https://github.com/bjasspa/jasspa"
  url "https://github.com/bjasspa/jasspa/releases/download/me_20240903/Jasspa_MicroEmacs_20240903_spelling_engb.zip"
  sha256 "35AF8D378BD0E895858233F8D651022C5BDA0BC1482018FAED305F742EBC4C8C"
  version "20240903"
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