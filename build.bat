@echo off

REM TODO(Jorge): Make sure we compile with all libraries in release mode

pushd build

set SDLINCLUDE="..\external\SDL2-devel-2.0.10-VC\SDL2-2.0.10\include"
set SDLLIB64="..\external\SDL2-devel-2.0.10-VC\SDL2-2.0.10\lib\x64"
set GLADINCLUDE="..\external\glad\include"
set GLM="..\external\glm-0.9.9.6\glm-0.9.9.6"
set FREETYPEINCLUDE="..\external\Freetype\include"
set FREETYPELIB="..\external\Freetype\win64"
set SDLMIXER_INCLUDE="..\external\SDL2_mixer-2.0.4\include"
set SDLMIXER_LIB="..\external\SDL2_mixer-2.0.4\lib\x64"

set IncludeDirectories=-I%SDLINCLUDE% -I%GLADINCLUDE% -I%GLM% -I%FREETYPEINCLUDE% -I%SDLMIXER_INCLUDE%
set LibDirectories=-LIBPATH:%SDLLIB64% -LIBPATH:%FREETYPELIB% -LIBPATH:%SDLMIXER_LIB%

REM set CompilerFlags= -DDEBUG -nologo -W4 -WX -Ot -FS %IncludeDirectories% -Zi -EHsc -MD /D "_WINDOWS"
set CompilerFlags= -nologo -W4 -Ot -FS %IncludeDirectories% -Zi -EHsc -MD /D "_WINDOWS"
set LinkerFlags=-nologo -DEBUG %LibDirectories%

REM cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:CONSOLE SDL2.lib SDL2main.lib SDL2_mixer.lib freetype.lib
cl ..\main.cpp %CompilerFlags% /link %LinkerFlags% -SUBSYSTEM:WINDOWS SDL2.lib SDL2main.lib SDL2_mixer.lib freetype.lib

popd
