cmake_minimum_required(VERSION 3.15)
project(netlayd VERSION 1.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

if(NOT DEFINED AOO_ROOT)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../deps/aoo")
        set(AOO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../deps/aoo")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/aoo")
        set(AOO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/aoo")
    else()
        message(FATAL_ERROR "AOO sources not found (expected ../deps/aoo or ./aoo)")
    endif()
endif()

add_library(aoo_netlay STATIC
    ${AOO_ROOT}/lib/src/client.cpp
    ${AOO_ROOT}/lib/src/codec_opus.cpp
    ${AOO_ROOT}/lib/src/codec_pcm.cpp
    ${AOO_ROOT}/lib/src/common.cpp
    ${AOO_ROOT}/lib/src/net_utils.cpp
    ${AOO_ROOT}/lib/src/server.cpp
    ${AOO_ROOT}/lib/src/sink.cpp
    ${AOO_ROOT}/lib/src/source.cpp
    ${AOO_ROOT}/lib/src/sync.cpp
    ${AOO_ROOT}/lib/src/time.cpp
    ${AOO_ROOT}/deps/md5/md5.c
    ${AOO_ROOT}/deps/oscpack/osc/OscOutboundPacketStream.cpp
    ${AOO_ROOT}/deps/oscpack/osc/OscPrintReceivedElements.cpp
    ${AOO_ROOT}/deps/oscpack/osc/OscReceivedElements.cpp
    ${AOO_ROOT}/deps/oscpack/osc/OscTypes.cpp
)

set_source_files_properties(${AOO_ROOT}/deps/md5/md5.c PROPERTIES LANGUAGE C)

target_include_directories(aoo_netlay PUBLIC
    ${AOO_ROOT}/lib
    ${AOO_ROOT}/deps
    ${AOO_ROOT}/deps/oscpack
)

target_compile_definitions(aoo_netlay PUBLIC
    USE_CODEC_OPUS=1
    AOO_STATIC
    AOO_TIMEFILTER_CHECK=0
    OSC_HOST_LITTLE_ENDIAN=1
    LOGLEVEL=1
)

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_OPUS QUIET opus)
    pkg_check_modules(PC_ALSA QUIET alsa)
endif()

find_path(OPUS_INCLUDE_DIR opus/opus.h HINTS ${PC_OPUS_INCLUDEDIR} ${PC_OPUS_INCLUDE_DIRS})
find_library(OPUS_LIBRARY NAMES opus HINTS ${PC_OPUS_LIBDIR} ${PC_OPUS_LIBRARY_DIRS})
if(NOT OPUS_INCLUDE_DIR OR NOT OPUS_LIBRARY)
    message(FATAL_ERROR "libopus not found (need libopus / opus headers)")
endif()

find_path(ALSA_INCLUDE_DIR alsa/asoundlib.h HINTS ${PC_ALSA_INCLUDEDIR} ${PC_ALSA_INCLUDE_DIRS})
find_library(ALSA_LIBRARY NAMES asound HINTS ${PC_ALSA_LIBDIR} ${PC_ALSA_LIBRARY_DIRS})
if(NOT ALSA_INCLUDE_DIR OR NOT ALSA_LIBRARY)
    message(FATAL_ERROR "ALSA not found (need alsa-lib)")
endif()

target_include_directories(aoo_netlay PUBLIC ${OPUS_INCLUDE_DIR})
target_link_libraries(aoo_netlay PUBLIC ${OPUS_LIBRARY})

find_package(Threads REQUIRED)

add_executable(netlayd
    src/netlayd/main.cpp
)

target_include_directories(netlayd PRIVATE
    ${ALSA_INCLUDE_DIR}
    ${OPUS_INCLUDE_DIR}
)

target_link_libraries(netlayd PRIVATE
    aoo_netlay
    ${ALSA_LIBRARY}
    Threads::Threads
)

install(TARGETS netlayd RUNTIME DESTINATION bin)
