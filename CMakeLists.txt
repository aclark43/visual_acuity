########################################################################################################################
#
# Copyright (c) 2017-2020 Santini Designs. All rights reserved.
#

cmake_minimum_required(VERSION 3.10.2)

set(__TARGET_NAME visual_acuity)

project(${__TARGET_NAME} VERSION 1.0.0 LANGUAGES CXX)

########################################################################################################################
set(SD_EYERIS_RUNTIME_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

set(SD_EYERIS_DEVELOPMENT_DIR ${SD_EYERIS_RUNTIME_DIR}/dev_ws)
set(SD_EYERIS_APP_DIR ${SD_EYERIS_RUNTIME_DIR}/eyeris_machine)
set(SD_EYERIS_APP_BINARY_DIR ${SD_EYERIS_APP_DIR}/bin)
set(SD_EYERIS_INCLUDES_DIR ${SD_EYERIS_APP_DIR}/include)
set(SD_EYERIS_JOBS_DIR ${SD_EYERIS_APP_DIR}/plugins/jobs)

########################################################################################################################
find_package(GTest REQUIRED)

find_library(IL_LIBRARY IL)

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

find_library(M_LIBRARY m)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(OpenSSL REQUIRED)

find_package(X11 REQUIRED)
find_library(X11_LIBRARY X11)

find_library(UNWIND_LIBRARY unwind)

########################################################################################################################
set(SD_EYERIS_LIBRARIES_ADDITIONAL_LIST $ENV{SD_ADDITIONAL_LIBRARIES})
set(SD_EYERIS_INCLUDES_ADDITIONAL_LIST $ENV{SD_ADDITIONAL_INCLUDES})

########################################################################################################################
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SD_DEBUG_ENABLED ON)
else()
    set(SD_DEBUG_ENABLED OFF)
endif()

########################################################################################################################
if(SD_DEBUG_ENABLED)
    set(SD_EYERIS_LIBRARIES_CORE_LIST
        boost_system
        ${CMAKE_DL_LIBS}
        pthread
        rt
        ${UNWIND_LIBRARY}
        stdc++fs
        ${GTEST_LIBRARIES}
    )
else()
    set(SD_EYERIS_LIBRARIES_CORE_LIST
        boost_system
        ${CMAKE_DL_LIBS}
        pthread
        rt
        ${UNWIND_LIBRARY}
        stdc++fs
    )
endif()

########################################################################################################################
set(SD_EYERIS_LIBRARIES_DB_LIST
    mongo::mongocxx_shared
)

########################################################################################################################
set(SD_EYERIS_LIBRARIES_GRAPHICS_LIST
    freetype
    GLdispatch
    GLEW
    glfw3
    glut
    ${IL_LIBRARY}
    ${OPENGL_LIBRARY}
    ${X11_LIBRARY}
)

set(SD_EYERIS_INCLUDES_GRAPHICS_LIST
    /usr/include/freetype2
)

########################################################################################################################
set(SD_EYERIS_LIBRARIES_MATH_LIST
    ${M_LIBRARY}
)

########################################################################################################################
set(SD_EYERIS_LIBRARIES_SSL_LIST
    ${OPENSSL_LIBRARIES}
)

set(SD_EYERIS_INCLUDES_SSL_LIST
    ${OPENSSL_INCLUDE_DIR}
)

#########################################################################################################################
set(SD_EYERIS_LIBRARIES_LIST
    ${SD_EYERIS_APP_BINARY_DIR}/libbasic.so
    ${SD_EYERIS_APP_BINARY_DIR}/libeye.so
    ${SD_EYERIS_APP_BINARY_DIR}/libhal.so
)

