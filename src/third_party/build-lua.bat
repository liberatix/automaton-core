# run in Developer Command Prompt
xcopy /e /i /v /h /k lua-5.3.4 ..\third_party_lib\temp\lua
cd ..\third_party_lib\temp\lua\src

cl /MD /O2 /c /DLUA_BUILD_AS_DLL *.c
ren lua.obj lua.o
ren luac.obj luac.o
link /DLL /IMPLIB:lua5.3.0.lib /OUT:lua5.3.0.dll *.obj 
link /OUT:lua.exe lua.o lua5.3.0.lib 
lib /OUT:lua5.3.0-static.lib *.obj
link /OUT:luac.exe luac.o lua5.3.0-static.lib 

xcopy lua5.3.0-static.lib ..\..\..
cd ..\..\..\..\third_party