#/bin/bash

if [ $# -eq 0 ]
then
  echo "No arguments supplied"
  exit 1
fi

find . | xargs perl -p -i -e s/\\\"$1\\\//\"automaton\\\/core\\\/$1\\\//g
find . | xargs perl -p -i -e s/\\\"\\\/\\\/$1\\\//\\\"automaton\\\/core\\\/$1\\\//g
