mkdir x64
cd x64
cmake -DINSTALL_WITH_CPACK=ON -G "Visual Studio 12 Win64" ../..
cmake --build . --config Release
cpack -DCPACK_GENERATOR=WIX -C Release
PAUSE