# -!- ruby -!-
class MicroemacsSpelling##Lang## < Formula
  desc "Jasspa MicroEmacs Text Editor - Spelling Rules for ##LANGUAGE## (##LANG##)"
  homepage "https://github.com/bjasspa/jasspa"
  version "##VERSION##"
  SHRPTH="#{HOMEBREW_PREFIX}/share"
  url "https://github.com/bjasspa/jasspa/releases/download/me_##VERSION##/Jasspa_MicroEmacs_##VERSION##_spelling_##LANG##.zip"
  sha256 "##SHA256##"

  def install
    require 'fileutils'
    FileUtils.mkdir_p("#{share}/jasspa/spelling") unless Dir.exist?("#{share}/jasspa/spelling")
    FileUtils.mkdir_p("#{SHRPTH}/jasspa/spelling") unless Dir.exist?("#{SHRPTH}/jasspa/spelling")
    Dir.glob("#{buildpath}/*").each do |bfn|
      if File.file?(bfn)
        fnm = File.basename(bfn)
        sfn = "#{share}/jasspa/spelling/#{fnm}"
        puts "Found file: spelling/#{fnm}"
        cp "#{bfn}","#{share}/jasspa/spelling"
        ln_sf sfn,"#{SHRPTH}/jasspa/spelling/#{fnm}"
      end
    end
  end

end
