set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DLB_Use_LightweightTypeID")

file(GLOB_RECURSE SOURCE_FILES
	*.cpp)
file(GLOB_RECURSE HEADER_FILES
    *.h
    *.hpp)

add_library(LBase STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(LBase PUBLIC ${SDK_PATH})
target_include_directories(LBase PUBLIC ${CMAKE_SOURCE_DIR})
