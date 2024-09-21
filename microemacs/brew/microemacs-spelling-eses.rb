
class MicroemacsSpellingEses < Formula
  desc "MicroEmacs 24 Text Editor Spelling Rules Spanish"
  homepage "https://github.com/bjasspa/jasspa"
  url "https://github.com/bjasspa/jasspa/releases/download/me_20240903/Jasspa_MicroEmacs_20240903_spelling_eses.zip"
  sha256 "0FD68355D0B3A34893DA315DA3D4088CDD4E16AA27D183A07BECF958E8319A2B"
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
