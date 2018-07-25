#!/bin/bash

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
git_repo "https://github.com/zeromq/zmqpp.git" "zmqpp" "f8ff127683dc555aa004c0e6e2b18d2354a375be"

# Build LuaJIT
cd LuaJIT
make
cd ..

# Build libzmq
unset GREP_COLOR
unset GREP_OPTIONS
cd libzmq
[ ! -f configure ] && ./autogen.sh && ./configure
make
cd ..

# Build zmqpp
cd zmqpp
sed -i '' 's/CUSTOM_INCLUDE_PATH =/CUSTOM_INCLUDE_PATH = ..\/libzmq\/include' Makefile
sed -i '' 's/LIBRARY_LIBS =/LIBRARY_LIBS = -L..\/libzmq\/src\/.libs/' Makefile
make
cd ..
