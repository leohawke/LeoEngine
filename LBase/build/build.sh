rm -rf release
mkdir release
cd release
g++ -s -c -O2 -std=c++17 ../../*.cpp -I../../../
ar cqs ../libLBase.a *.o
cd ..
