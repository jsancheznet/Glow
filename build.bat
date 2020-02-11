@echo off

REM TODO(Jorge): Make sure we compile with all libraries in release mode
REM TODO(Jorge): Use relative paths

pushd build

set SDLINCLUDE="C:\Users\Jsanchez\Dropbox\Projects\Untitled\external\SDL2-devel-2.0.10-VC\SDL2-2.0.10\include"
set SDLLIB64="C:\Users\Jsanchez\Dropbox\Projects\Untitled\external\SDL2-devel-2.0.10-VC\SDL2-2.0.10\lib\x64"
set GLADINCLUDE="C:\Users\Jsanchez\Dropbox\Projects\Untitled\external\glad\include"
set GLM="C:\Users\Jsanchez\Dropbox\Projects\Untitled\external\glm-0.9.9.6\glm-0.9.9.6"
set FREETYPEINCLUDE="C:\Users\Jsanchez\Dropbox\Projects\Untitled\external\Freetype\include"
set FREETYPELIB="C:\Users\Jsanchez\Dropbox\Projects\Untitled\external\Freetype\win64"
set SOLOUD_INCLUDE="C:\Users\Jsanchez\Dropbox\Projects\Untitled\external\soloud\include"

set IncludeDirectories=-I%SDLINCLUDE% -I%GLADINCLUDE% -I%GLM% -I%FREETYPEINCLUDE% -I%SOLOUD_INCLUDE%
set LibDirectories=-LIBPATH:%SDLLIB64% -LIBPATH:%FREETYPELIB%

set CompilerFlags= -DDEBUG -nologo -W4 -WX -Od %IncludeDirectories% -Zi -EHsc -MD /D "_WINDOWS"
set LinkerFlags=-nologo -DEBUG %LibDirectories%

cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:CONSOLE SDL2.lib SDL2main.lib freetype.lib

popd
