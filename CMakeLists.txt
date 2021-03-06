cmake_minimum_required(VERSION 3.4)

# bitprim-protocol
#==============================================================================
project(bitprim-protocol
  VERSION 3.0.0
  LANGUAGES CXX C)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Check for baseline language coverage in the compiler for the C++11 standard.
#------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Process options.
#==============================================================================
# Implement --with-tests and declare WITH_TESTS.
#------------------------------------------------------------------------------
option(WITH_TESTS "Compile with unit tests." ON)

# Inherit --enable-shared and define BOOST_TEST_DYN_LINK.
#------------------------------------------------------------------------------
option(ENABLE_SHARED "" OFF)

# Check dependencies.
#==============================================================================
# Require Boost of at least version 1.56.0 and output ${boost_CPPFLAGS/LDFLAGS}.
#------------------------------------------------------------------------------
if (NOT ENABLE_SHARED)
  set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(
  Boost 1.56.0 REQUIRED
  COMPONENTS unit_test_framework)


# Require protobuf of at least version 3.0.0 and output ${protobuf_CPPFLAGS/LIBS/PKG}.
#------------------------------------------------------------------------------
if (DEFINED Protobuf_ROOT_DIR)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${Protobuf_ROOT_DIR})
endif()

find_package(Protobuf 3 REQUIRED)



# message("Protobuf_FOUND:             ${Protobuf_FOUND}")
# message("Protobuf_VERSION:           ${Protobuf_VERSION}")
# message("Protobuf_INCLUDE_DIRS:      ${Protobuf_INCLUDE_DIRS}")
# message("Protobuf_LIBRARIES:         ${Protobuf_LIBRARIES}")
# message("Protobuf_PROTOC_LIBRARIES:  ${Protobuf_PROTOC_LIBRARIES}")
# message("Protobuf_LITE_LIBRARIES:    ${Protobuf_LITE_LIBRARIES}")
# message("Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")



# Require zmq of at least version 4.1.1 and output ${zmq_CPPFLAGS/LIBS/PKG}.
#------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(ZeroMQ 4.1.1 REQUIRED)

# Require bitprim-core of at least version 3.0.0 and output ${bitprim_core_CPPFLAGS/LIBS/PKG}.
#------------------------------------------------------------------------------
if (NOT TARGET bitprim-core)
  find_package(bitprim-core 3.0.0 REQUIRED)
endif()

# Set flags.
#==============================================================================
include(CheckCXXCompilerFlag)

function(_add_c_compile_flag _Flag _Var)
  check_cxx_compiler_flag(${_Flag} ${_Var})
  if (${_Var})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_Flag}" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_Flag}" )
  endif()
endfunction()

function(_add_cxx_compile_flag _Flag _Var)
  check_cxx_compiler_flag(${_Flag} ${_Var})
  if (${_Var})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_Flag}" )
  endif()
endfunction()

function(_add_link_flag _Flag _Var)
  check_cxx_compiler_flag(${_Flag} ${_Var})
  if (${_Var})
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${_Flag}" )
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${_Flag}" )
  endif()
endfunction()

# Warn on all stuff.
#------------------------------------------------------------------------------
if (NOT MSVC)
  _add_c_compile_flag(-Wall _has_all_warning_flag)
else()
  _add_c_compile_flag(-W4 _has_all_warning_flag)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

# Warn on extra stuff.
#------------------------------------------------------------------------------
if (NOT MSVC)
  _add_c_compile_flag(-Wextra _has_extra_warning_flag)
endif()

# Be really annoying.
#------------------------------------------------------------------------------
_add_c_compile_flag(-Wpedantic _has_pedantic_warning_flag)
if (_has_pedantic_warning_flag)
  _add_c_compile_flag(-pedantic _has_pedantic_flag)
endif()

# Conform to style.
#------------------------------------------------------------------------------
_add_cxx_compile_flag(-Wno-missing-braces _has_no_missing_braces_warning_flag)

# Conflict in stdlib under clang. Enabled in clang only.
#------------------------------------------------------------------------------
_add_cxx_compile_flag(-Wno-mismatched-tags _has_no_mismatched_tags_warning_flag)

# Clean up boost 1.55 headers. Enabled in gcc only.
#------------------------------------------------------------------------------
_add_c_compile_flag(-Wno-deprecated-declarations _has_no_deprecated_declarations_warning_flag)

# Protect stack.
#------------------------------------------------------------------------------
_add_link_flag(-fstack-protector _has_stack_protector_flag)

# Protect stack comprehensively.
#------------------------------------------------------------------------------
_add_link_flag(-fstack-protector-all _has_stack_protector_all_flag)

