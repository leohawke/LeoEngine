file(GLOB_RECURSE SOURCE_FILES
	*.cpp)
file(GLOB_RECURSE HEADER_FILES
    *.h
    *.hpp)

add_executable(EngineTest ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(EngineTest PUBLIC ${SDK_PATH})
target_include_directories(EngineTest PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(EngineTest
    Engine
    LBase
    LFramework
    LScheme)
