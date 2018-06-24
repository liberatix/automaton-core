# run in Developer Command Prompt
xcopy /e /i /v /h /k glog-c72907c4a813fa724430f8692706cb639acdb756 ..\third_party_lib\temp\glog
cd ..\third_party_lib\temp\glog\

cmake .
msbuild glog.sln /p:Configuration=Release 

xcopy Release\glog.lib ..\..
cd ..\..\..\third_party