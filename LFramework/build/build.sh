rm -rf release
mkdir release
cd release
g++ -s -c -O2 -std=c++17 ../../CHRLib/*.cpp ../../Core/*.cpp ../../Helper/*.cpp ../../LCLib/*.cpp ../../Service/*.cpp ../../Win32/LCLib/*.cpp -I../../../
ar cqs ../libLFramework.a *.o
cd ..
