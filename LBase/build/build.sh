rm -rf release
mkdir release
cd release
g++ -s -c -O2 -std=c++17 ../../Win32/*.cpp ../../CHRLib/*.cpp ../../*.cpp -I../../../
ar cqs ../libLBase.a *.o
cd ..
