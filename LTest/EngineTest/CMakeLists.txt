file(GLOB_RECURSE source_list
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	*.h
    *.cpp
	)

link_directories(${SDK_PATH})

add_executable(EngineTest WIN32 ${source_list})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${source_list})

target_include_directories(EngineTest PUBLIC ${SDK_PATH})
target_include_directories(EngineTest PUBLIC ${CMAKE_SOURCE_DIR})


target_link_libraries(EngineTest
    Engine
    LBase
    LFramework
    LScheme)
target_link_libraries(EngineTest Imm32)

