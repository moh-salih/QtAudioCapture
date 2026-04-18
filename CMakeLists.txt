cmake_minimum_required(VERSION 3.16)

project(QtAudioCapture VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Multimedia)

add_library(QtAudioCapture STATIC
    include/QtAudioCapture/Types.h
    include/QtAudioCapture/AudioResampler.h
    include/QtAudioCapture/AudioRecorder.h
    include/QtAudioCapture/AudioFileDecoder.h
    include/QtAudioCapture/AudioPipeline.h

    src/AudioResampler.cpp
    src/AudioRecorder.cpp
    src/AudioFileDecoder.cpp
    src/AudioPipeline.cpp
)

add_library(QtAudioCapture::QtAudioCapture ALIAS QtAudioCapture)

target_include_directories(QtAudioCapture PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(QtAudioCapture PUBLIC Qt6::Core Qt6::Multimedia)

set_target_properties(QtAudioCapture PROPERTIES
    AUTOMOC ON
    CXX_STANDARD 23
)

# ==============================================================================
# INSTALLATION & EXPORT LOGIC
# ==============================================================================
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS QtAudioCapture
    EXPORT QtAudioCaptureTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT QtAudioCaptureTargets
    FILE QtAudioCaptureTargets.cmake
    NAMESPACE QtAudioCapture::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtAudioCapture
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtAudioCaptureConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QtAudioCaptureConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtAudioCapture
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/QtAudioCaptureConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/QtAudioCaptureConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/QtAudioCaptureConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtAudioCapture
)
