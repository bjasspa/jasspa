> -!- document -!-
>
>  Copyright 2009 Jon Green.
>
>  Created By    : Jon Green
>  Created       : Wed Oct 21 12:09:12 2009
>  Last Modified : <100829.0100>
>

Using "tfs" with MicroEmacs
===========================

  The latest  builds of MicroEmacs  are  experimenting  with  inclusion of the
  macro tree in the  executable.  The command line utility "tfs"  provides the
  tools to build and add the archive to the executable.

  The latest  version  2009-11-09 is now ported to windows and the utility TFS
  builds with minGW only and not MS-DEV.  Linux/Apple/Unix  platforms  are all
  supported. No build has been performed for DOS.

  The  following  examples use "me.exe" as a clearer  example of which item is
  the executable.  Obviously in a UNIX type environment then the ".exe" suffix
  is not used and does not exist.

  To build a tfs  archive  then simply  unpack the "metree" and remove  things
  that you do not want in the archive:

  % tar zxvf jasspa-metree-20091017.tar.gz
  % rm -rf ./jasspa/pixmaps

  You can add new things such as a spelling  dictionary,  some of your private
  files etc. DO NOT add the  dynamic  files such as  <usr>.emf  or the session
  files as the archive is READ-ONLY and cannot write back any saved session or
  history information.

  Build the archive and add to an  executable  from a different  directory  to
  create a new executable with TFS archive attached:

  % tfs -a mypath/mecw.exe -o me.exe ./jasspa

  Append an already existing archive to the executable:

  % tfs -a mypath/mecw.exe -o me.exe jasspa-metree-20091117.tfs

  This  should now be a stand alone  executable.  If you append  again it will
  remove the old archive and add a new one.

  The  archive  may be updated in place  using  either a  directory  or a .tfs
  file. This will  remove the  existing  archive  and  replace it with the new
  archive. i.e.

  % tfs -a me.exe ./jasspa          # Add a new archive from directory.
  % tfs -a me.exe jasspa.tfs        # Add a pre-built archive.

  You can list the archive attached to the executable:

  % tfs -l me.exe

  You can test the archive attached to the executable:

  % tfs -t me.exe

  You can get some information on the archive attached to the executable:

  % tfs -i me.exe

  You can remove (or strip) the archive attached to the executable:

  % tfs -s me.exe

  You can extract the archive  attached to the  executable  which will extract
  the directory tree in the archive to a new directory called "oldarch".

  % tfs -x oldarch me.exe        # Extract to directory called "oldarch"
  % tfs -f oldarch.tfs me.exe    # Extract to a file called "oldarch.tfs"

Building "tfs"
==============

  You should be able to issue a simple "make"

  % make

  Also recognises

  % make clean
  % make spotless

End of instructions.
