#-------------------------------------------------------------------
# Flags common to all Linux based platforms with GNU compiler
#-------------------------------------------------------------------

include(${CMAKE_SOURCE_DIR}/cmake/platforms/Linux.cmake)

# Warning flags
set(NORMAL_WARNINGS -Wall -Wextra)
set(FULL_WARNINGS
    ${NORMAL_WARNINGS}
    -pedantic
    -Wno-long-long
    # Detect conversion problems (lot of warnings)
    -Wconversion
    -Wsign-conversion
    -Wdouble-promotion
)

# Compile with full warnings by default
add_definitions(${FULL_WARNINGS})

# Warn about missing virtual destructor (C++ only)
add_flags(CMAKE_CXX_FLAGS -Wnon-virtual-dtor)

# Add static and dynamic bounds checks (optimization required)
add_flags(CMAKE_CXX_FLAGS_RELEASE -D_FORTIFY_SOURCE=2)
add_flags(CMAKE_C_FLAGS_RELEASE -D_FORTIFY_SOURCE=2)

# Enable SSE3 instruction set
add_flags(CMAKE_CXX_FLAGS -msse3)
add_flags(CMAKE_C_FLAGS -msse3)

# Enable glibc parallel mode
#add_flags(CMAKE_CXX_FLAGS -D_GLIBCXX_PARALLEL)

# Enable some algebraic transforms
# (necessary for vectorizing certain reductions and dot products)
#!!! -- deactivated, because it breaks Shewchuck's predicates --
#add_flags(CMAKE_CXX_FLAGS -ffast-math -fassociative-math)
#add_flags(CMAKE_C_FLAGS -ffast-math -fassociative-math)
#-ftree-vectorizer-verbose=2 

# Generate debug information even in release mode
#add_flags(CMAKE_CXX_FLAGS_RELEASE -g)
#add_flags(CMAKE_C_FLAGS_RELEASE -g)


# Additional debug flags
# deactivated for now: I added bound checking in VOR::vector<>.
#add_flags(CMAKE_CXX_FLAGS_DEBUG -D_GLIBCXX_DEBUG)


# Compile and link with OpenMP
add_flags(CMAKE_CXX_FLAGS -fopenmp)
add_flags(CMAKE_C_FLAGS -fopenmp)


# Profiler compilation flags
if(VORPALINE_WITH_GPROF)
    message(STATUS "Building for code profiling")
    add_flags(CMAKE_CXX_FLAGS -pg -DPROFILER)
    add_flags(CMAKE_C_FLAGS -pg -DPROFILER)
endif()


# Code coverage compilation flags
if(VORPALINE_WITH_GCOV)
    message(STATUS "Building for coverage analysis")
    add_flags(CMAKE_CXX_FLAGS --coverage)
    add_flags(CMAKE_C_FLAGS --coverage)
endif()


# Compilation flags for Google's AddressSanitizer
# These flags can only be specified for dynamic builds
if(VORPALINE_WITH_ASAN)
    if(VORPALINE_BUILD_DYNAMIC)
        message(STATUS "Building with AddressSanitizer (debug only)")
        add_flags(CMAKE_CXX_FLAGS_DEBUG -fsanitize=address -fno-omit-frame-pointer)
        add_flags(CMAKE_C_FLAGS_DEBUG -fsanitize=address -fno-omit-frame-pointer)
    else()
        message(WARNING "AddressSanitizer can be used with dynamic builds only")
        set(VORPALINE_WITH_ASAN false)
    endif()
endif()
if(NOT VORPALINE_WITH_ASAN)
    # Use native GCC stack smash Protection and buffer overflow detection (debug only)
    add_flags(CMAKE_CXX_FLAGS_DEBUG -fstack-protector-all)
    add_flags(CMAKE_C_FLAGS_DEBUG -fstack-protector-all)
endif()


# Compilation flags for Google's ThreadSanitizer
# Does not work for the moment: cannot figure out how to link with library libtsan
if(VORPALINE_WITH_TSAN)
    message(STATUS "Building with ThreadSanitizer (debug only)")
    message(FATAL_ERROR "ThreadSanitizer is not available: cannot figure out how to link with library libtsan")
    add_flags(CMAKE_CXX_FLAGS_DEBUG -fsanitize=thread)
    add_flags(CMAKE_C_FLAGS_DEBUG -fsanitize=thread)
    if(NOT VORPALINE_BUILD_DYNAMIC)
        add_flags(CMAKE_EXE_LINKER_FLAGS -static-libtsan)
    endif()
endif()


# Reset the warning level for third parties
function(vor_reset_warning_level)
    remove_definitions(${FULL_WARNINGS})
    add_definitions(${NORMAL_WARNINGS})
endfunction()

macro(vor_add_executable)
    if(NOT VORPALINE_BUILD_DYNAMIC)
        # Create a statically linked executable
        # Link with static libraries
        add_flags(CMAKE_CXX_FLAGS -static-libstdc++ -static-libgcc -static)
        add_flags(CMAKE_C_FLAGS -static-libgcc -static)
    endif()

    add_executable(${ARGN})

    if(NOT VORPALINE_BUILD_DYNAMIC AND DEFINED VORPALINE_WITH_DDT)
        # Static builds running with Allinea's DDT must be linked with a
        # special malloc library which replaces the malloc primitives of
        # the Glibc (We must allow multiple definitions)
        add_flags(CMAKE_EXE_LINKER_FLAGS -Wl,--allow-multiple-definition)

        if(VORPALINE_ARCH_64)
            link_directories(${VORPALINE_WITH_DDT}/lib/64)
        else()
            link_directories(${VORPALINE_WITH_DDT}/lib/32)
        endif()
        target_link_libraries(${ARGV0} dmallocthcxx)
    endif()

    if(UNIX)
        target_link_libraries(${ARGV0} m pthread)
    endif()

endmacro()

