file(GLOB_RECURSE SOURCE_FILES
    *.cpp)
file(GLOB_RECURSE HEADER_FILES
    *.h
    *.hpp)

add_executable(LSchemeTest ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(LSchemeTest PUBLIC ${SDK_PATH})
target_include_directories(LSchemeTest PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(LSchemeTest
    LBase
    LFramework
    LScheme)
