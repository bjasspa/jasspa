
class MicroemacsMacros < Formula
  desc "MicroEmacs 24 Text Editor Macro files"
  homepage "https://github.com/bjasspa/jasspa"
  version "__version__"
  url "https://github.com/bjasspa/jasspa/releases/download/me_#{version}/Jasspa_MicroEmacs_#{version}_macros.zip"
  sha256 "__sha__"
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
    puts "start Microemacs with: MEPATH=~/.config/jasspa:/home/linuxbrew/.linuxbrew/share/jasspa/macros:/home/linuxbrew/.linuxbrew/share/jasspa/spelling mec (or mew)"
    puts "on MacOS replace /home/linuxbrew/.linuxbrew with /opt/homebrew for M1 Macs or /usr/local for Intel Macs"
  end

end
