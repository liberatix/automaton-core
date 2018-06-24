# run in Developer Command Prompt
xcopy /e /i /v /h /k gflags-2.2.1 ..\third_party_lib\temp\gflags
cd ..\third_party_lib\temp\gflags\

mkdir builds
cd builds
cmake ..
msbuild gflags.sln /p:Configuration=Release 

xcopy lib\Release ..\..\..
cd ..\..\..\..\third_party