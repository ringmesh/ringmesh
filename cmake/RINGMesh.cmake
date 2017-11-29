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
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    
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
endif(UNIX)

#------------------------------------------------------------------------------------------------
# Get MG-Tetra if a path is given
if(MG_TETRA)
  include_directories(SYSTEM ${MG_TETRA}/include)

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM _64)
  else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

  # Is there not a nice way to import this library? [JP]
  if(WIN32)
    link_directories(${MG_TETRA}/lib/WIN7${PLATFORM}_VC10)
  else(WIN32)
    link_directories(${MG_TETRA}/lib/Linux${PLATFORM})
  endif(WIN32)

  set(EXTRA_LIBS ${EXTRA_LIBS} meshgems mg-tetra meshgems_stubs)
endif()


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
# Collect the library files
#add_folder(ringmesh_src ringmesh_include basic)
#add_folder(ringmesh_src ringmesh_include geogram_extension)
#add_folder(ringmesh_src ringmesh_include geomodel/builder)
#add_folder(ringmesh_src ringmesh_include geomodel/core)
#add_folder(ringmesh_src ringmesh_include geomodel/tools)
#add_folder(ringmesh_src ringmesh_include io)
#add_folder(ringmesh_src ringmesh_include mesh)
#add_folder(ringmesh_src ringmesh_include tetrahedralize)
#add_folder(ringmesh_src ringmesh_include visualize)

# Add the RINGMesh target as a shared library
#add_library(RINGMesh SHARED "")

# Exports RINGMesh target
include ( CMakePackageConfigHelpers )
#export(TARGETS RINGMesh NAMESPACE RINGMesh:: FILE RINGMeshTargets.cmake)
configure_package_config_file(
    cmake/RINGMeshConfig.cmake.in 
    ${CMAKE_BINARY_DIR}/RINGMeshConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}
    PATH_VARS GEOGRAM_INSTALL_PREFIX
)


add_ringmesh_library(basic)
add_ringmesh_library(geogram_extension)
add_ringmesh_library(geomodel/builder)
add_ringmesh_library(geomodel/core)
add_ringmesh_library(geomodel/tools)
add_ringmesh_library(io)
add_ringmesh_library(mesh)
add_ringmesh_library(tetrahedralize)
if(RINGMESH_WITH_GRAPHICS)
    add_ringmesh_library(visualize)
endif(RINGMESH_WITH_GRAPHICS)

# Add include directories of RINGMesh
#target_include_directories(RINGMesh PUBLIC ${PROJECT_SOURCE_DIR}/include)

#target_link_libraries(RINGMesh PUBLIC Geogram::geogram PRIVATE ZLIB::ZLIB tinyxml2 MINIZIP::minizip)

if(RINGMESH_WITH_GRAPHICS)
#    target_link_libraries(RINGMesh PUBLIC Geogram::geogram_gfx)
endif(RINGMESH_WITH_GRAPHICS)

# Libraries to which RINGMesh should link
#target_link_libraries(RINGMesh LINK_PRIVATE ${EXTRA_LIBS})

#------------------------------------------------------------------------------------------------
# file automatically generated by cmake

# generate configure file to pass on some of the CMake settings
# to the source code
# Add configure file to ringmesh includes

configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/ringmesh_config.h.in 
    ${PROJECT_BINARY_DIR}/ringmesh/ringmesh_config.h
)

#generate macros for API export and add it to include directories
include (GenerateExportHeader)
set(generated_export_file ${PROJECT_BINARY_DIR}/ringmesh/ringmesh_export.h)
generate_export_header(basic EXPORT_MACRO_NAME RINGMESH_API EXPORT_FILE_NAME ${generated_export_file} )

# We want to be able to include these file
#target_include_directories(RINGMesh PUBLIC ${PROJECT_BINARY_DIR})

#------------------------------------------------------------------------------------------------
# Optional modules configuration

# ringmesh_files definition must be before the binaries (utilities, ringmesh-viewm, etc.).
# Thus this variable can be updated by the called cmake files.
# This line is for clang utilities.
set(ringmesh_files ${ringmesh_include} ${ringmesh_src})

# Collect the IO files
file(GLOB directories RELATIVE ${PROJECT_SOURCE_DIR}/src/ringmesh/io ${PROJECT_SOURCE_DIR}/src/ringmesh/io/*)
foreach(directory ${directories})
    if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/src/ringmesh/io/${directory})
        source_file_directory(ringmesh_files io/${directory})
    endif()
endforeach()

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
        add_ringmesh_utility(${utility_src})
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
