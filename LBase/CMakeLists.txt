set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DLB_Use_LightweightTypeID")

file(GLOB_RECURSE source_list
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	*.h
	*.hpp
    *.cpp
	)

add_library(LBase STATIC ${source_list})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX  "LBase" FILES ${source_list})

target_include_directories(LBase PUBLIC ${SDK_PATH})
target_include_directories(LBase PUBLIC ${CMAKE_SOURCE_DIR})
