cmake_minimum_required(VERSION 3.15)

project(service-mesh-cpp)

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0091 NEW)

if(NOT WIN32)
    add_compile_options(-fvisibility=hidden -fPIC)
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

if(BUILD_MODE MATCHES "Debug")
    message("##################BUILD DEBUG##################")
    set(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")
elseif()
    message("##################BUILD Release##################")
endif()

set(CMAKE_FOLDER 3rdparty)
add_subdirectory(${THIRD_PARTY_DIR}/grpc)
add_subdirectory(${THIRD_PARTY_DIR}/spdlog)
set(CMAKE_FOLDER)

add_subdirectory(src)

