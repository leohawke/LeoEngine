g++ -DINITGUID -DUNICODE -D_UNICODE -municode -O2 -s -std=c++17\
 -I../ -I../SDKS -I../SDKS/mingw -include ../SDKS/UniversalDXSDK/mingw/sal2.h \
 ../Engine/*.cpp ../Engine/Render/*.cpp ../Engine/Render/D3D12/*.cpp ../LTest/EngineTest/wWinMain.cpp \
  -L../LBase/build/ -lLBase -oEngineTest \
  -Wl,-subsystem,windows

g++ -O2 -s -std=c++17\
 -I../  \
  ../LTest/LSchemeTest/*.cpp \
  -L../LBase/build/ -L./ -lLScheme -lLBase -oLSchemeTest \
  -Wl,-subsystem,console