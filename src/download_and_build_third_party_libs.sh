#!/bin/bash

darwin=false;
case "`uname`" in
  Darwin*) darwin=true ;;
esac

if $darwin; then
  sedi="sed -i ''"
else
  sedi="sed -i "
fi

print_separator() {
 str=$1
 num=$2
 v=$(printf "%-${num}s" "$str")
 echo "${v// /$str}"
}

LOCAL_3P="local_third_party"

mkdir -p $LOCAL_3P
cd $LOCAL_3P

function git_repo() {
  repo=$1
  dir=$2
  commit=$3

  print_separator "=" 80
  echo  Updating $dir from repo $repo
  print_separator "=" 80

  if [ ! -d $dir ]
  then
    git clone $repo $dir
  fi

  cd $dir

  if [ ! -z "$commit" ]
  then
    git reset --hard $commit
  else
    git pull
  fi

  cd ..
}

# Download all libraries
git_repo "https://github.com/LuaJIT/LuaJIT.git" "LuaJIT" "0bf80b07b0672ce874feedcc777afe1b791ccb5a"
git_repo "https://github.com/zeromq/libzmq.git" "libzmq" "d062edd8c142384792955796329baf1e5a3377cd"
git_repo "https://github.com/zeromq/cppzmq.git" "cppzmq" "d9f0f016c07046742738c65e1eb84722ae32d7d4"
git_repo "https://github.com/zeromq/zmqpp.git" "zmqpp" "f8ff127683dc555aa004c0e6e2b18d2354a375be"
git_repo "https://github.com/ThePhD/sol2.git" "sol2" "254466eb4b3ae630c731a557987f3adb1a8f86b0"
git_repo "https://github.com/AmokHuginnsson/replxx.git" "replxx" "3cb884e3fb4b1a28efeb716fac75f77eecc7ea3d"

# Build LuaJIT
print_separator "=" 80
echo "  BUILDING LuaJIT"
print_separator "=" 80

cd LuaJIT
make
cd ..

# Build libzmq
print_separator "=" 80
echo "   BUILDING libzmq"
print_separator "=" 80

unset GREP_COLOR
unset GREP_OPTIONS
cd libzmq
[ ! -f configure ] && ./autogen.sh && ./configure
make
cd ..

# Build zmqpp
print_separator "=" 80
echo "  BUILDING zmqpp"
print_separator "=" 80

cd zmqpp
$sedi 's/CUSTOM_INCLUDE_PATH =/CUSTOM_INCLUDE_PATH = -I..\/libzmq\/include/' Makefile
$sedi 's/LIBRARY_LIBS =/LIBRARY_LIBS = -L..\/libzmq\/src\/.libs/' Makefile
make
cd ..

# Build replxx
print_separator "=" 80
echo "  BUILDING replxx"
print_separator "=" 80

cd replxx
mkdir -p build && cd build
[ ! -f CMakeCache.txt ] && cmake -DCMAKE_BUILD_TYPE=Release ..
make replxx
cd ../..
