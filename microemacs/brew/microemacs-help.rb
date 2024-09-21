
class MicroemacsHelp < Formula
  desc "MicroEmacs 24 Text Editor Help file"
  homepage "https://github.com/bjasspa/jasspa"
  sha256 "3CC3172C3B8F2EA0104ECC2E23D27D92E80DD0873285BDED7CEBD53D26D8ECF3"
  version "20240903"
  url "https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_help_ehf.zip"
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
    puts "start Microemacs with: MEPATH=~/.config/jasspa:/home/linuxbrew/.linuxbrew/share/jasspa/macros:/home/linuxbrew/.linuxbrew/share/jasspa/spelling mec (or mew)"
    puts "on MacOS replace /home/linuxbrew/.linuxbrew with /opt/homebrew for M1 Macs or /usr/local for Intel Macs"
  end

end
