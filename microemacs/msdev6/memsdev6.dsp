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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_WIN32" /D "_URLSUPP" /D NDEBUG=1 /D "_ME_WINDOW" /YX /FD /c
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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_WIN32" /D "_SOCKET" /D "_ME_WINDOW" /YX /FD /GZ /c
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

SOURCE=..\src\abbrev.c
# End Source File
# Begin Source File

SOURCE=..\src\basic.c
# End Source File
# Begin Source File

SOURCE=..\src\bind.c
# End Source File
# Begin Source File

SOURCE=..\src\buffer.c
# End Source File
# Begin Source File

SOURCE=..\src\crypt.c
# End Source File
# Begin Source File

SOURCE=..\src\dirlist.c
# End Source File
# Begin Source File

SOURCE=..\src\display.c
# End Source File
# Begin Source File

SOURCE=..\src\eval.c
# End Source File
# Begin Source File

SOURCE=..\src\exec.c
# End Source File
# Begin Source File

SOURCE=..\src\file.c
# End Source File
# Begin Source File

SOURCE=..\src\fileio.c
# End Source File
# Begin Source File

SOURCE=..\src\frame.c
# End Source File
# Begin Source File

SOURCE=..\src\hilight.c
# End Source File
# Begin Source File

SOURCE=..\src\history.c
# End Source File
# Begin Source File

SOURCE=..\src\input.c
# End Source File
# Begin Source File

SOURCE=..\src\isearch.c
# End Source File
# Begin Source File

SOURCE=..\src\key.c
# End Source File
# Begin Source File

SOURCE=..\src\line.c
# End Source File
# Begin Source File

SOURCE=..\src\macro.c
# End Source File
# Begin Source File

SOURCE=..\src\main.c
# End Source File
# Begin Source File

SOURCE=..\src\narrow.c
# End Source File
# Begin Source File

SOURCE=..\src\next.c
# End Source File
# Begin Source File

SOURCE=..\src\osd.c
# End Source File
# Begin Source File

SOURCE=..\src\print.c
# End Source File
# Begin Source File

SOURCE=..\src\random.c
# End Source File
# Begin Source File

SOURCE=..\src\regex.c
# End Source File
# Begin Source File

SOURCE=..\src\region.c
# End Source File
# Begin Source File

SOURCE=..\src\registry.c
# End Source File
# Begin Source File

SOURCE=..\src\search.c
# End Source File
# Begin Source File

SOURCE=..\src\spawn.c
# End Source File
# Begin Source File

SOURCE=..\src\spell.c
# End Source File
# Begin Source File

SOURCE=..\src\tag.c
# End Source File
# Begin Source File

SOURCE=..\src\termio.c
# End Source File
# Begin Source File

SOURCE=..\src\time.c
# End Source File
# Begin Source File

SOURCE=..\src\undo.c
# End Source File
# Begin Source File

SOURCE=..\src\window.c
# End Source File
# Begin Source File

SOURCE=..\src\winprint.c
# End Source File
# Begin Source File

SOURCE=..\src\winterm.c
# End Source File
# Begin Source File

SOURCE=..\src\word.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\ebind.h
# End Source File
# Begin Source File

SOURCE=..\src\edef.h
# End Source File
# Begin Source File

SOURCE=..\src\eextrn.h
# End Source File
# Begin Source File

SOURCE=..\src\efunc.h
# End Source File
# Begin Source File

SOURCE=..\src\emain.h
# End Source File
# Begin Source File

SOURCE=..\src\emode.h
# End Source File
# Begin Source File

SOURCE=..\src\eprint.h
# End Source File
# Begin Source File

SOURCE=..\src\esearch.h
# End Source File
# Begin Source File

SOURCE=..\src\eskeys.h
# End Source File
# Begin Source File

SOURCE=..\src\estruct.h
# End Source File
# Begin Source File

SOURCE=..\src\eterm.h
# End Source File
# Begin Source File

SOURCE=..\src\evar.h
# End Source File
# Begin Source File

SOURCE=..\src\evers.h
# End Source File
# Begin Source File

SOURCE=..\src\winterm.h
# End Source File
# Begin Source File

SOURCE=..\src\wintermr.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\src\me.rc
# End Source File
# Begin Source File

SOURCE=..\src\me32.bmp
# End Source File
# End Group
# End Target
# End Project
