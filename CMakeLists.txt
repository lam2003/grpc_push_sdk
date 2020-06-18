cmake_minimum_required(VERSION 3.9.1)

project(push_sdk)

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0063 NEW)
# ios构建系统不支持
# cmake_policy(SET CMP0074 NEW)
# cmake_policy(SET CMP0077 NEW)
# cmake_policy(SET CMP0091 NEW)

# ios构建系统查找go
find_program(GO_EXECUTABLE go PATHS /usr/local/bin)

if(NOT WIN32)
    add_compile_options(-fvisibility=hidden -fPIC)
else()
    add_compile_options(/WX-)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(windows_support)

set(gRPC_BUILD_CSHARP_EXT OFF)
set(protobuf_BUILD_TESTS OFF)
set(BENCHMARK_ENABLE_LTO:BOOL OFF)
set(BENCHMARK_ENABLE_TESTING:BOOL OFF)
set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

set(RUN_HAVE_STD_REGEX 0)
set(RUN_HAVE_POSIX_REGEX 0)
set(RUN_HAVE_GNU_POSIX_REGEX 0)
set(RUN_HAVE_STEADY_CLOCK 0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PUBLIC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/3rdparty)


set(PUSH_SDK_VERSION v1.0.0)
find_package(Git)
execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=6 --dirty --always --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE  GIT_REPO_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(WRITE ${PROJECT_SOURCE_DIR}/src/repo_version.h
    "#ifndef EDU_PUSH_SDK_VERSION_H\n#define EDU_PUSH_SDK_VERSION_H\n#define REPO_VERSION \"${GIT_REPO_VERSION}\"\n#define PUSH_SDK_VERSION \"${PUSH_SDK_VERSION}\"\n#endif\n"
)


if(PS_BUILD_MODE MATCHES "Debug")
    message("##################BUILD DEBUG##################")
    if(NOT WIN32)
        set(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")     
        set(CMAKE_C_FLAGS "-g ${CMAKE_C_FLAGS}")  
    endif()
else()
    message("##################BUILD Release##################")
endif()

set(CMAKE_FOLDER 3rdparty)
add_subdirectory(${THIRD_PARTY_DIR}/grpc)
add_subdirectory(${THIRD_PARTY_DIR}/libevent)
add_subdirectory(${THIRD_PARTY_DIR}/jsoncpp)
set(CMAKE_FOLDER)

add_subdirectory(src)

