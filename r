#!/bin/bash

mkdir -p build && cd build && cmake ..
if [[ $? -ne 0 ]]; then
	exit $?
fi

bin_path="/"

if [[ "$OSTYPE" == "msys"* ]]; then
	msbuild.exe *.sln
	bin_path="/Debug"
else
	make
fi

cd ..

./run_tests 

RED='\033[0;35m'
WHITE='\033[0;39m'

echo OSTYPE=$OSTYPE

echo 
echo -e $RED ---- Running Kai Console ----
echo -e $WHITE

./Bin${bin_path}/Console

