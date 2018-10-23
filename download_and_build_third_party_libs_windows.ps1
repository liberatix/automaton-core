cd D:\GitHub\automaton-core\ # Remove this line before commit 

# Get path to VS, remember to start powershell if using command prompt
#$key = "Registry::HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\SxS\VS7"
#$value = "15.0"
#$vs_path = (Get-ItemProperty -Path $key -Name $value).$value



$LOCAL_3P="local_third_party"
cd .\src\
New-Item -ItemType Directory -force -Path .\$LOCAL_3P
cd .\$LOCAL_3P
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11"


function Get-GitRepo($repo, $dir, $commit) {
  echo ("="*80)
  echo " Updating $dir from repo $repo "
  echo ("="*80)

  if(!(Test-Path -Path .\$dir)) {
    git clone $repo $dir
  }

  cd $dir

  if($commit) {
    git reset --hard $commit
  } else {
    git pull
  }

  cd ..
 
}

#function get_archive() {
#  url=$1
#  filename=$2
#  sha=$3
#
#  print_separator "=" 80
#  echo "  Downloading $filename"
#  print_separator "=" 80
#
#  [ ! -f $2 ] && wget $1
#
#  filesha=$(shasum -a 256 $filename | cut -d' ' -f1)
#  [ $filesha == $sha ] || ( echo "Error: Wrong hash [$filesha] Expected [$sha]" && exit 1 )
#
#  echo "Extracting $filename"
#  tar -xzf $filename
#}


#  ========  Download all libraries ========
Get-GitRepo "https://github.com/LuaJIT/LuaJIT.git" "LuaJIT" "0bf80b07b0672ce874feedcc777afe1b791ccb5a"
#Get-GitRepo "https://github.com/zeromq/libzmq.git" "libzmq" "d062edd8c142384792955796329baf1e5a3377cd"
#Get-GitRepo "https://github.com/zeromq/cppzmq.git" "cppzmq" "d9f0f016c07046742738c65e1eb84722ae32d7d4"
#Get-GitRepo "https://github.com/zeromq/zmqpp.git" "zmqpp" "f8ff127683dc555aa004c0e6e2b18d2354a375be"
#Get-GitRepo "https://github.com/ThePhD/sol2.git" "sol2" "254466eb4b3ae630c731a557987f3adb1a8f86b0"
#Get-GitRepo "https://github.com/AmokHuginnsson/replxx.git" "replxx" "3cb884e3fb4b1a28efeb716fac75f77eecc7ea3d"
Get-GitRepo "https://github.com/lua/lua.git" "lua" "e354c6355e7f48e087678ec49e340ca0696725b1"
#Get-GitRepo "https://github.com/muflihun/easyloggingpp.git" "easyloggingpp" "a5317986d74b6dd3956021cb7fbb0669cce398b2"
#Get-GitRepo "https://github.com/weidai11/cryptopp.git" "cryptopp" "c8d8caf70074655a2562ae1ea45cb30e28fee2b4"
#Get-GitRepo "https://github.com/orlp/ed25519.git" "ed25519" "7fa6712ef5d581a6981ec2b08ee623314cd1d1c4"
#Get-GitRepo "https://github.com/google/googletest.git" "googletest" "2fe3bd994b3189899d93f1d5a881e725e046fdc2"
## Get-GitRepo "https://github.com/nlohmann/json.git" "json" "359f98d14065bf4e53eeb274f5987fd08f16e5bf"
#Get-GitRepo "https://github.com/nelhage/rules_boost.git" "com_github_nelhage_rules_boost" "fe787183c14f2a5c6e5e1e75a7c57d2e799d3d19"
#Get-GitRepo "https://github.com/protocolbuffers/protobuf.git" "protobuf" "48cb18e5c419ddd23d9badcfe4e9df7bde1979b2"
#Get-GitRepo "https://github.com/svaarala/duktape.git" "duktape" "d7fdb67f18561a50e06bafd196c6b423af9ad6fe"



#  ====== Check if missing and download using wget ======

if(!(Test-Path -Path .\json-3.1.2) -and (New-Item -ItemType Directory -Path .\json-3.1.2)) {
  wget -URI https://github.com/nlohmann/json/releases/download/v3.2.0/json.hpp -OutFile json-3.1.2/json.hpp
}




#  ======== Build libs ========

#  ====== Create lua.hpp with extern includes ======
echo ("="*80)
echo "  Create lua.hpp"
echo ("="*80)

$lua_extern_includes = "extern `"C`" {
#include `"lua.h`"
#include `"lualib.h`"
#include `"lauxlib.h`"
}"
$lua_extern_includes | Out-File -FilePath .\lua\lua.hpp -Encoding utf8

#  ====== Building LuaJIT ======
echo ("="*80)
echo "  Create lua.hpp"
echo ("="*80)

cd LuaJIT\src
.\msvcbuild.bat
cd ..\..




cd D:\GitHub\automaton-core\ # Remove this line before commit 