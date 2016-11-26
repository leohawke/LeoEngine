mkdir scheme
cd scheme
g++ -O2 -s -c  -std=c++14\
 -I../../ \
 ../../LScheme/*.cpp \
  -L../../LBase/build/ -lLBase
  ar cqs ../libLScheme.a *.o
