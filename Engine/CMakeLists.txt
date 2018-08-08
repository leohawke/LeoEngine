if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /await")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(FATAL_ERROR "Gcc does not support Coroutine TS now, which this project requires, see https://gcc.gnu.org/projects/cxx-status.html#tses for further infomation.")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts -stdlib=libc++")
else()
    message(FATAL_ERROR "Unsupported toolchain \"${CMAKE_CXX_COMPILER_ID}\"")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DINITGUID -D_CRT_SECURE_NO_WARNINGS")
set(CMAKE_CXX_FLAGS_DEBUG_TOOL "${CMAKE_CXX_FLAGS_DEBUG_TOOL} -DINITGUID -DENGINE_TOOL -D_CRT_SECURE_NO_WARNINGS")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DLB_Use_LightweightTypeID")

file(GLOB_RECURSE source_list
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	*.h
	*.hpp
    *.cpp
	)

add_library(Engine STATIC ${source_list})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${source_list})

target_include_directories(Engine PUBLIC ${SDK_PATH})
target_include_directories(Engine PUBLIC ${CMAKE_SOURCE_DIR})
