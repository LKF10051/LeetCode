rm CMakeCache.txt
rm cmake_install.cmake
rm Makefile
rm -rf CMakeFiles
rm -rf src
cmake ..
make clean
make
