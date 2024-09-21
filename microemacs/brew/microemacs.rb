
class Microemacs < Formula
  desc "MicroEmacs 24 Text Editor Meta Package Terminal and GUI Version"
  homepage "https://github.com/bjasspa/jasspa"
  version "20240903"
  url "https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_readme.zip"
  sha256 "38f30e3005aa9bf7bd3c606d7f01fdb79467890219c138d09e2123d12eda75c0"
  # Add more dependencies as needed
  depends_on "microemacs-mec"
  depends_on "microemacs-mew"  
  depends_on "microemacs-help"
  depends_on "microemacs-macros"  

  def install
    jasspafolder = "#{share}/jasspa/"
    # Create the directory if it does not exist
    require 'fileutils'
    FileUtils.mkdir_p(jasspafolder) unless Dir.exist?(jasspafolder)
    # Example: List files in the buildpath
    Dir.glob("#{buildpath}/*").each do |file|
        puts "Found file: #{file}"
        cp "#{file}", jasspafolder
    end  
  end
end
