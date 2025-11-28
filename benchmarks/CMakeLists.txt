cmake_minimum_required(VERSION 3.10)
project(Benchmarks C)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "-Wall -O2")

# Output directory for all object files
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks-build)
file(MAKE_DIRECTORY ${BUILD_DIR})

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)

# CRC32
add_library(crc32_obj OBJECT crc32/crc_32.c)
set_target_properties(crc32_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Cubic
add_library(cubic_obj OBJECT cubic/libcubic.c cubic/basicmath_small.c)
set_target_properties(cubic_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Dijkstra
add_library(dijkstra_obj OBJECT dijkstra/dijkstra_small.c)
set_target_properties(dijkstra_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# FDCT
add_library(fdct_obj OBJECT fdct/libfdct.c)
set_target_properties(fdct_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# FIR
add_library(fir_obj OBJECT fir/libfir.c)
set_target_properties(fir_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Matmult
add_library(matmult_obj OBJECT matmult/matmult.c)
target_compile_definitions(matmult_obj PRIVATE MATMULT_INT)
set_target_properties(matmult_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Matmult-float
add_library(matmult_float_obj OBJECT matmult-float/matmult.c)
target_compile_definitions(matmult_float_obj PRIVATE MATMULT_FLOAT)
set_target_properties(matmult_float_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Matmult-int
add_library(matmult_int_obj OBJECT matmult-int/matmult.c)
target_compile_definitions(matmult_int_obj PRIVATE MATMULT_INT)
set_target_properties(matmult_int_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Nettle-SHA256
add_library(nettle_sha256_obj OBJECT nettle-sha256/nettle-sha256.c)
set_target_properties(nettle_sha256_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Rijndael
add_library(rijndael_obj OBJECT rijndael/aesxam.c rijndael/aes.c)
set_target_properties(rijndael_obj PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Custom target to copy object files after they are built
add_custom_target(copy_objects ALL
    DEPENDS crc32_obj cubic_obj dijkstra_obj fdct_obj fir_obj 
            matmult_obj matmult_float_obj matmult_int_obj 
            nettle_sha256_obj rijndael_obj
    COMMAND ${CMAKE_COMMAND} -E echo "Copying object files to ${BUILD_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/crc32_obj.dir/crc32/crc_32.c.o ${BUILD_DIR}/crc_32.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/cubic_obj.dir/cubic/libcubic.c.o ${BUILD_DIR}/libcubic.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/cubic_obj.dir/cubic/basicmath_small.c.o ${BUILD_DIR}/basicmath_small.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/dijkstra_obj.dir/dijkstra/dijkstra_small.c.o ${BUILD_DIR}/dijkstra_small.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/fdct_obj.dir/fdct/libfdct.c.o ${BUILD_DIR}/libfdct.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/fir_obj.dir/fir/libfir.c.o ${BUILD_DIR}/libfir.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/matmult_obj.dir/matmult/matmult.c.o ${BUILD_DIR}/matmult.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/matmult_float_obj.dir/matmult-float/matmult.c.o ${BUILD_DIR}/matmult_float.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/matmult_int_obj.dir/matmult-int/matmult.c.o ${BUILD_DIR}/matmult_int.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/nettle_sha256_obj.dir/nettle-sha256/nettle-sha256.c.o ${BUILD_DIR}/nettle-sha256.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/rijndael_obj.dir/rijndael/aesxam.c.o ${BUILD_DIR}/aesxam.o
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/rijndael_obj.dir/rijndael/aes.c.o ${BUILD_DIR}/aes.o
)
