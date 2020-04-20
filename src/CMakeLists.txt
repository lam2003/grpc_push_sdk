project(service-mesh-cpp DESCRIPTION "ServiceMesh C++ Client" LANGUAGES CXX)

include(windows_support)

set(_src_root_path ${CMAKE_CURRENT_LIST_DIR})
file(
    GLOB_RECURSE _source_list
    LIST_DIRECTORIES false
    "${_src_root_path}/*.c*"
    "${_src_root_path}/*.h*"
)

set(_include_root_path ${PUBLIC_INCLUDE_DIR})

file(
    GLOB_RECURSE _public_headers_list
    LIST_DIRECTORIES false
    "${_include_root_path}/*.h*"
)

add_library(service-mesh-cpp STATIC ${_source_list} ${_public_headers_list})

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(service-mesh-cpp PRIVATE /MP)
elseif(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(service-mesh-cpp PRIVATE -Wno-attributes)
    # disable warnings
    target_compile_options(service-mesh-cpp PRIVATE -Wno-conversion -Wno-pedantic -Wno-cast-qual -Wno-sign-compare)
endif()

target_compile_features(service-mesh-cpp PUBLIC cxx_std_11)

target_link_libraries(service-mesh-cpp PUBLIC libgrpc++.a)
target_link_libraries(service-mesh-cpp PUBLIC libssl crypto.a)
target_link_libraries(service-mesh-cpp PUBLIC libprotobuf.a)
target_link_libraries(service-mesh-cpp PUBLIC libspdlog.a)

target_include_directories(service-mesh-cpp
    PUBLIC ${THIRD_PARTY_DIR}
    PUBLIC ${PUBLIC_INCLUDE_DIR}
    PRIVATE .
    PRIVATE core
    PRIVATE ${THIRD_PARTY_DIR}/spdlog/include
    PRIVATE ${THIRD_PARTY_DIR}/grpc/include
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
