mkdir x86
cd x86
cmake -DNO_CONSOLE_MODE=OFF -DQT5_DIR=D:/dev/Qt/5.7/msvc2013/lib/cmake -DINSTALL_WITH_CPACK=ON -G "Visual Studio 12" ../..
cmake --build . --config Release
cpack -DCPACK_GENERATOR=WIX -C Release
PAUSE