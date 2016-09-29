cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)

set(SYSROOT $ENV{NDK}/platforms/android-23/arch-arm)
set(CMAKE_C_COMPILER $ENV{NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc-4.9)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${SYSROOT} -std=c99")

file(GLOB LYNX_LWM2M_SRC src/*.c)

include_directories(include)
link_directories(lib)

add_library(lynx SHARED ${LYNX_LWM2M_SRC})
add_executable(testapp example.c)

target_link_libraries(lynx paho)
target_link_libraries(testapp lynx)