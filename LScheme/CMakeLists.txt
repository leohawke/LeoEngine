set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLSCHEME_EXPORTS")

file(GLOB_RECURSE SOURCE_FILES
    *.cpp)
file(GLOB_RECURSE HEADER_FILES
    *.h
    *.hpp)

add_library(LScheme SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(LScheme PUBLIC ${SDK_PATH})
target_include_directories(LScheme PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(LScheme LBase LFramework)
