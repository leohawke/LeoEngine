file(GLOB_RECURSE SOURCE_FILES
	*.cpp)
file(GLOB_RECURSE HEADER_FILES
    *.h
    *.hpp)

add_library(LFramework STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(LFramework PUBLIC ${SDK_PATH})
target_include_directories(LFramework PUBLIC ${CMAKE_SOURCE_DIR})
