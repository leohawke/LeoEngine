mkdir release
cd release
g++ -s -c -O2 -std=c++14 ../../Win32/*.cpp ../../CHRLib/*.cpp ../../*.cpp -I../../../
ar cqs ../libLBase.a *.o
cd ..
