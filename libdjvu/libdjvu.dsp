# Microsoft Developer Studio Project File - Name="libdjvu" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libdjvu - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libdjvu.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libdjvu.mak" CFG="libdjvu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libdjvu - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libdjvu - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libdjvu - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libdjvu - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libdjvu - Win32 Release"
# Name "libdjvu - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=Arrays.cpp
# End Source File
# Begin Source File

SOURCE=BSByteStream.cpp
# End Source File
# Begin Source File

SOURCE=BSEncodeByteStream.cpp
# End Source File
# Begin Source File

SOURCE=ByteStream.cpp
# End Source File
# Begin Source File

SOURCE=..\tools\jb2cmp\classify.cpp
# End Source File
# Begin Source File

SOURCE=..\tools\jb2cmp\cuts.cpp
# End Source File
# Begin Source File

SOURCE=DataPool.cpp
# End Source File
# Begin Source File

SOURCE=ddjvuapi.cpp
# End Source File
# Begin Source File

SOURCE=debug.cpp
# End Source File
# Begin Source File

SOURCE=DjVmDir.cpp
# End Source File
# Begin Source File

SOURCE=DjVmDir0.cpp
# End Source File
# Begin Source File

SOURCE=DjVmDoc.cpp
# End Source File
# Begin Source File

SOURCE=DjVmNav.cpp
# End Source File
# Begin Source File

SOURCE=DjVuAnno.cpp
# End Source File
# Begin Source File

SOURCE=DjVuDocEditor.cpp
# End Source File
# Begin Source File

SOURCE=DjVuDocument.cpp
# End Source File
# Begin Source File

SOURCE=DjVuDumpHelper.cpp
# End Source File
# Begin Source File

SOURCE=DjVuFile.cpp
# End Source File
# Begin Source File

SOURCE=DjVuFileCache.cpp
# End Source File
# Begin Source File

SOURCE=DjVuImage.cpp
# End Source File
# Begin Source File

SOURCE=DjVuInfo.cpp
# End Source File
# Begin Source File

SOURCE=DjVuMessage.cpp
# End Source File
# Begin Source File

SOURCE=DjVuMessageLite.cpp
# End Source File
# Begin Source File

SOURCE=DjVuNavDir.cpp
# End Source File
# Begin Source File

SOURCE=DjVuPalette.cpp
# End Source File
# Begin Source File

SOURCE=DjVuPort.cpp
# End Source File
# Begin Source File

SOURCE=DjVuText.cpp
# End Source File
# Begin Source File

SOURCE=DjVuToPS.cpp
# End Source File
# Begin Source File

SOURCE=..\tools\jb2cmp\frames.cpp
# End Source File
# Begin Source File

SOURCE=GBitmap.cpp
# End Source File
# Begin Source File

SOURCE=GContainer.cpp
# End Source File
# Begin Source File

SOURCE=GException.cpp
# End Source File
# Begin Source File

SOURCE=GMapAreas.cpp
# End Source File
# Begin Source File

SOURCE=GOS.cpp
# End Source File
# Begin Source File

SOURCE=GPixmap.cpp
# End Source File
# Begin Source File

SOURCE=GRect.cpp
# End Source File
# Begin Source File

SOURCE=GScaler.cpp
# End Source File
# Begin Source File

SOURCE=GSmartPointer.cpp
# End Source File
# Begin Source File

SOURCE=GString.cpp
# End Source File
# Begin Source File

SOURCE=GThreads.cpp
# End Source File
# Begin Source File

SOURCE=GUnicode.cpp
# End Source File
# Begin Source File

SOURCE=GURL.cpp
# End Source File
# Begin Source File

SOURCE=IFFByteStream.cpp
# End Source File
# Begin Source File

SOURCE=IW44EncodeCodec.cpp
# End Source File
# Begin Source File

SOURCE=IW44Image.cpp
# End Source File
# Begin Source File

SOURCE=JB2EncodeCodec.cpp
# End Source File
# Begin Source File

SOURCE=JB2Image.cpp
# End Source File
# Begin Source File

SOURCE=miniexp.cpp
# End Source File
# Begin Source File

SOURCE=MMRDecoder.cpp
# End Source File
# Begin Source File

