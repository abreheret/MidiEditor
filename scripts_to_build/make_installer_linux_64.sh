#!/bin/bash
mkdir x64
cd x64
cmake -DINSTALL_WITH_CPACK=ON -G "Unix Makefiles" ../..
cmake -P version.cmake 
cmake --build . --config Release
cpack -C Release
