# Microsoft Developer Studio Project File - Name="memsdev6" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=memsdev6 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "memsdev6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "memsdev6.mak" CFG="memsdev6 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "memsdev6 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "memsdev6 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "memsdev6 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_WIN32" /D "_URLSUPP" /D NDEBUG=1 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /out:"Release/me32.exe"

!ELSEIF  "$(CFG)" == "memsdev6 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_WIN32" /D "_URLSUPP" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/me32.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "memsdev6 - Win32 Release"
# Name "memsdev6 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\abbrev.c
# End Source File
# Begin Source File

SOURCE=..\basic.c
# End Source File
# Begin Source File

SOURCE=..\bind.c
# End Source File
# Begin Source File

SOURCE=..\buffer.c
# End Source File
# Begin Source File

SOURCE=..\crypt.c
# End Source File
# Begin Source File

SOURCE=..\dirlist.c
# End Source File
# Begin Source File

SOURCE=..\display.c
# End Source File
# Begin Source File

SOURCE=..\eval.c
# End Source File
# Begin Source File

SOURCE=..\exec.c
# End Source File
# Begin Source File

SOURCE=..\file.c
# End Source File
# Begin Source File

SOURCE=..\fileio.c
# End Source File
# Begin Source File

SOURCE=..\hilight.c
# End Source File
# Begin Source File

SOURCE=..\history.c
# End Source File
# Begin Source File

SOURCE=..\input.c
# End Source File
# Begin Source File

SOURCE=..\isearch.c
# End Source File
# Begin Source File

SOURCE=..\key.c
# End Source File
# Begin Source File

SOURCE=..\line.c
# End Source File
# Begin Source File

SOURCE=..\macro.c
# End Source File
# Begin Source File

SOURCE=..\main.c
# End Source File
# Begin Source File

SOURCE=..\narrow.c
# End Source File
# Begin Source File

SOURCE=..\next.c
# End Source File
# Begin Source File

SOURCE=..\osd.c
# End Source File
# Begin Source File

SOURCE=..\print.c
# End Source File
# Begin Source File

SOURCE=..\random.c
# End Source File
# Begin Source File

SOURCE=..\regex.c
# End Source File
# Begin Source File

SOURCE=..\region.c
# End Source File
# Begin Source File

SOURCE=..\registry.c
# End Source File
# Begin Source File

SOURCE=..\search.c
# End Source File
# Begin Source File

SOURCE=..\spawn.c
# End Source File
# Begin Source File

SOURCE=..\spell.c
# End Source File
# Begin Source File

SOURCE=..\tag.c
# End Source File
# Begin Source File

SOURCE=..\termio.c
# End Source File
# Begin Source File

SOURCE=..\time.c
# End Source File
# Begin Source File

SOURCE=..\undo.c
# End Source File
# Begin Source File

SOURCE=..\window.c
# End Source File
# Begin Source File

SOURCE=..\winprint.c
# End Source File
# Begin Source File

SOURCE=..\winterm.c
# End Source File
# Begin Source File

SOURCE=..\word.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\ebind.h
# End Source File
# Begin Source File

SOURCE=..\edef.h
# End Source File
# Begin Source File

SOURCE=..\eextrn.h
# End Source File
# Begin Source File

SOURCE=..\efunc.h
# End Source File
# Begin Source File

SOURCE=..\emain.h
# End Source File
# Begin Source File

SOURCE=..\emode.h
# End Source File
# Begin Source File

SOURCE=..\eprint.h
# End Source File
# Begin Source File

SOURCE=..\esearch.h
# End Source File
# Begin Source File

SOURCE=..\eskeys.h
# End Source File
# Begin Source File

SOURCE=..\estruct.h
# End Source File
# Begin Source File

SOURCE=..\eterm.h
# End Source File
# Begin Source File

SOURCE=..\evar.h
# End Source File
# Begin Source File

SOURCE=..\evers.h
# End Source File
# Begin Source File

SOURCE=..\winterm.h
# End Source File
# Begin Source File

SOURCE=..\wintermr.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\me.rc
# End Source File
# Begin Source File

SOURCE=..\me32.bmp
# End Source File
# End Group
# End Target
# End Project
