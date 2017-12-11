# Define the project RINGMesh
project(RINGMesh)

#------------------------------------------------------------------------------------------------
# Turn on the ability to create folders to organize projects and files
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define version number
# It is then exported to the configuration file
set(RINGMesh_VERSION_MAJOR 5)
set(RINGMesh_VERSION_MINOR 0)
set(RINGMesh_VERSION_PATCH 0)
set(RINGMesh_VERSION ${RINGMesh_VERSION_MAJOR}.${RINGMesh_VERSION_MINOR}.${RINGMesh_VERSION_PATCH})

message(STATUS "RINGMesh binary directory is: ${PROJECT_BINARY_DIR}")
message(STATUS "RINGMesh source directory is: ${PROJECT_SOURCE_DIR}")

include(cmake/utils.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
#------------------------------------------------------------------------------------------------
# Platform dependent settings
if(UNIX)
    add_compile_options(-Wall -Wextra -Wno-long-long -Wconversion
        -Wsign-conversion -Wdouble-promotion -Wno-attributes)

    if(APPLE)
        if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            message(WARNING "RINGMesh on Apple is only tested with Clang compiler")
        endif()
        set(CMAKE_MACOSX_RPATH ON)
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    else(APPLE)
        # pthread is ignored on MacOS
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    endif(APPLE)

    if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
       include(cmake/Coverage.cmake)
    endif()
else(UNIX)
    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        message(WARNING "RINGMesh on Windows is only tested with Microsoft Visual C++")
    endif()
    add_custom_target(copy_dll ALL)
endif(UNIX)

# RINGMesh depends on Geogram Tinyxml2 Minizip and zlib
list(APPEND CMAKE_MODULE_PATH ${GEOGRAM_INSTALL_PREFIX}/lib/cmake/modules)
set(VORPALINE_BUILD_DYNAMIC TRUE CACHE BOOL "")
find_package(Geogram REQUIRED)
find_package(ZLIB REQUIRED)
find_package(tinyxml2 REQUIRED PATHS ${TINYXML2_INSTALL_PREFIX})
include(${MINIZIP_INSTALL_PREFIX}/cmake/minizip-exports.cmake)

#------------------------------------------------------------------------------------------------
# Build configuration
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

#------------------------------------------------------------------------------------------------
# file automatically generated by cmake

# generate configure file to pass on some of the CMake settings
# to the source code
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/ringmesh_config.h.in 
    ${PROJECT_BINARY_DIR}/ringmesh/ringmesh_config.h
)

# Exports RINGMesh target
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
configure_package_config_file(
    cmake/RINGMeshConfig.cmake.in 
    ${CMAKE_BINARY_DIR}/RINGMeshConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}
    PATH_VARS GEOGRAM_INSTALL_PREFIX
)

#------------------------------------------------------------------------------------------------
# Configure the ringmesh libraries

add_ringmesh_library(basic)
set(MANDATORY_RINGMESH_LIBRARIES basic)
set(WANTED_RINGMESH_LIBRARIES basic)

if(RINGMESH_WITH_GEOMODEL_CORE)
    add_ringmesh_library(geomodel/core)
endif(RINGMESH_WITH_GEOMODEL_CORE)
if(RINGMESH_WITH_GEOMODEL_BUILDER)
    add_ringmesh_library(geomodel/builder)
endif(RINGMESH_WITH_GEOMODEL_BUILDER)
if(RINGMESH_WITH_GEOMODEL_TOOLS)
    add_ringmesh_library(geomodel/tools)
endif(RINGMESH_WITH_GEOMODEL_TOOLS)
if(RINGMESH_WITH_GEOGRAM_EXTENSION)
    add_ringmesh_library(geogram_extension)
endif(RINGMESH_WITH_GEOGRAM_EXTENSION)
if(RINGMESH_WITH_IO)
    add_ringmesh_library(io)
endif(RINGMESH_WITH_IO)
if(RINGMESH_WITH_MESH)
    add_ringmesh_library(mesh)
endif(RINGMESH_WITH_MESH)
if(RINGMESH_WITH_TETRAHEDRALIZE)
    add_ringmesh_library(tetrahedralize)
endif(RINGMESH_WITH_TETRAHEDRALIZE)
if(RINGMESH_WITH_GRAPHICS)
    add_ringmesh_library(visualize)
endif(RINGMESH_WITH_GRAPHICS)


list(REMOVE_DUPLICATES MANDATORY_RINGMESH_LIBRARIES)
foreach(mandatory_ringmesh_lib ${MANDATORY_RINGMESH_LIBRARIES})
    list(FIND WANTED_RINGMESH_LIBRARIES ${mandatory_ringmesh_lib} item_index)
    if(${item_index} EQUAL -1)
        message(WARNING "Library ${mandatory_ringmesh_lib} is mandatory according to the ones you have selected.")
        message(FATAL_ERROR "Check the library ${mandatory_ringmesh_lib} or uncheck all the libraries which depend on it.")
    endif()
endforeach(mandatory_ringmesh_lib ${MANDATORY_RINGMESH_LIBRARIES})

#------------------------------------------------------------------------------------------------
# Optional modules configuration

set(binary_source_dir ${PROJECT_SOURCE_DIR}/src/bin)
if(BUILD_RINGMESH_VIEW)
    message(STATUS "Configure ringmesh-view")
    add_ringmesh_binary(${binary_source_dir}/ringmesh-view.cpp visualize)
    copy_for_windows(${PROJECT_BINARY_DIR}/bin)
endif()

if(RINGMESH_WITH_UTILITIES)
    message(STATUS "Configuring RINGMesh with utilities")
    # Get the paths of the utility files
    file(GLOB utility_sources "${binary_source_dir}/utilities/*.cpp")
    foreach(utility_src ${utility_sources})
        add_ringmesh_utility(${utility_src} geomodel_tools io)
    endforeach()
    copy_for_windows(${PROJECT_BINARY_DIR}/bin/utilities)
endif()

if(RINGMESH_WITH_TUTORIALS)
    message(STATUS "Configuring RINGMesh with tutorials")
    add_subdirectory(doc/tutorials)
    copy_for_windows(${PROJECT_BINARY_DIR}/bin/tutorials)
endif()

if(RINGMESH_WITH_TESTS)
    # Enable testing with CTest
    enable_testing()
    message(STATUS "Configuring RINGMesh with tests")
    add_subdirectory(tests)
    copy_for_windows(${PROJECT_BINARY_DIR}/bin/tests)
endif()

#------------------------------------------------------------------------------------------------
# Configure CPack

set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${RINGMesh_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${RINGMesh_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${RINGMesh_VERSION_PATCH})
set(CPACK_PACKAGE_VENDOR "RING-TEAM (www.ring-team.org)")
set(CPACK_SOURCE_GENERATOR "ZIP")

set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;/_CPack_Packages/")

# This must always be last!
include(CPack)
