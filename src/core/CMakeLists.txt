#
# zhangbin 20191210
#
cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0074 NEW)

project(ServiceMeshSdk DESCRIPTION "ServiceMesh Push CPPSdk" LANGUAGES CXX)

include(windows_support)

##CMAKE_CURRENT_LIST_DIR 是core目录
set(_src_root_path ${CMAKE_CURRENT_LIST_DIR}/..)
set(_dw_depends_path ${_src_root_path}/build/depends)

#for gslb GslbClientSdk.lib link
LINK_DIRECTORIES(${_dw_depends_path}/dwinternal/EduGrpcResolverSdk2015/lib)
message(STATUS "====>_dw_depends_path : ${_dw_depends_path}")

set(_src_core_path ${_src_root_path}/core)
set(_src_comm_path ${_src_root_path}/common)
set(_src_queue_path ${THIRD_PARTY_DIR}/MoodyCamel)
set(_src_thirdparty_path ${_src_root_path}/thirdparty)
set(_src_proto_path ${_src_root_path}/proto)
message(STATUS "====>_src_core_path : ${_src_core_path}")
message(STATUS "====>_src_queue_path : ${_src_queue_path}")
message(STATUS "====>_src_thirdparty_path : ${_src_thirdparty_path}")
file(
    GLOB_RECURSE _source_list
    LIST_DIRECTORIES false
    "${_src_core_path}/*.c*"
    "${_src_core_path}/*.h*"
    "${_src_comm_path}/*.c*"
    "${_src_comm_path}/*.h*"
    "${_src_queue_path}/*.c*"
    "${_src_queue_path}/*.h*"
    "${_src_proto_path}/*.c*"
    "${_src_proto_path}/*.h*"
)

##这个就是当前路径
message(STATUS "====>PUBLIC_INCLUDE_DIR : ${PUBLIC_INCLUDE_DIR}")
set(_include_root_path ${_src_root_path})
set(_include_core_path ${_include_root_path}/core)
set(_include_comm_path ${_include_root_path}/common)
set(_include_queue_path ${_src_queue_path})
set(_include_spd_path ${_src_thirdparty_path}/spdlog/include)
set(_include_grpc_path ${_src_root_path}/grpc/grpc)
#for gslb
set(_include_gslb_path ${_dw_depends_path}/dwinternal/EduGrpcResolverSdk2015/include)
MESSAGE( STATUS "_include_gslb_path = ${_include_gslb_path}")
include_directories(${_include_gslb_path})
#for common
include_directories(${_src_comm_path})

file(
    GLOB_RECURSE _public_headers_list
    LIST_DIRECTORIES false
    "${_include_core_path}/*.h*"
    "${_include_comm_path}/*.h*"
    "${_include_queue_path}/*.h*"
)
##打印出来看看
MESSAGE( STATUS "_source_list = ${_source_list}.")
MESSAGE( STATUS "_public_headers_list = ${_public_headers_list}.")


###build_windows.py 传递 SMPSDK_SHARED_LIBRARY 共享库
if(SMPSDK_SHARED_LIBRARY)
    MESSAGE( STATUS "===>enable SMPSDK_SHARED_LIBRARY")
    set(CMAKE_MACOSX_RPATH ON)

    add_library(ServiceMeshSdk SHARED ${_source_list} ${_public_headers_list})

    target_compile_definitions(ServiceMeshSdk PRIVATE SMPSDK_SHARED_LIBRARY)
    ##定义导出符号表
    target_compile_definitions(ServiceMeshSdk PRIVATE SMPSDK_SHARED_LIBRARY_EXPORTS)

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # disable C4251 warnings
        target_compile_options(ServiceMeshSdk PRIVATE /wd4251)
    endif()

else()
    # static lib
    add_library(ServiceMeshSdk STATIC ${_source_list} ${_public_headers_list})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(ServiceMeshSdk PRIVATE /MP)
elseif(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(ServiceMeshSdk PRIVATE -Wno-attributes)
endif()

###c++11
add_compile_options(-std=c++11)

##gen pdb file
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")


#CMAKE_MAKE_PROGRAM
MESSAGE( STATUS "==2=>CMAKE_MAKE_PROGRAM = ${CMAKE_MAKE_PROGRAM}")

##这个要有
if(BUILD_GRPC)
    target_link_libraries(ServiceMeshSdk PUBLIC grpc++)
    ##这个有什么影响么？
    target_compile_definitions(ServiceMeshSdk PRIVATE BUILD_GRPC)
endif()

target_link_libraries(ServiceMeshSdk PUBLIC libprotobuf)

if(ANDROID)
    target_link_libraries(ServiceMeshSdk PUBLIC log)
endif()



#gslb Edu100GslbSdk link
target_link_libraries(ServiceMeshSdk PUBLIC GslbClientSdk)

target_include_directories(ServiceMeshSdk
    PUBLIC ${_src_thirdparty_path}
    PUBLIC ${PUBLIC_INCLUDE_DIR}
    PUBLIC  ${_include_spd_path}
    PUBLIC  ${_src_comm_path}
    PUBLIC  ${_include_root_path}
    PUBLIC  ${_include_grpc_path}
    PUBLIC  ${_include_queue_path}
    PUBLIC  ${_include_gslb_path}
    PRIVATE api
    PRIVATE .
    PRIVATE ${THIRD_PARTY_DIR}/grpc/third_party/protobuf/src
)

foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("src\\${_group_path}" FILES "${_source}")
endforeach()

foreach(_header IN ITEMS ${_public_headers_list})
    get_filename_component(_header_path "${_header}" PATH)
    file(RELATIVE_PATH _header_path_rel "${_include_root_path}" "${_header_path}")
    string(REPLACE "/" "\\" _group_path "${_header_path_rel}")
    source_group("include\\${_group_path}" FILES "${_header}")
endforeach()
