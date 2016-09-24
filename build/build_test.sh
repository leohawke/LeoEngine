g++ -DINITGUID -DUNICODE -D_UNICODE -municode -O2 -s \
 -I../ -include ../UniversalDXSDK/mingw/sal2.h \
 ../Engine/*.cpp ../Engine/Render/*.cpp ../Engine/Render/D3D12/*.cpp ../LTest/wWinMain.cpp \
  -L../LBase/build/ -lLBase -oLTest \
  -Wl,-subsystem,windows