# note that it is required that zmq is build in folder named "libzmq"
xcopy /e /i /v /h /k zeromq-4.2.5 ..\third_party_lib\temp\libzmq
cd ..\third_party_lib\temp\libzmq\builds\msvc\vs2015\
msbuild libzmq.sln /p:Configuration=StaticRelease /property:Platform=x64 
cd ..\..\..\..\..
xcopy /e /i /v /h /k temp\libzmq\bin\x64\Release\v140\static
cd ..\third_party