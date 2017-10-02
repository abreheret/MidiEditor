mkdir x64
cd x64
mkdir vc14
cd vc14
cmake -DNO_CONSOLE_MODE=ON -DQT5_DIR=D:/dev/Qt/5.9.1/msvc2015_64/lib/cmake -DINSTALL_WITH_CPACK=ON -G "Visual Studio 14 Win64" ../../..
cmake --build . --config Release
cpack -DCPACK_GENERATOR=WIX -C Release

cd ../..

mkdir x86
cd x86
mkdir vc14
cd vc14

cmake -DQT5_DIR=D:/dev/Qt/5.9.1/msvc2015/lib/cmake -DINSTALL_WITH_CPACK=ON -G "Visual Studio 14" ../../..
cmake --build . --config Release
cpack -DCPACK_GENERATOR=WIX -C Release

PAUSE