########################################################################################################################
# Macro definition for Google tests
#
macro(package_add_test TARGET_NAME TEST_SUITE_NAME TEST_DIRECTORY)
    file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIRECTORY}/*.cpp)
    add_executable(${TEST_SUITE_NAME}
        ${TEST_FILES}
    )
    target_include_directories(${TEST_SUITE_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${SD_EYERIS_INCLUDES_DIR}
            ${SD_EYERIS_INCLUDES_GRAPHICS_LIST}
            ${SD_EYERIS_INCLUDES_SSL_LIST}
            ${SD_EYERIS_INCLUDES_ADDITIONAL_LIST}
    )
    target_compile_features(${TEST_SUITE_NAME}
        PRIVATE
            cxx_std_17
    )
    target_link_libraries(${TEST_SUITE_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/output/lib${TARGET_NAME}.so
            ${SD_EYERIS_LIBRARIES_LIST}
            ${SD_EYERIS_LIBRARIES_CORE_LIST}
            ${SD_EYERIS_LIBRARIES_GRAPHICS_LIST}
            ${SD_EYERIS_LIBRARIES_MATH_LIST}
            ${SD_EYERIS_LIBRARIES_SSL_LIST}
            ${SD_EYERIS_LIBRARIES_ADDITIONAL_LIST}
    )
    add_test(${TEST_SUITE_NAME} ${TEST_SUITE_NAME})
    set_target_properties(${TEST_SUITE_NAME} PROPERTIES FOLDER tests)
    add_dependencies(${TEST_SUITE_NAME} ${TARGET_NAME})
endmacro()

########################################################################################################################
set(SD_PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SD_PROJECT_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/output)
set(SD_PROJECT_OUTPUT_ASSETS_DIR ${SD_PROJECT_OUTPUT_DIR}/assets)
set(SD_PROJECT_SRC_ASSETS_DIR ${SD_PROJECT_ROOT_DIR}/assets)
set(SD_PROJECT_DESTINATION_DIR ${SD_EYERIS_JOBS_DIR}/${__TARGET_NAME})

set(SD_PROJECT_ASSETS_LIST
    ${SD_PROJECT_ROOT_DIR}/opicon_small.png
    ${SD_PROJECT_ROOT_DIR}/opicon_large.png
    ${SD_PROJECT_ROOT_DIR}/optag.json
)

########################################################################################################################
# Build the task .so file
#
file(GLOB_RECURSE SD_PROJECT_SOURCES_LIST ${SD_PROJECT_ROOT_DIR}/src/*.cpp)
file(GLOB_RECURSE SD_PROJECT_HEADERS_LIST ${SD_PROJECT_ROOT_DIR}/include/*.hpp)

add_library(${__TARGET_NAME}
    SHARED
        ${SD_PROJECT_SOURCES_LIST}
        ${SD_PROJECT_HEADERS_LIST}
)
set_target_properties(${__TARGET_NAME}
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${SD_PROJECT_OUTPUT_DIR}
)

target_compile_definitions(${__TARGET_NAME}
    PUBLIC
        __PLUGIN_NAME__="\\"${PROJECT_NAME}\\""
    INTERFACE
        TW_STATIC
        TW_NO_LIB_PRAGMA
        TW_NO_DIRECT3D
        GLEW_STATIC
        _CRT_SECURE_NO_WARNINGS
)

target_include_directories(${__TARGET_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${SD_PROJECT_ROOT_DIR}/include>
        ${SD_EYERIS_INCLUDES_ADDITIONAL_LIST}
        ${SD_EYERIS_INCLUDES_DIR}
        ${SD_EYERIS_INCLUDES_GRAPHICS_LIST}
)

target_compile_options(${__TARGET_NAME}
    PRIVATE
        -Winconsistent-missing-override
        -fstandalone-debug
)

target_compile_features(${__TARGET_NAME}
    PRIVATE
        cxx_std_17
)

target_link_libraries(${__TARGET_NAME}
    PUBLIC
        ${SD_BASIC_LIBRARY_TARGET_NAME}
        ${SD_HAL_LIBRARY_TARGET_NAME}
        ${SD_EYE_LIBRARY_TARGET_NAME}
        ${SD_EYERIS_LIBRARIES_ADDITIONAL_LIST}
        ${SD_EYERIS_LIBRARIES_CORE_LIST}
        ${SD_EYERIS_LIBRARIES_GRAPHICS_LIST}
        ${SD_EYERIS_LIBRARIES_MATH_LIST}
)

########################################################################################################################
# Copy assets directory and icons to the destination directory
#
add_custom_command(TARGET ${__TARGET_NAME}
    PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SD_PROJECT_DESTINATION_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SD_PROJECT_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SD_PROJECT_OUTPUT_DIR}
)

add_custom_command(TARGET ${__TARGET_NAME}
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SD_PROJECT_SRC_ASSETS_DIR} ${SD_PROJECT_OUTPUT_ASSETS_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${SD_PROJECT_ASSETS_LIST} ${SD_PROJECT_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SD_PROJECT_OUTPUT_DIR} ${SD_PROJECT_DESTINATION_DIR}
)
