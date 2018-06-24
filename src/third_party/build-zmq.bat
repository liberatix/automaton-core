# run in Developer Command Prompt
# note that it is required that zmq is build in folder named "libzmq"

xcopy /e /i /v /h /k zeromq-4.2.5 ..\third_party_lib\temp\libzmq
cd ..\third_party_lib\temp\libzmq\builds\msvc\vs2015\

msbuild libzmq.sln /p:Configuration=StaticRelease /property:Platform=x64 

cd ..\..\..\bin\x64\Release\v140\static

xcopy /e /i /v /h /k libzmq.lib ..\..\..\..\..\..\..
cd ..\..\..\..\..\..\..\..\third_party