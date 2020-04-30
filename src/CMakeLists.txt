project(push_sdk DESCRIPTION "Push SDK" LANGUAGES CXX)

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

if(PS_BUILD_SHARED)
    add_library(push_sdk SHARED ${_source_list} ${_public_headers_list})
else()
    add_library(push_sdk STATIC ${_source_list} ${_public_headers_list})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(push_sdk PRIVATE /MP)
elseif(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(push_sdk PRIVATE -Wno-attributes)
    # disable warnings
    target_compile_options(push_sdk PRIVATE -Wno-conversion -Wno-pedantic -Wno-cast-qual -Wno-sign-compare)
endif()

target_compile_features(push_sdk PUBLIC cxx_std_11)

target_include_directories(push_sdk
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