# Hide internal functions from external libs. Enabled in gcc only.
#------------------------------------------------------------------------------
_add_cxx_compile_flag(-fvisibility-hidden _has_visibility_hidden_flag)

# Hide inlines from external libs. Enabled in gcc only.
#------------------------------------------------------------------------------
_add_cxx_compile_flag(-fvisibility-inlines-hidden _has_visibility_inlines_hidden_flag)

# Target Windows Vista. Enabled in msvc only.
#------------------------------------------------------------------------------
if (MSVC)
  add_definitions(-D_WIN32_WINNT=0x0600)
endif()

# Build
#==============================================================================
function(_group_sources target sources_dir)
  file(GLOB_RECURSE _headers
    ${sources_dir}/include/*.h ${sources_dir}/include/*.hpp)
  target_sources(${target} PRIVATE ${_headers})

  get_target_property(sources ${target} SOURCES)
  foreach (source ${sources})
    get_filename_component(group ${source} ABSOLUTE)
    get_filename_component(group ${group} DIRECTORY)
    file(RELATIVE_PATH group "${sources_dir}" "${group}")
    if (group)
      if (MSVC)
        string(REPLACE "/" "\\" group "${group}")
      endif()
      source_group("${group}" FILES "${source}")
    endif()
  endforeach()

  set_target_properties(${target} PROPERTIES FOLDER "protocol")
endfunction()

# src/bitprim-protocol.la => ${libdir}
#------------------------------------------------------------------------------
set(MODE STATIC)
if (ENABLE_SHARED)
  set(MODE SHARED)
endif()

set(_bitprim_proto
  proto/bitcoin/protocol/interface.proto
  proto/bitcoin/protocol/blockchain.proto
  proto/bitcoin/protocol/consensus.proto
  proto/bitcoin/protocol/database.proto)
set(_proto_outdir "${CMAKE_CURRENT_BINARY_DIR}/proto")

message("Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE} ")

foreach (_proto ${_bitprim_proto})
  get_filename_component(_proto_we ${_proto} NAME_WE)
  file(MAKE_DIRECTORY "${_proto_outdir}/bitcoin/protocol")
  set(_proto_source "${_proto_outdir}/bitcoin/protocol/${_proto_we}.pb.cc")
  set(_proto_header "${_proto_outdir}/bitcoin/protocol/${_proto_we}.pb.h")

  get_filename_component(_proto_file ${_proto} ABSOLUTE)
  add_custom_command(
    OUTPUT "${_proto_source}" "${_proto_header}"
    COMMAND ${CMAKE_COMMAND}
    ARGS    -DProtobuf_PROTOC_EXECUTABLE=${Protobuf_PROTOC_EXECUTABLE}
            -DPROTO_FILE=${_proto_file}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/protobuf_generate_cpp.cmake
    WORKING_DIRECTORY "${_proto_outdir}/bitcoin/protocol"
    MAIN_DEPENDENCY ${_proto_file}
    DEPENDS ${Protobuf_PROTOC_EXECUTABLE}
    COMMENT "Running C++ protocol buffer compiler on ${_proto}"
    VERBATIM )
  set_source_files_properties(
    ${_proto_source} ${_proto_header} PROPERTIES
    GENERATED TRUE)

  list(APPEND _proto_sources "${_proto_source}")
  list(APPEND _proto_headers "${_proto_header}")
endforeach()

add_library(bitprim-protocol ${MODE}
  src/converter.cpp
  src/packet.cpp
  src/replier.cpp
  src/request_packet.cpp
  src/requester.cpp
  src/requester_simple.cpp
  src/response_packet.cpp
  src/zmq/authenticator.cpp
  src/zmq/certificate.cpp
  src/zmq/context.cpp
  src/zmq/frame.cpp
  src/zmq/identifiers.cpp
  src/zmq/message.cpp
  src/zmq/poller.cpp
  src/zmq/socket.cpp
  src/zmq/worker.cpp
  src/zmq/zeromq.cpp)
target_sources(bitprim-protocol PRIVATE ${_bitprim_proto})
target_include_directories(bitprim-protocol PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_compile_definitions(bitprim-protocol PUBLIC -DBCP_STATIC)

target_link_libraries(bitprim-protocol bitprim-core)

target_sources(bitprim-protocol PRIVATE ${_proto_sources})
target_include_directories(bitprim-protocol PUBLIC
  $<BUILD_INTERFACE:${_proto_outdir}>
  $<INSTALL_INTERFACE:include>)
target_include_directories(bitprim-protocol PUBLIC ${Protobuf_INCLUDE_DIRS})
target_link_libraries(bitprim-protocol ${Protobuf_LIBRARY})

target_include_directories(bitprim-protocol PUBLIC ${ZeroMQ_INCLUDE_DIR})
target_link_libraries(bitprim-protocol ${ZeroMQ_LIBRARIES})

_group_sources(bitprim-protocol "${CMAKE_CURRENT_LIST_DIR}")

# Tests
#==============================================================================
function(_add_tests target)
  if (ENABLE_SHARED)
    target_compile_definitions(${target} PRIVATE -DBOOST_TEST_DYN_LINK)
  endif()
  target_include_directories(${target} SYSTEM PUBLIC ${Boost_INCLUDE_DIR})
  target_link_libraries(${target} PUBLIC ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

  foreach (_test_name ${ARGN})
    add_test(
      NAME test.protocol.${_test_name}
      COMMAND ${target}
        --run_test=${_test_name}
        --show_progress=no
        --detect_memory_leak=0
        --report_level=no
        --build_info=yes)
  endforeach()
endfunction()

if (WITH_TESTS)
  enable_testing()
endif()

# local: test/bitprim_protocol_test
#------------------------------------------------------------------------------
if (WITH_TESTS)
  add_executable(bitprim_protocol_test
    test/converter.cpp
    test/main.cpp
    test/examples/authenticator_example.cpp
    test/examples/poller_example.cpp
    test/zmq/authenticator.cpp
    test/zmq/certificate.cpp
    test/zmq/context.cpp
    test/zmq/frame.cpp
    test/zmq/identifiers.cpp
    test/zmq/message.cpp
    test/zmq/poller.cpp
    test/zmq/socket.cpp
    test/zmq/worker.cpp)
  target_link_libraries(bitprim_protocol_test PUBLIC bitprim-protocol)
  _group_sources(bitprim_protocol_test "${CMAKE_CURRENT_LIST_DIR}/test")

  _add_tests(bitprim_protocol_test
    authenticator_tests
    certificate_tests
    context_tests
    converter_tests
    frame_tests
    identifiers_tests
    message_tests
    poller_tests
    socket_tests
    worker_tests)
endif()

# Install
#==============================================================================
install(TARGETS bitprim-protocol
  EXPORT bitprim-protocol
  ARCHIVE DESTINATION lib)

set(_bitprim_headers
  # include_bitcoin_HEADERS =
  bitcoin/protocol.hpp
  # include_bitcoin_protocol_HEADERS =
  bitcoin/protocol/converter.hpp
  bitcoin/protocol/define.hpp
  bitcoin/protocol/packet.hpp
  bitcoin/protocol/primitives.hpp
  bitcoin/protocol/replier.hpp
  bitcoin/protocol/request_packet.hpp
  bitcoin/protocol/requester.hpp
  bitcoin/protocol/requester_simple.hpp
  bitcoin/protocol/response_packet.hpp
  bitcoin/protocol/version.hpp
  # include_bitcoin_protocol_zmq_HEADERS =
  bitcoin/protocol/zmq/authenticator.hpp
  bitcoin/protocol/zmq/certificate.hpp
  bitcoin/protocol/zmq/context.hpp
  bitcoin/protocol/zmq/frame.hpp
  bitcoin/protocol/zmq/identifiers.hpp
  bitcoin/protocol/zmq/message.hpp
  bitcoin/protocol/zmq/poller.hpp
  bitcoin/protocol/zmq/socket.hpp
  bitcoin/protocol/zmq/worker.hpp
  bitcoin/protocol/zmq/zeromq.hpp)
foreach (_header ${_bitprim_headers})
  get_filename_component(_directory "${_header}" DIRECTORY)
  install(FILES "include/${_header}" DESTINATION "include/${_directory}")
endforeach()

foreach (_header ${_proto_headers})
  get_filename_component(_directory ${_header} ABSOLUTE)
  get_filename_component(_directory ${_directory} PATH)
  file(RELATIVE_PATH _directory ${_proto_outdir} ${_directory})
  install(FILES "${_header}" DESTINATION "include/${_directory}")
endforeach()

# Export
#==============================================================================
include(CMakePackageConfigHelpers)

export(EXPORT bitprim-protocol
  FILE "${CMAKE_CURRENT_BINARY_DIR}/bitprim-protocolTargets.cmake")
export(PACKAGE bitprim-protocol)

if (UNIX)
  set(_config_package_location "lib/cmake")
elseif (WIN32)
  set(_config_package_location "cmake")
endif()

install(EXPORT bitprim-protocol
  FILE bitprim-protocolTargets.cmake
  DESTINATION ${_config_package_location})

configure_file(bitprim-protocolConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/bitprim-protocolConfig.cmake"
  @ONLY)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/bitprim-protocolConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/bitprim-protocolConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/bitprim-protocolConfigVersion.cmake"
  DESTINATION ${_config_package_location})
