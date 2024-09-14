
class MicroemacsHelp < Formula
  desc "MicroEmacs 24 Text Editor Help file"
  homepage "https://github.com/bjasspa/jasspa"
  sha256 "CC95587293B33B0805906D1143CF473F419325F493E146C695C6325A45D56D27"
  version "20240902"
  url "https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_help_ehf.zip"
  depends_on "microemacs-mec"
  depends_on "microemacs-macros"

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
    puts "start Microemacs with: MEPATH=~/.config/jasspa:#{share}/jasspa/macros:${share}/jasspa/spelling mec"
  end

end
