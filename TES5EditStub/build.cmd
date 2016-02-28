@echo off
setlocal
rem Setup our VC environment
set MAXVERS=14
set BUILDARCH=X64
:getopt
if "%~1x"=="x"           goto setup_msvc
if /i "%~1"=="x86"       (set "BUILDARCH=%~1") & (shift /1) & goto getopt
if /i "%~1"=="amd64"     (set "BUILDARCH=%~1") & (shift /1) & goto getopt
if /i "%~1"=="x64"       (set "BUILDARCH=%~1") & (shift /1) & goto getopt
if /i "%~1"=="ia64"      (set "BUILDARCH=%~1") & (shift /1) & goto getopt
if /i "%~1"=="x86_amd64" (set "BUILDARCH=%~1") & (shift /1) & goto getopt
if /i "%~1"=="x86_ia64"  (set "BUILDARCH=%~1") & (shift /1) & goto getopt
if not "%~1x"=="x"       (set "MAXVERS=%~1")   & (shift /1) & goto getopt

:setup_msvc
if defined INCLUDE goto error_env
call "%~dp0find-msvc.cmd" %MAXVERS% 8
if errorlevel 1 exit /B %ERRORLEVEL%
if not defined VCROOT exit /B 1
call "%VCROOT%\vcvarsall.bat" %BUILDARCH%>nul 2>nul
if errorlevel 1 exit /B %ERRORLEVEL%

:setup_flags
rem Normalize our build architecture
if "x%BUILDARCH:~-2%"=="x64" set BUILDARCH=X64
if not "%BUILDARCH%x"=="X64x" set BUILDARCH=X86
if "%BUILDARCH%"=="X86" set BUILDBITS=32
if "%BUILDARCH%"=="X64" set BUILDBITS=64

rem Used for checking ranges a bit later
set /A VCVERNUM=%VCVERS%

rem Source/Output filenames
set _BASENAME=TES5EditStub
set "_SRCBASE=%~dp0TES5EditStub"
set "_OUTFILE=%~dp0TES5Edit.exe"

rem RC Flags
set RCFLAGS=/nologo /D_NDEBUG /DNDEBUG /D_WIN32 /DWIN32 /D_CONSOLE /DCONSOLE /D_WIN64

rem Static CRT library arg. Change to /MD if you want to use the shared libraries.
set _CRTFLAG=/MT

rem Resolve the CRT library file based on the value of _CRTFLAG.
if /i "%_CRTFLAG:~1%"=="MD" set _CRTLIB=msvcrt.lib
if /i "%_CRTFLAG:~1%"=="MDd" set _CRTLIB=msvcrtd.lib
if /i "%_CRTFLAG:~1%"=="MT" set _CRTLIB=libcmt.lib
if /i "%_CRTFLAG:~1%"=="MTd" set _CRTLIB=libcmtd.lib

rem Build subsystem. Uncomment the second line if you want a console window.
set _SUBSYSTEM=WINDOWS
rem set _SUBSYSTEM=CONSOLE

rem Preprocessor definitions.
set _CDEFS=/D_UNICODE /DUNICODE /DNDEBUG=1 /D_NDEBUG=1 /D_CRT_SECURE_NO_DEPRECATE
if defined _SUBSYSTEM set "_CDEFS=%_CDEFS% /D_%_SUBSYSTEM% /D%_SUBSYSTEM%"

rem Shared and year-specific compiler options
set "CFLAGS=/nologo /W3 /WX- /fp:precise /Qfast_transcendentals /EHs-c- /Ox /GA /GL /GF /Gm- /GS- /Gy /GT -I."
set CFLAGS64=/favor:INTEL64
set CFLAGS2010= 
set CFLAGS2012=/Qpar
rem Extended instruction sets - uncomment if your CPU supports them.
REM set CFLAGS2010=/arch:SSE2
REM set CFLAGS2012=/arch:AVX /Qpar
set "CFLAGS2013=%CFLAGS2012% /Gw /Zc:inline /cgthreads4"
set "CFLAGS2015=%CFLAGS2013%"

rem Shared and year-specific linker options
set LDFLAGS=/nologo /INCREMENTAL:NO /LTCG /OPT:ICF=32 /OPT:REF /MACHINE:%BUILDARCH% /LARGEADDRESSAWARE
if defined _SUBSYSTEM set "LDFLAGS=%LDFLAGS% /SUBSYSTEM:%_SUBSYSTEM%"
set LDFLAGS2010=/MANIFEST
set LDFLAGS2012=/MANIFEST:EMBED
set "LDFLAGS2013=%LDFLAGS2012% /CGTHREADS:4"
set "LDFLAGS2015=%LDFLAGS2013%"

rem Merge our shared toolset options with the appropriate year-specific options
call set "CFLAGS=%%CFLAGS%% %%CFLAGS%VCYEAR%%%"
call set "LDFLAGS=%%LDFLAGS%% %%LDFLAGS%VCYEAR%%%"

rem If we're doing a 64-bit build, replace any arch:SSE2 args with arch:AVX and append our 64-bit options
if "%BUILDARCH%"=="X64" set "CFLAGS=%CFLAGS:SSE2=AVX% %CFLAGS64%"

rem Compile sources
echo cl.exe %_CDEFS% %_CRTFLAG% %CFLAGS% /c /Fo%_BASENAME%.obj %_BASENAME%.c
call cl.exe %_CDEFS% %_CRTFLAG% %CFLAGS% /c "/Fo%_SRCBASE%.obj" "%_SRCBASE%.c"
if errorlevel 1 exit /B %ERRORLEVEL%

rem Compile resources
call rc.exe %RCFLAGS% "/fo%_SRCBASE%.res" "%_SRCBASE%.rc"
if errorlevel 1 exit /B %ERRORLEVEL%

rem Link executable
call link.exe %LDFLAGS% "/OUT:%_OUTFILE%" /NODEFAULTLIB "%_SRCBASE%.obj" "%_SRCBASE%.res" kernel32.lib user32.lib %_CRTLIB%
if errorlevel 1 exit /B %ERRORLEVEL%

rem VC2012 and later can embed the manifest into the executable from the linker.
if %VCVERNUM% GEQ 11 goto cleanup

rem Embed manifest into the executable
call mt.exe /nologo -manifest %_OUTFILE%.manifest -outputresource:%_OUTFILE%
if errorlevel 1 exit /B %ERRORLEVEL%

:cleanup
rem Cleanup temporarey build filds
if exist "%~dp0*.obj" del /F /Q "%~dp0*.obj"
if exist "%~dp0*.res" del /F /Q "%~dp0*.res"
if exist "%~dp0*.manifest" del /F /Q "%~dp0*.manifest"
endlocal
goto :EOF

:error_env
echo ERROR: %~nx0 needs to be invoked from a clean environment, with no existing VC env setup.
exit /B 1
