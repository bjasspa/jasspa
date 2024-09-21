
class MicroemacsSpellingItit < Formula
  desc "MicroEmacs 24 Text Editor Spelling Rules Italian"
  homepage "https://github.com/bjasspa/jasspa"
  url "https://github.com/bjasspa/jasspa/releases/download/me_20240903/Jasspa_MicroEmacs_20240903_spelling_itit.zip"
  sha256 "BA59AFBCA245FC4F15979EF89034C1593EF08BF4EDC3B4E19E2B64C6B0077628"
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
