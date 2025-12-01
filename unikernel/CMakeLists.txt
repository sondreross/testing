cmake_minimum_required(VERSION 3.6)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(hello_includeos)

find_package(IncludeOS)

include(os)

# Define all benchmark object files
set(BENCHMARK_OBJS
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/crc_32.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/libcubic.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/basicmath_small.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/dijkstra_small.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/libfdct.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/libfir.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/matmult_float.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/matmult_int.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/nettle-sha256.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/aesxam.o
    ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks/benchmarks-build/aes.o
)

os_add_executable(hello_includeos "Hello world - OS included" src/main.cpp ${BENCHMARK_OBJS})
os_add_stdout(hello_includeos default_stdout)

# This would be for application specific libs.
#os_link_libraries(hello_includeos PRIVATE your_custom_lib)


# TODO: remove or hide the ELF_POSTFIX.
# currently os.cmake expects the target name to be TARGET.elf.bin
# where TARGET is the first param to os_add_executable.
# I added it here becuase nix expects an install target.
set(BINARY_NAME hello_includeos)
set(ELF_TARGET ${BINARY_NAME}${ELF_POSTFIX})

install(TARGETS ${ELF_TARGET} DESTINATION bin)
