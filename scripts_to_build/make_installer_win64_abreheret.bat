mkdir x64
cd x64
cmake -DNO_CONSOLE_MODE=ON -DQT5_DIR_GUESS=D:/dev/Qt/5.9.1/msvc2013_64/lib/cmake -DINSTALL_WITH_CPACK=ON -G "Visual Studio 12 Win64" ../..
cmake --build . --config Release
cpack -DCPACK_GENERATOR=WIX -C Release
PAUSE