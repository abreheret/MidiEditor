#!/bin/bash
sudo apt-get install libsfml-dev
sudo apt-get install libasound2-dev
mkdir x64
cd x64

cmake -G "Unix Makefiles" ../..
cmake -P version.cmake 
