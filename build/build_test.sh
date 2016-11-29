g++ -DINITGUID -DUNICODE -D_UNICODE -municode -O2 -s -std=c++17\
 -I../ -include ../UniversalDXSDK/mingw/sal2.h \
 ../Engine/*.cpp ../Engine/Render/*.cpp ../Engine/Render/D3D12/*.cpp ../LTest/EngineTest/wWinMain.cpp \
  -L../LBase/build/ -lLBase -oEngineTest \
  -Wl,-subsystem,windows

g++ -O2 -s -std=c++17\
 -I../  \
  ../LTest/LSchemeTest/*.cpp \
  -L../LBase/build/ -L./ -lLBase -lLScheme -oEngineTest \
  -Wl,-subsystem,console