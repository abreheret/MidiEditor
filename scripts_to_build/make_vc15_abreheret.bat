mkdir x64
cd x64
mkdir vc15
cd vc15
cmake -DNO_CONSOLE_MODE=ON -DQT5_DIR=D:/dev/Qt/5.9.1/msvc2017_64/lib/cmake -DINSTALL_WITH_CPACK=ON -G "Visual Studio 15 2017 Win64" ../../..
cmake --build . --config Release
cpack -DCPACK_GENERATOR=WIX -C Release

PAUSE