SOURCE=MMX.cpp
# End Source File
# Begin Source File

SOURCE=..\tools\jb2cmp\patterns.cpp
# End Source File
# Begin Source File

SOURCE=UnicodeByteStream.cpp
# End Source File
# Begin Source File

SOURCE=XMLParser.cpp
# End Source File
# Begin Source File

SOURCE=XMLTags.cpp
# End Source File
# Begin Source File

SOURCE=ZPCodec.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=Arrays.h
# End Source File
# Begin Source File

SOURCE=BSByteStream.h
# End Source File
# Begin Source File

SOURCE=ByteStream.h
# End Source File
# Begin Source File

SOURCE=DataPool.h
# End Source File
# Begin Source File

SOURCE=ddjvuapi.h
# End Source File
# Begin Source File

SOURCE=debug.h
# End Source File
# Begin Source File

SOURCE=DjVmDir.h
# End Source File
# Begin Source File

SOURCE=DjVmDir0.h
# End Source File
# Begin Source File

SOURCE=DjVmDoc.h
# End Source File
# Begin Source File

SOURCE=DjVmNav.h
# End Source File
# Begin Source File

SOURCE=DjVuAnno.h
# End Source File
# Begin Source File

SOURCE=DjVuDocEditor.h
# End Source File
# Begin Source File

SOURCE=DjVuDocument.h
# End Source File
# Begin Source File

SOURCE=DjVuDumpHelper.h
# End Source File
# Begin Source File

SOURCE=DjVuErrorList.h
# End Source File
# Begin Source File

SOURCE=DjVuFile.h
# End Source File
# Begin Source File

SOURCE=DjVuFileCache.h
# End Source File
# Begin Source File

SOURCE=DjVuGlobal.h
# End Source File
# Begin Source File

SOURCE=DjVuImage.h
# End Source File
# Begin Source File

SOURCE=DjVuInfo.h
# End Source File
# Begin Source File

SOURCE=DjVuMessage.h
# End Source File
# Begin Source File

SOURCE=DjVuMessageLite.h
# End Source File
# Begin Source File

SOURCE=DjVuNavDir.h
# End Source File
# Begin Source File

SOURCE=DjVuPalette.h
# End Source File
# Begin Source File

SOURCE=DjVuPort.h
# End Source File
# Begin Source File

SOURCE=DjVuText.h
# End Source File
# Begin Source File

SOURCE=DjVuToPS.h
# End Source File
# Begin Source File

SOURCE=GBitmap.h
# End Source File
# Begin Source File

SOURCE=GContainer.h
# End Source File
# Begin Source File

SOURCE=GException.h
# End Source File
# Begin Source File

SOURCE=GIFFManager.h
# End Source File
# Begin Source File

SOURCE=GMapAreas.h
# End Source File
# Begin Source File

SOURCE=GOS.h
# End Source File
# Begin Source File

SOURCE=GPixmap.h
# End Source File
# Begin Source File

SOURCE=GRect.h
# End Source File
# Begin Source File

SOURCE=GScaler.h
# End Source File
# Begin Source File

SOURCE=GSmartPointer.h
# End Source File
# Begin Source File

SOURCE=GString.h
# End Source File
# Begin Source File

SOURCE=GThreads.h
# End Source File
# Begin Source File

SOURCE=GURL.h
# End Source File
# Begin Source File

SOURCE=IFFByteStream.h
# End Source File
# Begin Source File

SOURCE=IW44Image.h
# End Source File
# Begin Source File

SOURCE=JB2Image.h
# End Source File
# Begin Source File

SOURCE=JPEGDecoder.h
# End Source File
# Begin Source File

SOURCE=miniexp.h
# End Source File
# Begin Source File

SOURCE=MMRDecoder.h
# End Source File
# Begin Source File

SOURCE=MMX.h
# End Source File
# Begin Source File

SOURCE=Template.h
# End Source File
# Begin Source File

SOURCE=UnicodeByteStream.h
# End Source File
# Begin Source File

SOURCE=XMLParser.h
# End Source File
# Begin Source File

SOURCE=XMLTags.h
# End Source File
# Begin Source File

SOURCE=ZPCodec.h
# End Source File
# End Group
# End Target
# End Project
